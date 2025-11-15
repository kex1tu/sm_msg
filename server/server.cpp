/**
 * @file server.cpp
 * @brief Реализация главного класса, управляющего логикой чат-сервера.
 * @see Server
 */
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>
#include <QSqlRecord>
#include <algorithm>
#include <QWebSocket>
#include <thread>
#include <QUuid>
//#include <QtCore>



#include "structures.h"
#include "server.h"



/**
 * @brief Конструктор класса Server.
 *
 * @details Этот конструктор выполняет всю необходимую первоначальную настройку
 *          для запуска сервера:
 *          1.  Создает экземпляры TCP и WebSocket серверов.
 *          2.  Соединяет их сигналы `newConnection` со слотами для обработки
 *              новых клиентских подключений.
 *          3.  Инициализирует базу данных, создавая необходимые таблицы, если их нет.
 *              В случае неудачи, приложение аварийно завершается.
 *          4.  Инициализирует карту обработчиков команд (`m_handlers`).
 *
 * @param parent Родительский объект в иерархии Qt.
 */
Server::Server(QObject *parent) : QObject(parent)
{
    // 1. Создаем экземпляр TCP-сервера. `this` в качестве родителя
    //    гарантирует, что объект `m_tcpServer` будет автоматически удален
    //    при уничтожении объекта `Server`.
    m_tcpServer = new QTcpServer(this);

    // 2. Создаем экземпляр WebSocket-сервера.
    //    - "MessengerServer": Имя сервера, которое может быть видно клиентам.
    //    - QWebSocketServer::NonSecureMode: Указывает, что мы используем
    //      незащищенное соединение (ws://), а не защищенное (wss://).
    //    - `this`: Родительский объект.
    m_webSocketServer = new QWebSocketServer("MessengerServer", QWebSocketServer::NonSecureMode, this);


    // 3. Соединяем сигналы серверов со слотами-обработчиками.
    //    - Когда `QTcpServer` получает новое входящее TCP-подключение, он
    //      испускает сигнал `newConnection`, который вызывает наш слот `onNewTcpConnection`.
    connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewTcpConnection);

    //    - Аналогично для WebSocket-сервера.
    connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &Server::onNewWebSocketConnection);




    // 4. Инициализируем базу данных.
    if (!initDatabase()) {
        // Если инициализация БД провалилась (например, нет прав на запись файла),
        // дальнейшая работа сервера невозможна. `qFatal` выводит критическое
        // сообщение и немедленно завершает приложение.
        qFatal("Fatal: Database initialization failed!");
    }

    // 5. Заполняем нашу карту `m_handlers`, которая связывает строковые
    //    имена команд с указателями на методы, их обрабатывающие.
    initHandlers();
}

/**
 * @brief Запускает TCP и WebSocket серверы для прослушивания входящих подключений.
 *
 * @param address IP-адрес, на котором будут прослушиваться порты. По умолчанию QHostAddress::Any,
 *                что означает все доступные сетевые интерфейсы.
 * @param tcpPort Порт для TCP-подключений.
 * @param wsPort  Порт для WebSocket-подключений.
 * @return `true` если оба сервера успешно запущены, иначе `false`.
 */
bool Server::listen(const QHostAddress &address, quint16 tcpPort, quint16 wsPort)
{
    // Пытаемся запустить каждый сервер на указанном адресе и порту.
    bool tcpSuccess = m_tcpServer->listen(address, tcpPort);
    bool wsSuccess = m_webSocketServer->listen(address, wsPort);
    // Если оба запустились успешно...
    if (tcpSuccess && wsSuccess) {
        qDebug() << "TCP Server listening on port" <<address << tcpPort;
        qDebug() << "WebSocket Server listening on port" << address << wsPort;
        ///return true; // ...возвращаем успех.
    }

    // Если какой-либо из серверов не запустился, выводим подробную ошибку.
    if (!tcpSuccess) qDebug() << "TCP Server failed to start:" << m_tcpServer->errorString();
    if (!wsSuccess) qDebug() << "WebSocket Server failed to start:" << m_webSocketServer->errorString();



    return tcpSuccess && wsSuccess;
}

/**
 * @brief Слот, обрабатывающий новое входящее TCP-подключение.
 *
 * @details Этот слот вызывается сигналом `newConnection` от `m_tcpServer`.
 *          Он извлекает ожидающий сокет, устанавливает необходимые соединения
 *          сигнал-слот для этого конкретного клиента и инициализирует для него
 *          структуры данных (в данном случае, `m_nextBlockSizes`).
 */
void Server::onNewTcpConnection()
{
    // Извлекаем сокет клиента, ожидающий подключения.
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    qDebug() << "New TCP client connected from:" << socket->peerAddress().toString();

    // Соединяем сигналы от этого конкретного сокета с нашими слотами-обработчиками.
    connect(socket, &QTcpSocket::readyRead, this, &Server::onTcpReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

    // Инициализируем хранилище размера следующего блока для этого сокета нулем.
    m_nextBlockSizes.insert(socket, 0);
}

/**
 * @brief Слот, обрабатывающий новое входящее WebSocket-подключение.
 * @details Аналогичен `onNewTcpConnection`, но для WebSocket. Извлекает сокет,
 *          устанавливает соединения для получения текстовых сообщений и отслеживания
 *          разрыва соединения.
 */
void Server::onNewWebSocketConnection()
{
    // Извлекаем WebSocket-сокет клиента.
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();
    qDebug() << "New WebSocket client connected from:" << socket->peerAddress().toString();

    // Соединяем сигналы.
    connect(socket, &QWebSocket::textMessageReceived, this, &Server::onWebSocketTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &Server::onClientDisconnected);
}

/**
 * @brief Инициализирует карту обработчиков команд (Command Pattern).
 * @details Этот метод заполняет `m_handlers`, сопоставляя строковые имена команд,
 *          приходящие от клиентов в JSON-поле "type", с указателями на методы
 *          класса `Server`, которые должны их обрабатывать.
 */
void Server::initHandlers() {
    m_handlers["login"] = &Server::handleLogin;
    m_handlers["register"] = &Server::handleRegister;
    m_handlers["search_users"] = &Server::handleSearchUsers;
    m_handlers["private_message"] = &Server::handlePrivateMessage;
    m_handlers["get_history"] = &Server::handleGetHistory;
    m_handlers["add_contact_request"] = &Server::handleAddContactRequest;
    m_handlers["contact_request_response"] = &Server::handleContactRequestResponse;
    m_handlers["delete_message"] = &Server::handleDeleteMessage;
    m_handlers["edit_message"] = &Server::handleEditMessage;
    m_handlers["typing"] = &Server::handleTyping;
    m_handlers["message_delivered"] = &Server::handleMessageDelivered;
    m_handlers["message_read"] = &Server::handleMessageRead;
    m_handlers["logout_request"] = &Server::handleLogoutRequest;
    m_handlers["call_request"] = &Server::handleCallRequest;
    m_handlers["call_accepted"] = &Server::handleCallAccepted;
    m_handlers["call_rejected"] = &Server::handleCallRejected;
    m_handlers["call_end"] = &Server::handleCallEnd;
    m_handlers["get_call_history"] = &Server::handleGetCallHistory;
    m_handlers["update_profile"] = &Server::handleUpdateProfile;
}

void Server::handleUpdateProfile(QObject* socket, const QJsonObject& request)
{
    // Определяем имя пользователя по сокету
    QString username = m_clientsReverse.value(socket);

    // Вытаскиваем новые значения из запроса
    QString displayName   = request.value("display_name").toString();
    QString statusMessage = request.value("status_message").toString();
    QString avatarUrl     = request.value("avatar_url").toString(); // если передаётся, опционально

    // Готовим SQL-запрос на обновление профиля
    QSqlQuery query;
    query.prepare("UPDATE users SET display_name = :display_name, status_message = :status_message, avatar_url = :avatar_url WHERE username = :username");
    query.bindValue(":display_name", displayName);
    query.bindValue(":status_message", statusMessage);
    query.bindValue(":avatar_url", avatarUrl);
    query.bindValue(":username", username);

    QJsonObject response;
    response["type"] = "update_profile_result";

    if (query.exec()) {
        // Обновление прошло успешно
        response["success"] = true;
        response["username"] = username;
        response["display_name"] = displayName;
        response["status_message"] = statusMessage;
        response["avatar_url"] = avatarUrl;
        qDebug() << "[SERVER] User" << username << "updated their profile";
    } else {
        // Ошибка базы данных
        response["success"] = false;
        response["reason"] = query.lastError().text();
        qWarning() << "[SERVER] Profile update FAILED for" << username << ":" << query.lastError().text();
    }
    sendJson(socket, response);
}
/**
 * @brief Обрабатывает уведомление от клиента о том, что сообщение было ему доставлено.
 *
 * @details Когда клиент получает сообщение, он отправляет это подтверждение обратно.
 *          Сервер обновляет статус сообщения в базе данных на `is_delivered = 1`,
 *          а затем пересылает это уведомление **автору** сообщения, чтобы тот
 *          увидел, что его сообщение дошло до адресата.
 *
 * @param socket Сокет клиента, приславшего подтверждение (получатель сообщения).
 * @param request JSON-объект, содержащий `id` доставленного сообщения.
 */
void Server::handleMessageDelivered(QObject* socket, const QJsonObject& request) {
    Q_UNUSED(socket);
    quint64 messageId = request["id"].toInt();

    // 1. Обновляем статус в базе данных.
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE messages SET is_delivered = 1 WHERE id = :id");
    updateQuery.bindValue(":id", messageId);
    if (!updateQuery.exec()) {
        qWarning() << "[SERVER] Failed to mark message as delivered:" << updateQuery.lastError().text();
    } else {
        qDebug() << "[SERVER] Marked message" << messageId << "as delivered.";
    }

    // 2. Находим автора исходного сообщения, чтобы переслать ему уведомление.
    QSqlQuery query;
    query.prepare("SELECT fromUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (!query.exec() || !query.next()) {
        qWarning() << "DB Error: Could not find original sender for message ID" << messageId << ":" << query.lastError().text();
        return;
    }
    QString originalSender = query.value("fromUser").toString();

    // 3. Формируем команду "message_delivered" для автора.
    QJsonObject deliveredCmd;
    deliveredCmd["type"] = "message_delivered";
    deliveredCmd["id"] = (double)messageId;

    // 4. Отправляем уведомление автору сообщения, если он онлайн.
    sendJson(m_clients.value(originalSender), deliveredCmd);
}

/**
 * @brief Обрабатывает уведомление от клиента о том, что сообщение было прочитано.
 *
 * @details Логика аналогична `handleMessageDelivered`. Когда клиент-получатель
 *          показывает сообщение на экране, он отправляет это подтверждение.
 *          Сервер обновляет статус в БД на `is_read = 1` и пересылает
 *          уведомление автору сообщения, чтобы тот увидел, что его сообщение
 *          прочитано.
 *
 * @param socket Сокет клиента, приславшего подтверждение (получатель сообщения).
 * @param request JSON-объект, содержащий `id` прочитанного сообщения.
 */
void Server::handleMessageRead(QObject* socket, const QJsonObject& request) {
    Q_UNUSED(socket);
    quint64 messageId = request["id"].toInt();

    // 1. Обновляем статус в базе данных.
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE messages SET is_read = 1 WHERE id = :id");
    updateQuery.bindValue(":id", messageId);
    if (!updateQuery.exec()) {
        qWarning() << "[SERVER] Failed to mark message as read:" << updateQuery.lastError().text();
    } else {
        qDebug() << "[SERVER] Marked message" << messageId << "as read.";
    }

    // 2. Находим автора исходного сообщения.
    QSqlQuery query;
    query.prepare("SELECT fromUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (!query.exec() || !query.next()) {
        qWarning() << "DB Error: Could not find original sender for message ID" << messageId << ":" << query.lastError().text();
        return;
    }
    QString originalSender = query.value("fromUser").toString();

    // 3. Формируем команду "message_read" для автора.
    QJsonObject readCmd;
    readCmd["type"] = "message_read";
    readCmd["id"] = (double)messageId;

    // 4. Отправляем уведомление автору, если он онлайн.
    sendJson(m_clients.value(originalSender), readCmd);
}

/**
 * @brief Обрабатывает запрос клиента на выход из системы.
 *
 * @details Этот метод выполняет проверку безопасности, чтобы убедиться, что клиент
 *          пытается выйти из своего собственного аккаунта. В случае успеха, он
 *          отправляет подтверждение клиенту, удаляет его из списков онлайн-пользователей
 *          и рассылает всем остальным обновленный список присутствия.
 *
 * @param socket Сокет клиента, отправившего запрос.
 * @param request JSON-объект, содержащий `username` пользователя, который хочет выйти.
 */
void Server::handleLogoutRequest(QObject* socket, const QJsonObject& request)
{
    // Имя пользователя, указанное в запросе.
    QString fromUser = request["username"].toString();
    // Имя пользователя, реально ассоциированное с этим сокетом (извлеченное при логине).
    QString requestingUser = m_clientsReverse.value(socket);
    QJsonObject response;

    // Проверка безопасности: совпадают ли имена.
    if (fromUser != requestingUser) {
        // Если нет - это подозрительная активность.
        qWarning() << "[SERVER] SECURITY: User" << requestingUser << "is trying to log out as" << fromUser;
        response["type"] = "logout_request_failure";
        response["reason"] = "Authentication mismatch.";
        sendJson(socket, response);
        return;
    } else {
        // Если все в порядке, отправляем клиенту подтверждение об успешном выходе.
        response["type"] = "logout_request_success";
        sendJson(socket, response);

        // Удаляем пользователя из обеих карт, отслеживающих онлайн-статус.
        QString username = m_clientsReverse.value(socket); // Получаем имя еще раз для надежности.
        m_clients.remove(username);
        m_clientsReverse.remove(socket);

        // Рассылаем всем оставшимся онлайн-пользователям обновленный список присутствия.
        broadcastUserList();
        qDebug() << "[SERVER] User" << username << "logged out successfully.";
    }
}

/**
 * @brief Слот для чтения данных из TCP-сокета.
 *
 * @details Этот метод вызывается каждый раз, когда от TCP-клиента приходят новые данные.
 *          Он реализует логику сборки полных JSON-пакетов из непрерывного TCP-потока,
 *          используя протокол "размер + данные". После успешного извлечения
 *          JSON-объекта, он находит и вызывает соответствующий метод-обработчик.
 *          В этом методе есть небольшая логическая ошибка, он вызывает `processJsonRequest` дважды.
 */
void Server::onTcpReadyRead()
{
    // Определяем, какой именно сокет отправил сигнал.
    auto socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2); // Версия должна совпадать с клиентом.

    // Цикл для чтения всех полных пакетов, которые могли прийти за один раз.
    while (true) {
        quint32 &nextBlockSize = m_nextBlockSizes[socket];

        // Фаза 1: Чтение размера пакета.
        if (nextBlockSize == 0) { // Если мы еще не знаем размер следующего пакета...
            if (socket->bytesAvailable() < (qint64)sizeof(quint32)) {
                break; // ...и данных в буфере недостаточно даже для чтения размера, выходим.
            }
            in >> nextBlockSize; // Читаем размер (4 байта).
        }

        // Фаза 2: Чтение данных пакета.
        if (socket->bytesAvailable() < nextBlockSize) {
            break; // Если пакет пришел не полностью, выходим и ждем следующей порции данных.
        }

        // Данных достаточно, читаем JSON-пакет.
        QByteArray jsonData;
        in >> jsonData;
        nextBlockSize = 0; // Сбрасываем размер для следующей итерации.

        // Фаза 3: Парсинг и маршрутизация.
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (!doc.isNull() && doc.isObject()) {
            processJsonRequest(doc.object(), socket);
            continue;
        } else {
            QString type = doc.object()["type"].toString();
            qWarning() << "[SERVER] Unknown request type received:" << type;
            sendJson(socket, {{"type", "error"}, {"reason", "Unknown command: " + type}});
        }
    }
}

/**
 * @brief Слот для обработки текстового сообщения, полученного по WebSocket.
 *
 * @details WebSocket, в отличие от TCP, работает с сообщениями, а не с потоком.
 *          Поэтому нам не нужна сложная логика сборки пакетов. Мы просто
 *          получаем готовое сообщение, парсим его как JSON и передаем
 *          в общий диспетчер `processJsonRequest`.
 *
 * @param message Полученное текстовое сообщение (ожидается, что это строка в формате JSON).
 */
void Server::onWebSocketTextMessageReceived(const QString &message)
{
    auto socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        // Если парсинг прошел успешно, передаем запрос в общий обработчик.
        processJsonRequest(doc.object(), socket);
    } else {
        qWarning() << "[SERVER] Received invalid JSON from WebSocket client.";
    }
}

/**
 * @brief Общий диспетчер запросов от любого типа сокета.
 *
 * @details Этот метод является единой точкой входа для всех запросов после их
 *          парсинга. Он извлекает тип команды и, используя карту `m_handlers`,
 *          вызывает соответствующий метод-обработчик.
 *          (Примечание: В вашем коде этот метод дублирует логику из `onTcpReadyRead`.
 *          В идеале, `onTcpReadyRead` должен был бы вызывать `processJsonRequest`,
 *          а не реализовывать ту же логику заново.)
 *
 * @param request Распарсенный JSON-запрос.
 * @param socket Указатель на сокет клиента (QObject*), может быть QTcpSocket или QWebSocket.
 */
void Server::processJsonRequest(const QJsonObject& request, QObject* socket)
{
    QString type = request["type"].toString();
    qDebug() << "[SERVER] Processing message of type:" << type << "from" << m_clientsReverse.value(socket);

    // Проверяем, существует ли обработчик для такой команды.
    if (m_handlers.contains(type)) {
        Handler handler = m_handlers.value(type); // Получаем указатель на метод.
        (this->*handler)(socket, request);         // Вызываем метод по указателю.
    } else {
        qWarning() << "[SERVER] Unknown request type received in processJsonRequest:" << type;
        sendJson(socket, {{"type", "error"}, {"reason", "Unknown command: " + type}});
    }
}

/**
 * @brief Слот, обрабатывающий отключение клиента от сервера.
 *
 * @details Этот общий слот вызывается сигналом `disconnected` как от `QTcpSocket`,
 *          так и от `QWebSocket`. Он выполняет все необходимые действия по "очистке":
 *          1.  Определяет, был ли отключившийся клиент аутентифицирован.
 *          2.  Если да, обновляет в базе данных время его последнего визита (`last_seen`).
 *          3.  Удаляет клиента из списков онлайн-пользователей.
 *          4.  Рассылает всем оставшимся клиентам обновленный список присутствия.
 *          5.  Очищает специфичные для TCP структуры данных, если это был TCP-клиент.
 *          6.  Помечает объект сокета для безопасного удаления в цикле событий Qt.
 */
void Server::onClientDisconnected()
{
    // 1. Получаем указатель на объект-отправитель сигнала (отключившийся сокет).
    auto socket = qobject_cast<QObject*>(sender());
    if (!socket) return;

    // 2. Определяем, был ли этот клиент аутентифицирован (т.е. есть ли он в `m_clientsReverse`).
    QString username = m_clientsReverse.value(socket);
    if (!username.isEmpty()) {
        // --- Если клиент был аутентифицирован ---
        qDebug() << "User" << username << "disconnected.";

        // 3. Обновляем в БД время последнего визита.
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET last_seen = :lastSeen WHERE username = :username");
        updateQuery.bindValue(":lastSeen", QDateTime::currentDateTime().toString(Qt::ISODate));
        updateQuery.bindValue(":username", username);
        if (!updateQuery.exec()) {
            qWarning() << "[SERVER][ERROR] Failed to update last_seen for user" << username << ":" << updateQuery.lastError().text();
        } else {
            qDebug() << "[SERVER] Updated last_seen for user" << username;
        }

        // 4. Удаляем пользователя из обеих карт, отслеживающих онлайн-статус.
        m_clients.remove(username);
        m_clientsReverse.remove(socket);

        // 5. Рассылаем всем оставшимся онлайн-клиентам обновленный список присутствия.
        broadcastUserList();
    }

    // 6. Если это был TCP-сокет, удаляем его из карты `m_nextBlockSizes`.
    if (auto tcpSocket = qobject_cast<QTcpSocket*>(socket)) {
        m_nextBlockSizes.remove(tcpSocket);
    }

    // 7. Помечаем объект сокета для удаления. `deleteLater()` - это безопасный
    //    способ удалить QObject. Удаление произойдет, когда управление вернется
    //    в цикл обработки событий Qt, что предотвращает падения, если
    //    для этого объекта еще есть ожидающие события.
    sender()->deleteLater();
}

/**
 * @brief Инициализирует соединение с базой данных SQLite и создает таблицы, если их нет.
 *
 * @details Этот метод вызывается один раз в конструкторе сервера. Он выполняет
 *          критически важную задачу по настройке слоя персистентности.
 *          Он создает файл `messenger.db` (если он отсутствует) и выполняет
 *          SQL-запросы `CREATE TABLE IF NOT EXISTS` для создания необходимой
 *          структуры таблиц: `users`, `messages`, `contacts`.
 *
 * @return `true` если соединение установлено и все таблицы успешно созданы/проверены,
 *         иначе `false`.
 */
bool Server::initDatabase()
{
    // 1. Получаем объект соединения с БД. QSqlDatabase::addDatabase() создает
    //    именованное соединение (по умолчанию "qt_sql_default_connection").
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.db"); // Указываем имя файла БД.

    // 2. Пытаемся открыть соединение.
    if (!db.open()) {
        qCritical() << "Error: connection with database failed:" << db.lastError().text();
        return false;
    }
    qDebug() << "Database connection established successfully.";

    QSqlQuery query; // Создаем объект для выполнения SQL-запросов.

    // 3. Создаем таблицу `users`.
    //    `IF NOT EXISTS` предотвращает ошибку, если таблица уже существует.
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " // Уникальный ID, автоинкремент.
                    "username TEXT UNIQUE NOT NULL, "         // Логин, должен быть уникальным.
                    "display_name TEXT NOT NULL, "
                    "password_hash TEXT NOT NULL, "
                    "creation_date TEXT NOT NULL, "
                    "last_seen TEXT, "
                    "avatar_url TEXT, "
                    "status_message TEXT"
                    ");"))
    {
        qCritical() << "DB Error: failed to create 'users' table:" << query.lastError().text();
        return false;
    }

    // 4. Создаем таблицу `messages`.
    if (!query.exec("CREATE TABLE IF NOT EXISTS messages ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "fromUser TEXT NOT NULL, "
                    "toUser TEXT NOT NULL, "
                    "payload TEXT NOT NULL, "
                    "timestamp TEXT NOT NULL, "
                    "is_delivered INTEGER NOT NULL DEFAULT 0, " // Флаги доставки/прочтения.
                    "is_read INTEGER NOT NULL DEFAULT 0, "
                    "is_edited INTEGER NOT NULL DEFAULT 0, "
                    "reply_to_id INTEGER, " // ID цитируемого сообщения.
                    "forwarded_from TEXT, "
                    "message_type INTEGER NOT NULL DEFAULT 0, "
                    "media_url TEXT"
                    ");")) {
        qCritical() << "[SERVER] DB Error: failed to create 'messages' table:" << query.lastError().text();
        return false;
    }

    // 5. Создаем таблицу `contacts`.
    //    Это связующая таблица для отношений "многие ко многим" между пользователями.
    if (!query.exec("CREATE TABLE IF NOT EXISTS contacts ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "user_id_1 INTEGER NOT NULL, "
                    "user_id_2 INTEGER NOT NULL, "
                    "status INTEGER NOT NULL DEFAULT 0, " // 0=pending, 1=accepted, 2=blocked и т.д.
                    "creation_date TEXT NOT NULL, "
                    "FOREIGN KEY(user_id_1) REFERENCES users(id), " // Внешние ключи для целостности данных.
                    "FOREIGN KEY(user_id_2) REFERENCES users(id), "
                    "UNIQUE(user_id_1, user_id_2), " // Гарантирует, что пара юзеров уникальна.
                    "CHECK(user_id_1 < user_id_2)" // Предотвращает дублирование (1,2) и (2,1).
                    ");"))
    {
        qCritical() << "DB Error: failed to create 'contacts' table:" << query.lastError().text();
        return false;
    }



    // 6. Создаем таблицу для истории звонков
    if (!query.exec("CREATE TABLE IF NOT EXISTS call_history (\n"
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                    "call_id TEXT UNIQUE NOT NULL,\n"
                    "caller_username TEXT NOT NULL,\n"
                    "callee_username TEXT NOT NULL,\n"
                    "status VARCHAR(20) NOT NULL DEFAULT 'ringing',\n"  // ringing, connected, completed, rejected, missed
                    "start_time TEXT NOT NULL,\n"
                    "connect_time TEXT,\n"
                    "end_time TEXT,\n"
                    "duration_seconds INTEGER DEFAULT 0,\n"
                    "caller_ip VARCHAR(45),\n"
                    "caller_port INTEGER,\n"
                    "callee_ip VARCHAR(45),\n"
                    "callee_port INTEGER,\n"
                    "FOREIGN KEY(caller_username) REFERENCES users(username),\n"
                    "FOREIGN KEY(callee_username) REFERENCES users(username)\n"
                    ");")) {
        qCritical() << "[SERVER] DB Error: failed to create 'call_history' table:" << query.lastError().text();
        return false;
    }

    qDebug() << "[SERVER] Call history table initialized successfully.";

    // Индексы для быстрого поиска
    query.exec("CREATE INDEX IF NOT EXISTS idx_call_caller ON call_history(caller_username);");
    query.exec("CREATE INDEX IF NOT EXISTS idx_call_callee ON call_history(callee_username);");
    query.exec("CREATE INDEX IF NOT EXISTS idx_call_start_time ON call_history(start_time DESC);");
    query.exec("CREATE INDEX IF NOT EXISTS idx_call_id ON call_history(call_id);");




    //FOR GO

    if (!query.exec("CREATE TABLE IF NOT EXISTS files ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "file_uuid TEXT UNIQUE NOT NULL, "//       -- Уникальный ID, который мы генерируем
                    "owner_username TEXT NOT NULL, " //      -- Кто загрузил
                    "original_filename TEXT NOT NULL, "//  -- Исходное имя файла
                    "filesize INTEGER NOT NULL, "//         -- Размер
                    "status INTEGER NOT NULL DEFAULT 0, "// -- 0: pending, 1: uploaded, 2: error
                    "upload_date TEXT NOT NULL);")){
        qCritical() << "DB Error: failed to create 'files' table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database tables initialized successfully.";
    return true;
}

/**
 * @brief Обрабатывает и пересылает уведомление о наборе текста.
 *
 * @details Этот метод вызывается, когда клиент отправляет команду "typing".
 *          Сервер выступает в роли простого посредника: он определяет, кто отправил
 *          уведомление, находит сокет получателя (если тот онлайн) и пересылает
 *          ему это уведомление.
 *
 * @param socket Сокет клиента, который начал печатать.
 * @param request JSON-объект, содержащий `toUser` - имя получателя.
 */
void Server::handleTyping(QObject *socket, const QJsonObject &request)
{
    // 1. Определяем отправителя по его сокету.
    QString fromUsername = m_clientsReverse.value(socket);

    // 2. Получаем получателя из запроса.
    QString toUsername = request["toUser"].toString();

    // 3. Ищем сокет получателя в списке онлайн-клиентов.
    QObject* toSocket = m_clients.value(toUsername, nullptr);

    // 4. Если получатель онлайн (сокет найден)...
    if (toSocket) {
        // ...создаем новое JSON-сообщение и пересылаем его.
        QJsonObject forwardMessage;
        forwardMessage["type"] = "typing";
        forwardMessage["fromUser"] = fromUsername;

        sendJson(toSocket, forwardMessage);
    }
}

/**
 * @brief Обрабатывает запрос клиента на получение истории сообщений.
 *
 * @details Этот метод извлекает из базы данных порцию (до 20) сообщений для
 *          указанного чата. Он поддерживает пагинацию: если в запросе указан
 *          `before_id`, он загружает сообщения, которые старше этого ID.
 *
 * @param socket Сокет клиента, запросившего историю.
 * @param request JSON-объект, содержащий:
 *                - `with_user`: Имя собеседника, для которого нужна история.
 *                - `before_id` (опционально): ID сообщения, старше которого нужно загрузить историю.
 */
void Server::handleGetHistory(QObject* socket, const QJsonObject& request)
{
    // 1. Извлекаем параметры запроса.
    QString requestingUser = m_clientsReverse.value(socket);
    QString chatPartner = request["with_user"].toString();
    qint64 beforeId = request["before_id"].toDouble(); // 0, если поле отсутствует.

    qDebug() << "[SERVER] History request from" << requestingUser
             << "for chat with" << chatPartner
             << "before message ID:" << beforeId;

    // --- 2. Формирование SQL-запроса ---
    QSqlQuery query;
    QString queryString =
        "SELECT id, fromUser, toUser, payload, timestamp, reply_to_id, is_read, is_edited, is_delivered FROM messages "
        "WHERE ((fromUser = :user1 AND toUser = :user2) OR (fromUser = :user2 AND toUser = :user1)) ";

    // Если указан `beforeId`, добавляем условие для пагинации (загрузка более старых сообщений).
    if (beforeId > 0) {
        queryString += "AND id < :beforeId ";
    }

    // Сортируем по ID в обратном порядке (от новых к старым) и берем не более 20.
    queryString += "ORDER BY id DESC LIMIT 20";

    // --- 3. Подготовка и выполнение запроса ---
    query.prepare(queryString);
    query.bindValue(":user1", requestingUser);
    query.bindValue(":user2", chatPartner);
    if (beforeId > 0) {
        query.bindValue(":beforeId", beforeId);
    }

    if (!query.exec()) {
        qWarning() << "DB Error: History request failed:" << query.lastError().text();
        return;
    }

    // --- 4. Формирование JSON-ответа ---
    QJsonArray historyArray;
    while (query.next()) {
        QSqlRecord record = query.record();
        QJsonObject messageObject;
        // Заполняем JSON-объект данными из записи БД.
        messageObject["id"] = record.value("id").toLongLong();
        messageObject["fromUser"] = record.value("fromUser").toString();
        messageObject["toUser"] = record.value("toUser").toString();
        messageObject["payload"] = record.value("payload").toString();
        messageObject["timestamp"] = record.value("timestamp").toString();
        messageObject["is_read"] = record.value("is_read").toInt();
        messageObject["is_delivered"] = record.value("is_delivered").toInt();
        messageObject["is_edited"] = record.value("is_edited").toInt();
        messageObject["reply_to_id"] = record.value("reply_to_id").toLongLong();

        historyArray.append(messageObject);
    }
    qDebug() << "[SERVER] Found" << historyArray.count() << "messages for this chat.";

    // Так как мы выбирали сообщения от новых к старым (DESC), а клиенту нужно
    // их отобразить от старых к новым, мы инвертируем порядок массива.
    QJsonArray reversedArray;
    for (int i = historyArray.size() - 1; i >= 0; --i) {
        reversedArray.append(historyArray.at(i));
    }

    // --- 5. Отправка ответа клиенту ---
    QJsonObject response;
    // Тип ответа зависит от того, был ли это первоначальный запрос или подгрузка.
    if (beforeId > 0) {
        response["type"] = "old_history_data"; // Подгрузка старой истории.

    } else {
        response["type"] = "history_data"; // Первоначальная загрузка.
    }

    response["with_user"] = chatPartner;
    response["history"] = reversedArray;
    sendJson(socket, response);
}

/**
 * @brief Обрабатывает запрос на регистрацию нового пользователя.
 *
 * @details Метод пытается добавить нового пользователя в таблицу `users`.
 *          Хэширует пароль с использованием SHA-256 перед сохранением.
 *          Отправляет клиенту ответ `register_success` или `register_failure`
 *          в зависимости от результата операции.
 *
 * @param socket Сокет клиента, запросившего регистрацию.
 * @param request JSON-объект, содержащий `username`, `display_name` и `password`.
 */
void Server::handleRegister(QObject* socket, const QJsonObject& request)
{
    // 1. Извлекаем данные из запроса.
    QString username = request["username"].toString();
    QString display_name = request["display_name"].toString();
    QString password = request["password"].toString();

    // 2. Хэшируем пароль для безопасного хранения.
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 3. Готовим SQL-запрос на вставку.
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, display_name, password_hash, creation_date) "
                  "VALUES (:username, :display_name, :password_hash, :creation_date)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", QString(passwordHash));
    query.bindValue(":display_name", display_name);
    query.bindValue(":creation_date", QDateTime::currentDateTime().toString(Qt::ISODate));

    // 4. Выполняем запрос и формируем ответ.
    QJsonObject response;
    if (query.exec()) {
        // Если запрос выполнен успешно, значит, пользователь создан.
        response["type"] = "register_success";
        qDebug() << "[SERVER] New user registered:" << username;
        // Примечание: `broadcastUserList` здесь не совсем уместен, так как новый
        // пользователь еще не в списке контактов ни у кого.
        // broadcastUserList();
    } else {
        // Если произошла ошибка, скорее всего, сработало ограничение UNIQUE на поле `username`.
        response["type"] = "register_failure";
        response["reason"] = "Username already exists.";
        qWarning() << "[SERVER] Registration failed for" << username << ":" << query.lastError().text();
    }

    // 5. Отправляем ответ клиенту.
    sendJson(socket, response);
}

/**
 * @brief Обрабатывает запрос на глобальный поиск пользователей.
 *
 * @details Этот метод выполняет поиск в таблице `users` по частичному совпадению
 *          в полях `username` или `display_name`. Он исключает из результатов
 *          самого пользователя, который выполняет поиск, и ограничивает
 *          выводку 20 результатами для производительности.
 *
 * @param socket Сокет клиента, выполнившего поиск.
 * @param request JSON-объект, содержащий `term` - строку для поиска.
 */
void Server::handleSearchUsers(QObject* socket, const QJsonObject& request)
{
    // 1. Извлекаем параметры: поисковый запрос и имя текущего пользователя.
    QString searchTerm = request["term"].toString();
    QString currentUser = m_clientsReverse.value(socket);

    // 2. Готовим SQL-запрос.
    QSqlQuery query;
    // `LIKE` с символами `%` используется для поиска подстроки.
    // `username != :currentUser` исключает самого себя из результатов.
    // `LIMIT 20` ограничивает количество возвращаемых строк.
    query.prepare("SELECT username, display_name FROM users "
                  "WHERE (username LIKE :term OR display_name LIKE :term) "
                  "AND username != :currentUser LIMIT 20");
    query.bindValue(":term", "%" + searchTerm + "%"); // Оборачиваем поисковый запрос в %.
    query.bindValue(":currentUser", currentUser);

    if (!query.exec()) {
        qWarning() << "[SERVER] User search failed:" << query.lastError().text();
        return;
    }

    // 3. Формируем JSON-массив с результатами.
    QJsonArray usersFound;
    while (query.next()) {
        QJsonObject userObject;
        userObject["username"] = query.value(0).toString();
        userObject["displayname"] = query.value(1).toString();
        usersFound.append(userObject);
    }

    // 4. Отправляем ответ клиенту.
    QJsonObject response;
    response["type"] = "search_results";
    response["users"] = usersFound;
    sendJson(socket, response);
    qDebug() << "[SERVER] Found" << usersFound.count() << "users for term '" << searchTerm << "'.";
}

/**
 * @brief Отправляет клиенту его персональный список контактов.
 *
 * @details Этот метод вызывается после успешного входа пользователя. Он выполняет
 *          сложный SQL-запрос с `JOIN` для получения всех пользователей,
 *          с которыми у текущего пользователя установлены подтвержденные
 *          отношения (`status = 1`) в таблице `contacts`.
 *
 * @param socket Сокет клиента, которому нужно отправить список контактов.
 * @param username Имя пользователя, для которого нужно сформировать список.
 */
void Server::sendContactList(QObject* socket, const QString& username)
{
    // --- Шаг 1: Получаем ID пользователя по его имени ---
    QSqlQuery userQuery;
    userQuery.prepare("SELECT id FROM users WHERE username = :username");
    userQuery.bindValue(":username", username);

    if (!userQuery.exec() || !userQuery.next()) {
        qWarning() << "[SERVER] Could not find ID for user" << username << "to send contact list.";
        return;
    }
    qint64 userId = userQuery.value("id").toLongLong();

    // --- Шаг 2: Основной SQL-запрос для получения контактов ---
    QSqlQuery query;
    // Этот запрос объединяет таблицы `users` и `contacts`.
    // Он находит все записи в `contacts`, где `userId` является либо `user_id_1`, либо `user_id_2`.
    // Затем он извлекает данные пользователя (`username`, `display_name`, `last_seen`)
    // для "другой стороны" этой связи.
    query.prepare(
        "SELECT u.username, u.display_name, u.last_seen FROM users u "
        "JOIN contacts c ON (u.id = c.user_id_1 OR u.id = c.user_id_2) " // Условие соединения.
        "WHERE (c.user_id_1 = :userId OR c.user_id_2 = :userId) " // Находим все связи, где есть наш юзер.
        "AND c.status = 1 " // Выбираем только подтвержденные контакты.
        "AND u.id != :userId" // Исключаем самого себя из результатов.
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qWarning() << "[SERVER] Failed to get contact list for" << username << ":" << query.lastError().text();
        return;
    }

    // --- Шаг 3: Формирование и отправка JSON-ответа ---
    QJsonArray contactsArray;
    while (query.next()) {
        QJsonObject userObject;
        userObject["username"] = query.value(0).toString();
        userObject["displayname"] = query.value(1).toString();
        userObject["last_seen"] = query.value(2).toString();
        contactsArray.append(userObject);
    }

    QJsonObject message;
    message["type"] = "contact_list";
    message["users"] = contactsArray;
    sendJson(socket, message);
    qDebug() << "[SERVER] Sent contact list with" << contactsArray.count() << "contacts to" << username;
}

/**
 * @brief Обрабатывает запрос от одного пользователя на добавление другого в список контактов.
 *
 * @details Этот метод выполняет сложную логику, включающую множество проверок и
 *          взаимодействий с базой данных:
 *          1.  Валидация входящих данных.
 *          2.  Получение ID обоих пользователей из БД.
 *          3.  Проверка, не существует ли уже какая-либо связь (запрос, дружба) между ними.
 *          4.  Создание новой записи в таблице `contacts` со статусом "pending" (ожидает).
 *          5.  Отправка push-уведомления целевому пользователю, если он онлайн.
 *          6.  Отправка подтверждения об успехе или сообщения об ошибке инициатору запроса.
 *
 * @param socket Сокет клиента-инициатора запроса.
 * @param request JSON-объект, содержащий `username` целевого пользователя.
 */
void Server::handleAddContactRequest(QObject* socket, const QJsonObject& request)
{
    // 1. Определяем отправителя и получателя.
    QString fromUsername = m_clientsReverse.value(socket);
    QString toUsername = request["username"].toString();

    // --- 2. Валидация входных данных ---
    if (toUsername.isEmpty()) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Некорректное имя пользователя."}});
        return;
    }
    if (fromUsername == toUsername) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Вы не можете добавить самого себя в контакты."}});
        return;
    }

    // --- 3. Получаем ID и display_name обоих пользователей из БД одним запросом ---
    QSqlQuery idQuery;
    idQuery.prepare("SELECT id, username, display_name FROM users WHERE username = :from OR username = :to");
    idQuery.bindValue(":from", fromUsername);
    idQuery.bindValue(":to", toUsername);
    if (!idQuery.exec()) {
        qWarning() << "[SERVER] DB Error: Failed to find user IDs:" << idQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Ошибка базы данных."}});
        return;
    }

    qint64 fromId = -1, toId = -1;
    QString fromDisplayName;
    while (idQuery.next()) {
        if (idQuery.value("username").toString() == fromUsername) {
            fromId = idQuery.value("id").toLongLong();
            fromDisplayName = idQuery.value("display_name").toString();
        } else {
            toId = idQuery.value("id").toLongLong();
        }
    }

    // Проверяем, что оба пользователя были найдены.
    if (fromId == -1 || toId == -1) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Запрашиваемый пользователь не существует."}});
        return;
    }

    // --- 4. Проверка на существующие отношения ---
    // Чтобы избежать дублирования (1,2) и (2,1), мы всегда храним ID в порядке возрастания.
    qint64 userId1 = std::min(fromId, toId);
    qint64 userId2 = std::max(fromId, toId);

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT status FROM contacts WHERE user_id_1 = :id1 AND user_id_2 = :id2");
    checkQuery.bindValue(":id1", userId1);
    checkQuery.bindValue(":id2", userId2);
    if (!checkQuery.exec()) {
        qWarning() << "[SERVER] DB Error: Failed to check for existing contact:" << checkQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Ошибка базы данных."}});
        return;
    }

    // Если запрос вернул результат, значит, запись уже существует.
    if (checkQuery.next()) {
        int status = checkQuery.value(0).toInt();
        QString reason;
        if (status == 0) reason = "Запрос этому пользователю уже отправлен и ожидает ответа.";
        else if (status == 1) reason = "Этот пользователь уже в вашем списке контактов.";
        else reason = "С этим пользователем уже существует связь.";
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", reason}});
        return;
    }

    // --- 5. Создание нового запроса в контакты ---
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO contacts (user_id_1, user_id_2, status, creation_date) "
                        "VALUES (:id1, :id2, 0, :date)"); // status = 0 означает "pending".
    insertQuery.bindValue(":id1", userId1);
    insertQuery.bindValue(":id2", userId2);
    insertQuery.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!insertQuery.exec()) {
        qWarning() << "[SERVER] DB Error: Failed to insert contact request:" << insertQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Ошибка базы данных при отправке запроса."}});
        return;
    }

    // --- 6. Отправка уведомлений ---
    // Если целевой пользователь онлайн, отправляем ему push-уведомление.
    QObject* toSocket = m_clients.value(toUsername, nullptr);
    if (toSocket) {
        QJsonObject notification;
        notification["type"] = "incoming_contact_request";
        notification["fromUsername"] = fromUsername;
        notification["fromDisplayname"] = fromDisplayName;
        sendJson(toSocket, notification);
        qDebug() << "[SERVER] Sent incoming contact request notification to" << toUsername;
    }

    // Отправляем инициатору подтверждение об успехе.
    sendJson(socket, {{"type", "add_contact_success"}, {"reason", "Запрос на добавление успешно отправлен пользователю " + toUsername + "."}});
    qDebug() << "[SERVER] User" << fromUsername << "sent a contact request to" << toUsername;
}

/**
 * @brief Обрабатывает запрос на аутентификацию пользователя.
 *
 * @details Этот метод является точкой входа для любого клиента. Он выполняет следующие шаги:
 *          1.  Извлекает `username` и `password` из запроса.
 *          2.  Хэширует пароль.
 *          3.  Ищет пользователя в БД по `username` и сравнивает хэши паролей.
 *          4.  В случае успеха:
 *              -   Отправляет клиенту `login_success`.
 *              -   Регистрирует клиента в системе (добавляет в `m_clients` и `m_clientsReverse`).
 *              -   Отправляет ему его список контактов, список ожидающих запросов и счетчики непрочитанных.
 *              -   Рассылает всем остальным обновленный список онлайн-пользователей.
 *          5.  В случае неудачи отправляет `login_failure` с причиной.
 *
 * @param socket Сокет клиента, пытающегося войти.
 * @param request JSON-объект, содержащий `username` и `password`.
 */
void Server::handleLogin(QObject* socket, const QJsonObject& request)
{
    QString username = request["username"].toString();
    QString password = request["password"].toString();
    qDebug() << username << " : " << password;
    // Хэшируем полученный пароль тем же алгоритмом (SHA-256), что и при регистрации.
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    // Ищем пользователя в БД.
    QSqlQuery query;
    query.prepare("SELECT password_hash, display_name, avatar_url, status_message FROM users WHERE username = :username");
    query.bindValue(":username", username);

    QJsonObject response;
    // Если запрос выполнен и пользователь с таким именем найден...
    if (query.exec() && query.next()) {
        QString storedHash = query.value(0).toString(); // Получаем хэш из БД.

        // ...сравниваем хэши.
        if (storedHash == passwordHash) {
            QString display_name = query.value(1).toString();
            QString status_message = query.value(2).toString();
            QString avatar_url = query.value(3).toString();
            // --- Успешный вход ---
            qDebug() << "[SERVER] User" << username << "logged in successfully.";
            response["type"] = "login_success";
            response["username"] = username;
            response["display_name"] = display_name;
            response["status_message"] =status_message;       // если есть
            response["avatar_url"] = avatar_url;      // если есть

            sendJson(socket, response);


            // Регистрируем клиента в системе.
            m_clients[username] = socket;
            m_clientsReverse[socket] = username;

            // Отправляем клиенту подтверждение успеха.
            //sendJson(socket, response);

            // Отправляем всю необходимую для начала работы информацию этому клиенту.
            sendContactList(socket, username);
            sendPendingContactRequests(socket, username);
            sendUnreadCounts(socket, username);

            // Оповещаем всех онлайн-пользователей (включая нового),
            // что список присутствия изменился.
            broadcastUserList();


        } else {
            // --- Неверный пароль ---
            qWarning() << "[SERVER] Failed login attempt for user" << username << ": Invalid password.";
            response["type"] = "login_failure";
            response["reason"] = "Неверное имя пользователя или пароль.";
            sendJson(socket, response);
        }
    } else {
        // --- Пользователь не найден ---
        qWarning() << "[SERVER] Failed login attempt: User" << username << "not found.";
        response["type"] = "login_failure";
        response["reason"] = "Неверное имя пользователя или пароль."; // Отправляем общую ошибку из соображений безопасности.
        sendJson(socket, response);
    }
}

/**
 * @brief Обрабатывает запрос на отправку личного сообщения от одного пользователя другому.
 *
 * @details Это одна из самых важных функций сервера. Она выполняет полный цикл обработки сообщения:
 *          1.  Проверяет аутентичность отправителя.
 *          2.  Сохраняет сообщение в базу данных, присваивая ему уникальный ID и серверную временную метку.
 *          3.  Отправляет "echo"-ответ отправителю, подтверждая получение сообщения и сообщая его ID.
 *              Этот ответ содержит `temp_id` из исходного запроса клиента.
 *          4.  Если получатель онлайн, немедленно пересылает ему это же сообщение.
 *          5.  Если получатель офлайн, сообщение просто остается в БД для последующей доставки.
 *
 * @param socket Сокет клиента-отправителя.
 * @param request JSON-объект, содержащий `fromUser`, `toUser`, `payload`, `reply_to_id` и `temp_id`.
 */
void Server::handlePrivateMessage(QObject* socket, const QJsonObject& request)
{
    // 1. Извлекаем все данные из JSON-запроса.
    QString fromUser = request["fromUser"].toString();
    QString toUser = request["toUser"].toString();
    QString payload = request["payload"].toString();
    qint64 replyToId = request["reply_to_id"].toVariant().toLongLong();
    QString tempId = request["temp_id"].toString();
    // Генерируем временную метку на сервере, чтобы избежать рассинхронизации времени у клиентов.
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    // 2. Проверка безопасности: убеждаемся, что пользователь, отправляющий сообщение,
    //    действительно тот, за кого себя выдает (сравниваем `fromUser` с именем,
    //    ассоциированным с этим сокетом при логине).
    if (fromUser != m_clientsReverse.value(socket)) {
        qWarning() << "[SERVER] SECURITY WARNING: User" << m_clientsReverse.value(socket)
        << "tried to send a message as" << fromUser;
        return; // Молча прерываем обработку.
    }

    // 3. Сохраняем сообщение в базу данных.
    QSqlQuery query;
    query.prepare("INSERT INTO messages (fromUser, toUser, payload, timestamp, reply_to_id) "
                  "VALUES (:fromUser, :toUser, :payload, :timestamp, :reply_to_id)");
    query.bindValue(":fromUser", fromUser);
    query.bindValue(":toUser", toUser);
    query.bindValue(":payload", payload);
    query.bindValue(":timestamp", timestamp);
    // Обрабатываем `replyToId`: если он 0, вставляем NULL в БД.
    query.bindValue(":reply_to_id", replyToId > 0 ? QVariant(replyToId) : QVariant());

    if (!query.exec()) {
        qWarning() << "[SERVER] Failed to save message to DB:" << query.lastError().text();
        // Можно отправить отправителю сообщение об ошибке, но пока просто логируем.
    }

    // 4. Получаем ID, который база данных присвоила нашему новому сообщению.
    quint64 messageId = query.lastInsertId().toULongLong();

    // --- 5. Отправка Echo-ответа отправителю ---
    QJsonObject echoMessage;
    echoMessage["type"] = "private_message";
    echoMessage["id"] = (double)messageId;
    echoMessage["fromUser"] = fromUser;
    echoMessage["toUser"] = toUser;
    echoMessage["payload"] = payload;
    echoMessage["timestamp"] = timestamp;
    echoMessage["is_delivered"] = 0; // Изначально не доставлено.
    echoMessage["is_read"] = 0;
    echoMessage["is_edited"] = 0;
    if (replyToId > 0) echoMessage["reply_to_id"] = replyToId;
    echoMessage["temp_id"] = tempId; // Включаем temp_id для сопоставления на клиенте.

    sendJson(socket, echoMessage);

    // --- 6. Пересылка сообщения получателю (если он онлайн) ---
    // Удаляем `temp_id`, так как он не нужен получателю.
    echoMessage.remove("temp_id");

    // Ищем сокет получателя в списке онлайн-клиентов.
    QObject *toUserSocket = m_clients.value(toUser, nullptr);

    if (toUserSocket) {
        // Если получатель онлайн, отправляем ему сообщение.
        sendJson(toUserSocket, echoMessage);
        qDebug() << "[SERVER] Private message" << messageId << "forwarded from" << fromUser << "to" << toUser;
    } else {
        qDebug() << "[SERVER] User" << toUser << "is offline. Message" << messageId << "stored.";
    }
}

/**
 * @brief Универсальный метод для отправки JSON-объекта клиенту.
 *
 * @details Этот метод инкапсулирует логику сериализации JSON и отправки данных
 *          по сети. Он автоматически определяет тип сокета (QTcpSocket или QWebSocket)
 *          и использует соответствующий протокол для отправки.
 *
 * @param socket Указатель на сокет получателя (в виде QObject*).
 * @param json JSON-объект для отправки.
 */
void Server::sendJson(QObject* socket, const QJsonObject& json)
{
    if (!socket) return; // Проверка на null.

    // Преобразуем JSON-объект в компактный байтовый массив.
    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // Проверяем, является ли сокет TCP-сокетом.
    if (auto tcpSocket = qobject_cast<QTcpSocket*>(socket)) {
        // --- Логика для TCP ---
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_2);
        // Формируем пакет "размер (4 байта) + данные".
        out << (quint32)0; // Резервируем место для размера.
        out << jsonData;
        out.device()->seek(0); // Возвращаемся в начало.
        out << (quint32)(block.size() - sizeof(quint32)); // Записываем реальный размер.
        tcpSocket->write(block);

        // Проверяем, является ли сокет WebSocket-сокетом.
    } else if (auto wsSocket = qobject_cast<QWebSocket*>(socket)) {
        // --- Логика для WebSocket ---
        // Просто отправляем данные как одно текстовое сообщение.
        wsSocket->sendTextMessage(QString::fromUtf8(jsonData));
    }
    qDebug() << "JSON send" << jsonData.size();
}

/**
 * @brief (Устарело/Для админки) Отправляет клиенту полный список всех зарегистрированных пользователей.
 *
 * @details Этот метод извлекает из БД всех пользователей без исключения.
 *          В текущей логике он не используется, так как клиенту обычно нужен
 *          только его список контактов (`sendContactList`). Может быть полезен
 *          для будущей административной панели или функции "найти всех".
 *
 * @param socket Сокет клиента, которому нужно отправить список.
 */
void Server::sendFullUserList(QObject* socket)
{
    QSqlQuery query;
    query.prepare("SELECT username, display_name FROM users");
    if (!query.exec()) {
        qWarning() << "Failed to get full user list:" << query.lastError().text();
        return;
    }

    QJsonArray allUsers;
    while (query.next()) {
        QJsonObject userObject;
        userObject["username"] = query.value(0).toString();
        userObject["displayname"] = query.value(1).toString();
        allUsers.append(userObject);
    }

    QJsonObject message;
    message["type"] = "full_user_list";
    message["users"] = allUsers;
    sendJson(socket, message);
    qDebug() << "Sent full user list to a client.";
}

/**
 * @brief Рассылает всем онлайн-клиентам актуальный список пользователей, находящихся в сети.
 * @details Этот метод получает список имен пользователей из ключей карты `m_clients`,
 *          что представляет собой полный список всех аутентифицированных и подключенных
 *          в данный момент клиентов. Затем он формирует JSON-сообщение с типом "user_list"
 *          и отправляет его каждому клиенту из списка `m_clients`.
 *          Обычно вызывается после успешного входа пользователя или после его отключения,
 *          чтобы все клиенты могли обновить свои списки контактов, отображая актуальный
 *          онлайн-статус.
 */
void Server::broadcastUserList(){
    QStringList onlineUsers = m_clients.keys();
    qDebug() << "Broadcasting ONLINE user list:" << onlineUsers;

    QJsonObject message;
    message["type"] = "user_list";
    message["users"] = QJsonArray::fromStringList(onlineUsers);


    for (QObject  *socket :  m_clients.values()) {
        sendJson(socket, message);
    }
}

/**
 * @brief Обрабатывает запрос от клиента на редактирование ранее отправленного сообщения.
 * @details Эта функция выполняет полный цикл обработки запроса на редактирование:
 *          1. Определяет пользователя, отправившего запрос, по его сокету.
 *          2. Извлекает ID сообщения и новый текст (`payload`) из JSON-запроса.
 *          3. Выполняет проверку безопасности: запрашивающий пользователь должен быть
 *             авторизован и являться автором (`fromUser`) редактируемого сообщения.
 *          4. В случае успеха, обновляет текст сообщения в базе данных и устанавливает
 *             флаг `is_edited = 1`.
 *          5. Создает JSON-уведомление типа "edit_message" с обновленными данными.
 *          6. Отправляет это уведомление как автору, так и получателю сообщения
 *             (если они онлайн), чтобы их интерфейсы могли отобразить изменения.
 * @param clientSocket Сокет клиента, инициировавшего редактирование.
 * @param request JSON-объект, содержащий:
 *                - `id`: Уникальный идентификатор редактируемого сообщения.
 *                - `payload`: Новый текст сообщения.
 */
void Server::handleEditMessage(QObject* clientSocket, const QJsonObject& request)
{
    QString requestingUser = m_clientsReverse.value(clientSocket);
    quint64 messageId = request["id"].toDouble();

    qDebug() << requestingUser << "wants to edit message with id:" << messageId;
    if (messageId == 0) return;

    // Проверка, что запрос пришел от аутентифицированного пользователя.
    if (requestingUser.isEmpty()) {
        qWarning() << "SECURITY: Edit request from an unauthenticated socket!";
        return;
    }

    // Находим оригинального автора и получателя сообщения в БД.
    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (query.exec() && query.next()) {
        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();

        qDebug() << "Requesting user:" << requestingUser << " fromUser in DB:" << fromUser << " toUser in DB:" << toUser;

        // Проверка безопасности: только автор может редактировать свое сообщение.
        if (fromUser == requestingUser) {
            QSqlQuery updateQuery;
            QString newPayload = request["payload"].toString();
            updateQuery.prepare("UPDATE messages SET payload = :payload, is_edited = 1 WHERE id = :id");
            updateQuery.bindValue(":payload", newPayload);
            updateQuery.bindValue(":id", messageId);

            if (updateQuery.exec()) {
                qDebug() << "[SERVER] User" << requestingUser << "edited message" << messageId;

                // Формируем уведомление для рассылки.
                QJsonObject editCmd;
                editCmd["type"] = "edit_message";
                editCmd["id"] = (double)messageId;
                editCmd["payload"] = newPayload;

                // Отправляем автору.
                QObject* fromSocket = m_clients.value(fromUser, nullptr);
                if (fromSocket) {
                    editCmd["with_user"] = toUser;
                    sendJson(fromSocket, editCmd);
                }

                // Отправляем получателю.
                QObject* toSocket = m_clients.value(toUser, nullptr);
                if (toSocket) {
                    editCmd["with_user"] = fromUser;
                    sendJson(toSocket, editCmd);
                }
            }
        } else {
            qWarning() << "SECURITY: User" << requestingUser << "tried to edit a message they do not own (author:" << fromUser << ")";
        }
    }
}

/**
 * @brief Обрабатывает запрос от клиента на удаление ранее отправленного сообщения.
 * @details Эта функция обеспечивает безопасное удаление сообщения. Она выполняет следующие действия:
 *          1.  Определяет пользователя (`requestingUser`), отправившего запрос, по его сокету.
 *          2.  Проверяет, что пользователь аутентифицирован.
 *          3.  Находит в базе данных автора (`fromUser`) и получателя (`toUser`) сообщения по его ID.
 *          4.  Выполняет ключевую проверку безопасности: `requestingUser` должен совпадать с `fromUser`.
 *              Это гарантирует, что только автор сообщения может его удалить.
 *          5.  Если проверка пройдена, выполняет SQL-запрос `DELETE` для удаления записи из таблицы `messages`.
 *          6.  После успешного удаления формирует JSON-уведомление типа "delete_message".
 *          7.  Рассылает это уведомление обоим участникам чата (автору и получателю), если они
 *              находятся онлайн, чтобы их клиенты могли в реальном времени убрать сообщение из интерфейса.
 * @param clientSocket Сокет клиента, который инициировал удаление.
 * @param request JSON-объект от клиента, который должен содержать поле `id`
 *                с уникальным идентификатором удаляемого сообщения.
 */
void Server::handleDeleteMessage(QObject* clientSocket, const QJsonObject& request)
{
    // 1. Определяем пользователя, отправившего запрос, по сокету.
    QString requestingUser = m_clientsReverse.value(clientSocket);
    quint64 messageId = request["id"].toDouble();

    qDebug() << requestingUser << "wants to delete message with id:" << messageId;
    if (messageId == 0) return;

    // 2. Проверяем, аутентифицирован ли пользователь.
    if (requestingUser.isEmpty()) {
        qWarning() << "SECURITY: Delete request from an unauthenticated socket!";
        return;
    }

    // 3. Находим в БД автора и получателя сообщения, чтобы проверить права доступа.
    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);

    if (query.exec() && query.next()) {
        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();

        // 4. Ключевая проверка безопасности: пользователь может удалить только свое сообщение.
        if (requestingUser == fromUser) {
            // 5. Если проверка пройдена, удаляем сообщение из базы данных.
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM messages WHERE id = :id");
            deleteQuery.bindValue(":id", messageId);

            if (deleteQuery.exec()) {
                qDebug() << "[SERVER] User" << requestingUser << "deleted message" << messageId;

                // 6. Формируем уведомление об удалении для рассылки клиентам.
                QJsonObject deleteCmd;
                deleteCmd["type"] = "delete_message";
                deleteCmd["id"] = (double)messageId;

                // 7. Рассылаем уведомление автору и получателю, если они онлайн.
                QObject* fromSocket = m_clients.value(fromUser, nullptr);
                if (fromSocket) {
                    deleteCmd["with_user"] = toUser;
                    sendJson(fromSocket, deleteCmd);
                }

                QObject* toSocket = m_clients.value(toUser, nullptr);
                if (toSocket) {
                    deleteCmd["with_user"] = fromUser;
                    sendJson(toSocket, deleteCmd);
                }
            }
        } else {
            // Если пользователь пытается удалить чужое сообщение, логируем это как угрозу безопасности.
            qWarning() << "SECURITY: User" << requestingUser << "tried to delete a message they do not own (author:" << fromUser << ")";
        }
    }
}

/**
 * @brief Обрабатывает ответ пользователя на входящий запрос на добавление в контакты.
 * @details Эта функция вызывается, когда пользователь принимает ("accepted") или
 *          отклоняет ("declined") запрос на добавление в контакты от другого пользователя.
 *          1.  Определяет пользователя, который дал ответ (`toUsername`), и инициатора запроса (`fromUsername`).
 *          2.  Получает из базы данных их уникальные идентификаторы (ID).
 *          3.  В зависимости от ответа:
 *              -   Если ответ "accepted": обновляет запись в таблице `contacts`, изменяя ее статус
 *                  с `0` (pending) на `1` (accepted). После этого обоим пользователям (если они онлайн)
 *                  отправляются обновленные списки контактов.
 *              -   Если ответ "declined": полностью удаляет запись о запросе из таблицы `contacts`.
 * @param clientSocket Сокет клиента, который ответил на запрос.
 * @param request JSON-объект, содержащий:
 *                - `fromUsername`: Имя пользователя, который изначально отправил запрос.
 *                - `response`: Строка с ответом, "accepted" или "declined".
 */
void Server::handleContactRequestResponse(QObject* clientSocket, const QJsonObject& request)
{
    qDebug() << "[SERVER] Received contact_request_response:" << request;

    // 1. Определяем участников: кто ответил (`toUsername`) и кто отправлял запрос (`fromUsername`).
    QString toUsername = m_clientsReverse.value(clientSocket);
    QString fromUsername = request["fromUsername"].toString();
    QString response = request["response"].toString();

    qDebug() << "[SERVER] Parsed response value:" << response;

    // 2. Получаем ID обоих пользователей из базы данных.
    QSqlQuery idQuery;
    idQuery.prepare("SELECT id, username FROM users WHERE username = :from OR username = :to");
    idQuery.bindValue(":from", fromUsername);
    idQuery.bindValue(":to", toUsername);

    if (!idQuery.exec()) { return; }

    qint64 fromId = -1, toId = -1;
    while (idQuery.next()) {
        if (idQuery.value("username").toString() == fromUsername) {
            fromId = idQuery.value("id").toLongLong();
        } else {
            toId = idQuery.value("id").toLongLong();
        }
    }

    if (fromId == -1 || toId == -1) { return; } // Если один из пользователей не найден, выходим.

    // Для консистентности в БД храним ID в порядке возрастания.
    qint64 userId1 = std::min(fromId, toId);
    qint64 userId2 = std::max(fromId, toId);

    // 3. Обрабатываем ответ.
    if (response == "accepted") {
        // Если запрос принят, обновляем статус в таблице `contacts`.
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE contacts SET status = 1 WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        updateQuery.bindValue(":id1", userId1);
        updateQuery.bindValue(":id2", userId2);

        if (updateQuery.exec() && updateQuery.numRowsAffected() > 0) {
            qDebug() << "[SERVER]" << toUsername << "accepted contact request from" << fromUsername;

            // Находим сокеты обоих пользователей.
            QObject* fromSocket = m_clients.value(fromUsername, nullptr);
            QObject* toSocket = m_clients.value(toUsername, nullptr);

            // Отправляем обоим обновленные списки контактов, так как они изменились.
            if (fromSocket) {
                sendContactList(fromSocket, fromUsername);
            }
            if (toSocket) {
                sendContactList(toSocket, toUsername);
            }

            // Также отправляем им обновленные списки онлайн-пользователей.
            if (fromSocket){
                sendOnlineStatusList(fromSocket);
            }
            if (toSocket){
                sendOnlineStatusList(toSocket);
            }
        }
    } else if (response == "declined") {
        // Если запрос отклонен, просто удаляем его из таблицы.
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM contacts WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        deleteQuery.bindValue(":id1", userId1);
        deleteQuery.bindValue(":id2", userId2);

        if (deleteQuery.exec()) {
            qDebug() << "[SERVER]" << toUsername << "declined contact request from" << fromUsername;
        }
    }
}

/**
 * @brief Отправляет клиенту счетчики непрочитанных сообщений, сгруппированные по отправителям.
 * @details Эта функция вызывается после успешного входа пользователя в систему. Она выполняет
 *          SQL-запрос к таблице `messages`, чтобы найти все сообщения, адресованные
 *          данному пользователю (`username`), которые еще не были помечены как прочитанные
 *          (`is_read = 0`). Результаты группируются по отправителю (`fromUser`),
 *          и для каждого отправителя подсчитывается количество таких сообщений.
 *          Сформированный список (пользователь + количество) отправляется клиенту
 *          в виде JSON-сообщения типа "unread_counts". Это позволяет клиентскому
 *          приложению отображать "бейджи" с количеством новых сообщений рядом с
 *          именами контактов.
 * @param socket Сокет клиента, которому необходимо отправить счетчики.
 * @param username Имя пользователя, для которого производится подсчет.
 */
void Server::sendUnreadCounts(QObject* socket, const QString& username)
{
    qDebug() << "[SERVER][UNREAD] Собираем счетчики непрочитанных для пользователя:" << username;

    // 1. Получаем ID пользователя (хотя он и не используется в основном запросе, это хорошая практика).
    QSqlQuery idQuery;
    idQuery.prepare("SELECT id FROM users WHERE username = :username");
    idQuery.bindValue(":username", username);
    if (!idQuery.exec() || !idQuery.next()) {
        qDebug() << "[SERVER][UNREAD][ERROR] Не удалось найти ID для пользователя:" << username;
        return;
    }
    qint64 userId = idQuery.value(0).toLongLong();
    Q_UNUSED(userId);
    // 2. Основной запрос: подсчитываем непрочитанные сообщения (`is_read = 0`)
    //    и группируем их по отправителю (`fromUser`).
    QSqlQuery query;
    query.prepare(
        "SELECT fromUser, COUNT(*) as unread_count "
        "FROM messages "
        "WHERE toUser = :username AND is_read = 0 "
        "GROUP BY fromUser"
        );
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "[SERVER][UNREAD][ERROR] Ошибка при запросе к БД:" << query.lastError().text();
        return;
    }

    // 3. Формируем JSON-массив с результатами.
    QJsonArray countsArray;
    while (query.next()) {
        QJsonObject countObject;
        countObject["username"] = query.value("fromUser").toString();
        countObject["count"] = query.value("unread_count").toInt();
        countsArray.append(countObject);
    }

    // Если непрочитанных сообщений нет, просто выходим.
    if (countsArray.isEmpty()) {
        qDebug() << "[SERVER][UNREAD] Непрочитанных сообщений для" << username << "не найдено.";
        return;
    }

    // 4. Упаковываем массив в JSON-объект и отправляем клиенту.
    QJsonObject response;
    response["type"] = "unread_counts";
    response["counts"] = countsArray;

    qDebug() << "[SERVER][UNREAD] Отправка счетчиков для" << username << ":" << response;
    sendJson(socket, response);
}

/**
 * @brief Отправляет список онлайн-пользователей одному конкретному клиенту.
 * @details В отличие от `broadcastUserList`, которая рассылает список всем, эта
 *          функция отправляет тот же самый список онлайн-пользователей только
 *          одному клиенту, указанному через `clientSocket`. Это может быть полезно
 *          в ситуациях, когда необходимо обновить онлайн-статусы для одного
 *          клиента, не затрагивая остальных, например, после того как он принял
 *          чей-то запрос в друзья.
 * @param clientSocket Сокет клиента, которому нужно отправить список.
 */
void Server::sendOnlineStatusList(QObject* clientSocket)
{
    // 1. Получаем список всех онлайн-пользователей из ключей карты m_clients.
    QStringList onlineUsers = m_clients.keys();

    // 2. Создаем JSON-сообщение.
    QJsonObject message;
    message["type"] = "user_list";
    message["users"] = QJsonArray::fromStringList(onlineUsers);

    // 3. Отправляем сообщение указанному клиенту.
    sendJson(clientSocket, message);
    qDebug() << "[SERVER] Sent online status list to a single client.";
}

/**
 * @brief Находит и отправляет пользователю список ожидающих подтверждения запросов в контакты.
 * @details Эта функция является частью процесса инициализации сессии пользователя после
 *          успешного входа. Она выполняет сложный SQL-запрос, который находит все
 *          записи в таблице `contacts`, где текущий пользователь является одним из
 *          участников, а статус записи равен `0` (pending/ожидает).
 *          С помощью `JOIN` и `CASE` запрос извлекает `username` и `display_name`
 *          другого пользователя (того, кто отправил запрос).
 *          Если такие запросы найдены, они упаковываются в JSON-массив и отправляются
 *          клиенту сообщением с типом "pending_requests_list", чтобы он мог их
 *          принять или отклонить.
 * @param socket Сокет клиента, для которого выполняется проверка.
 * @param username Имя пользователя, для которого нужно найти запросы.
 */
void Server::sendPendingContactRequests(QObject* socket, const QString& username){
    qDebug() << "[SERVER][PENDING] Checking for pending requests for user:" << username;
    // 1. Находим ID текущего пользователя.
    QSqlQuery userQuery;
    userQuery.prepare("SELECT id FROM users WHERE username = :username");
    userQuery.bindValue(":username", username);
    if (!userQuery.exec() || !userQuery.next()) {
        qDebug() << "[SERVER][PENDING][ERROR] Could not find ID for user:" << username;
        return;
    }
    qint64 userId = userQuery.value(0).toLongLong();
    qDebug() << "[SERVER][PENDING] User ID is:" << userId;

    // 2. Основной, сложный запрос для поиска ожидающих запросов.
    QSqlQuery query;
    query.prepare(
        "SELECT u.username, u.display_name FROM users u "
        // Соединяем с таблицей `contacts`, находя ID "другого" пользователя в паре.
        "JOIN contacts c ON u.id = (CASE WHEN c.user_id_1 = :userId THEN c.user_id_2 ELSE c.user_id_1 END) "
        // Ищем все записи, где участвует наш пользователь и статус "pending".
        "WHERE (c.user_id_1 = :userId OR c.user_id_2 = :userId) AND c.status = 0"
        );
    query.bindValue(":userId", QVariant(userId));

    if (!query.exec()) {
        qDebug() << "[SERVER][PENDING][ERROR] DB Error: Failed to fetch pending requests:" << query.lastError().text();
        return;
    }
    qDebug() << "[SERVER][PENDING] Main SQL query executed successfully. Processing results...";

    // 3. Собираем результаты в JSON-массив.
    QJsonArray pendingRequests;
    while (query.next()) {
        QString fromUser = query.value(0).toString();
        qDebug() << "[SERVER][PENDING] Found a pending request from:" << fromUser;

        QJsonObject reqObject;
        reqObject["fromUsername"] = fromUser;
        reqObject["fromDisplayname"] = query.value(1).toString();
        pendingRequests.append(reqObject);
    }
    qDebug() << "[SERVER][PENDING] Total pending requests found:" << pendingRequests.count();

    // 4. Если найден хотя бы один запрос, отправляем их клиенту.
    if (!pendingRequests.isEmpty()) {
        qDebug() << "[SERVER] Sending" << pendingRequests.count() << "pending contact requests to" << username;
        QJsonObject response;
        response["type"] = "pending_requests_list";
        response["requests"] = pendingRequests;
        sendJson(socket, response);
    }
    else {
        qDebug() << "[SERVER][PENDING] No requests to send. Function finished.";
    }
}
void Server::handleCallRequest(QObject* socket, const QJsonObject& request)

{
    qDebug() << "[SERVER] Processing message of type: call_request";
    QString fromUser = request["from"].toString();
    QString toUser = request["to"].toString();
    QString callId = request["call_id"].toString();
    quint16 callerPort = request["caller_port"].toInt();

    QString callerIp = request["caller_ip"].toString(); //static_cast<QTcpSocket*>(socket)->peerAddress().toString();
    QObject* toUserSocket = m_clients.value(toUser, nullptr);

    createCallRecord(callId, fromUser, toUser, callerIp, callerPort);

    CallInfo callInfo;
    callInfo.callId = callId;
    callInfo.from = fromUser;
    callInfo.to = toUser;
    callInfo.fromSocket = socket;
    callInfo.toSocket = toUserSocket;
    callInfo.callerPort = callerPort;
    callInfo.callerIp = callerIp;
    m_activeCalls[callId] = callInfo;

    // Отправляем "call_request" получателю (B)
    QJsonObject incomingCall;
    incomingCall["type"] = "call_request";
    incomingCall["from"] = fromUser;
    incomingCall["call_id"] = callId;
    incomingCall["caller_ip"] = callerIp;
    incomingCall["caller_port"] = (int)callerPort;

    if(toUserSocket){
        sendJson(toUserSocket, incomingCall);
        qDebug() << "[CALL] Forwarded to" << toUser;
    } else{
        updateCallEnded(callId, "missed");
        qDebug() << "[CALL]" << toUser << "is offline - marked as missed";
    }
}

void Server::handleCallAccepted(QObject* socket, const QJsonObject& request)
{
    qDebug() << "[SERVER] Processing message of type: call_accepted";

    QString respondingUser = m_clientsReverse.value(socket); // кто отправляет
    QString callId = request.value("call_id").toString();
    quint16 calleePort = request.value("callee_port").toInt();
    QString calleeIp = request.value("callee_ip").toString(); //static_cast<QTcpSocket*>(info.toSocket)->peerAddress().toString();
    // Защита
    if (!m_activeCalls.contains(callId)) {
        qWarning() << "[SERVER] Unknown call id!";
        return;
    }
    updateCallConnected(callId, calleeIp, calleePort);

    CallInfo& info = m_activeCalls[callId];



    // Нужен инициатор звонка (A):
    QObject* initiatorSocket = info.fromSocket;
    if (initiatorSocket) {
        QJsonObject response;
        response["type"] = "call_accepted";
        response["from"] = respondingUser;
        response["call_id"] = callId;
        response["callee_ip"] = calleeIp;
        response["callee_port"] = (int)calleePort;

        sendJson(initiatorSocket, response);
    }
}

void Server::handleCallRejected(QObject* socket, const QJsonObject& request)
{
    QString callId = request["call_id"].toString();
    QString toUser = request["to"].toString();

    if (!m_activeCalls.contains(callId)) {
        qWarning() << "[SERVER] Call rejected: unknown call ID" << callId;
        return;
    }
    updateCallEnded(callId, "rejected");

    CallInfo callInfo = m_activeCalls[callId];
    QString fromUser = callInfo.from;
    QObject* fromUserSocket = callInfo.fromSocket;

    // Проверка безопасности
    if (toUser != m_clientsReverse.value(socket)) {
        qWarning() << "[SERVER] SECURITY: Unauthorized call rejection attempt";
        return;
    }

    qDebug() << "[SERVER] Call rejected:" << fromUser << "<-" << toUser
             << "| callId:" << callId;

    // Удаляем звонок из активных
    m_activeCalls.remove(callId);

    // Уведомляем инициатора об отклонении
    QJsonObject rejectionNotification;
    rejectionNotification["type"] = "call_rejected";
    rejectionNotification["call_id"] = callId;
    rejectionNotification["from"] = toUser;

    sendJson(fromUserSocket, rejectionNotification);

    qDebug() << "[SERVER] Call rejection notification sent to" << fromUser;
}

void Server::handleCallEnd(QObject* socket, const QJsonObject& request)
{
    QString callId = request["call_id"].toString();
    QString toUser = request["to"].toString();
    updateCallEnded(callId, "completed");

    if (!m_activeCalls.contains(callId)) {
        qDebug() << m_activeCalls.keys();
        qWarning() << "[SERVER] Call end: unknown call ID" << callId;
        return;
    }

    CallInfo callInfo = m_activeCalls[callId];
    QString currentUser = m_clientsReverse.value(socket);
    QObject* otherSocket = nullptr;

    // Определяем, кто завершает звонок, и находим "другую сторону"
    if (currentUser == callInfo.from) {
        otherSocket = callInfo.toSocket;
    } else if (currentUser == callInfo.to) {
        otherSocket = callInfo.fromSocket;
    } else {
        qWarning() << "[SERVER] SECURITY: Unauthorized call end attempt";
        return;
    }

    qDebug() << "[SERVER] Call ended:" << currentUser << "| callId:" << callId;

    // Удаляем звонок из активных
    m_activeCalls.remove(callId);

    // Уведомляем другую сторону, что звонок завершен
    QJsonObject endNotification;
    endNotification["type"] = "call_end";
    endNotification["call_id"] = callId;
    endNotification["from"] = currentUser;

    if (otherSocket != nullptr) {
        sendJson(otherSocket, endNotification);
    }

    qDebug() << "[SERVER] Call end notification sent";
}

/**
 * @brief Создает новую запись звонка в БД при поступлении call_request
 */
void Server::createCallRecord(const QString& callId, const QString& from,
                              const QString& to, const QString& fromIp, quint16 fromPort)
{
    QSqlQuery query;
    query.prepare("INSERT INTO call_history "
                  "(call_id, caller_username, callee_username, status, start_time, "
                  "caller_ip, caller_port) "
                  "VALUES (:callId, :from, :to, 'ringing', :startTime, :fromIp, :fromPort)");

    query.bindValue(":callId", callId);
    query.bindValue(":from", from);
    query.bindValue(":to", to);
    query.bindValue(":startTime", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":fromIp", fromIp);
    query.bindValue(":fromPort", fromPort);

    if (!query.exec()) {
        qWarning() << "[CALL] DB Error creating call record:" << query.lastError().text();
    } else {
        qDebug() << "[CALL] Created call record:" << callId;
    }
}

/**
 * @brief Обновляет запись при принятии звонка (call_accepted)
 */
void Server::updateCallConnected(const QString& callId, const QString& toIp, quint16 toPort)
{
    QSqlQuery query;
    query.prepare("UPDATE call_history "
                  "SET status = 'connected', "
                  "    connect_time = :connectTime, "
                  "    callee_ip = :toIp, "
                  "    callee_port = :toPort "
                  "WHERE call_id = :callId");

    query.bindValue(":callId", callId);
    query.bindValue(":connectTime", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":toIp", toIp);
    query.bindValue(":toPort", toPort);

    if (!query.exec()) {
        qWarning() << "[CALL] DB Error updating call connected:" << query.lastError().text();
    } else {
        qDebug() << "[CALL] Updated call as connected:" << callId;
    }
}

/**
 * @brief Завершает звонок с расчетом длительности
 */
void Server::updateCallEnded(const QString& callId, const QString& status)
{
    QSqlQuery query;
    query.prepare("UPDATE call_history "
                  "SET status = :status, "
                  "    end_time = :endTime, "
                  "    duration_seconds = "
                  "      CAST((julianday(:endTime) - julianday(connect_time)) * 86400 AS INTEGER) "
                  "WHERE call_id = :callId");

    query.bindValue(":callId", callId);
    query.bindValue(":status", status);
    query.bindValue(":endTime", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qWarning() << "[CALL] DB Error updating call ended:" << query.lastError().text();
    } else {
        qDebug() << "[CALL] Updated call as" << status << ":" << callId;
    }
}

void Server::handleGetCallHistory(QObject* socket, const QJsonObject& request)
{
    QString username = request["username"].toString();

    QSqlQuery query;
    query.prepare("SELECT call_id, caller_username, callee_username, status, "
                  "       start_time, end_time, duration_seconds "
                  "FROM call_history "
                  "WHERE caller_username = :user OR callee_username = :user "
                  "ORDER BY start_time DESC LIMIT 50");
    query.bindValue(":user", username);

    if (!query.exec()) {
        qWarning() << "[CALL] Error fetching call history:" << query.lastError().text();
        sendJson(socket, {{"type", "error"}, {"reason", "Failed to fetch history"}});
        return;
    }

    QJsonArray calls;
    while (query.next()) {
        QJsonObject call;
        call["call_id"] = query.value("call_id").toString();
        call["caller"] = query.value("caller_username").toString();
        call["callee"] = query.value("callee_username").toString();
        call["status"] = query.value("status").toString();
        call["start_time"] = query.value("start_time").toString();
        call["end_time"] = query.value("end_time").toString();
        call["duration_seconds"] = query.value("duration_seconds").toInt();
        call["call_type"] = (username == query.value("caller_username").toString()) ? "outgoing" : "incoming";

        calls.append(call);
    }

    QJsonObject response;
    response["type"] = "call_history";
    response["calls"] = calls;
    sendJson(socket, response);

    qDebug() << "[CALL] Sent call history to" << username << ":" << calls.size() << "records";
}

void Server::handleGetCallStats(QObject* socket, const QJsonObject& request)
{
    QString username = request["username"].toString();

    QSqlQuery query;
    query.prepare("SELECT "
                  "  COUNT(*) FILTER (WHERE caller_username = :user) as outgoing_count, "
                  "  COUNT(*) FILTER (WHERE callee_username = :user) as incoming_count, "
                  "  COUNT(*) FILTER (WHERE status = 'completed') as completed_count, "
                  "  COUNT(*) FILTER (WHERE status = 'missed' AND callee_username = :user) as missed_count, "
                  "  SUM(CASE WHEN status = 'completed' THEN duration_seconds ELSE 0 END) as total_duration "
                  "FROM call_history "
                  "WHERE caller_username = :user OR callee_username = :user");

    query.bindValue(":user", username);

    if (!query.exec() || !query.next()) {
        sendJson(socket, {{"type", "error"}});
        return;
    }

    QJsonObject stats;
    stats["type"] = "call_stats";
    stats["outgoing"] = query.value("outgoing_count").toInt();
    stats["incoming"] = query.value("incoming_count").toInt();
    stats["completed"] = query.value("completed_count").toInt();
    stats["missed"] = query.value("missed_count").toInt();
    stats["total_duration_sec"] = query.value("total_duration").toInt();

    sendJson(socket, stats);
}

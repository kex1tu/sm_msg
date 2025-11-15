#include "databaseservice.h"
#include <QSqlTableModel>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>

DatabaseService::DatabaseService(QObject *parent)
    : QObject(parent), m_initialized(false) {}


DatabaseService::~DatabaseService() {
    close();
}


bool DatabaseService::initialize(const QString &dbPath)
{
    // Диагностика окружения: выводит все драйверы SQL, которые видны для Qt — удобно для поиска причин ошибок.
    qDebug() << "[DatabaseService] Available SQL drivers:" << QSqlDatabase::drivers();

    // Создаём/подключаем SQLite базу; используем addDatabase("QSQLITE") (один экземпляр для всего приложения).
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // Формируем абсолютный путь к файлу БД в каталоге приложения (при необходимости можно заменить на dbPath).
    QString fullPath = QCoreApplication::applicationDirPath() + "/database.db";
    m_db.setDatabaseName(fullPath);

    // Выводим путь к базе для быстрой диагностики, при проблемах — это поможет выявить причину.
    qDebug() << "[DatabaseService] Trying to open:" << fullPath;

    // Пробуем открыть базу; если не удалось — формируем подробный текст ошибки и отправляем сигнал для UI и отладки.
    if (!m_db.open()) {
        QString error = "[DatabaseService] ERROR: Cannot open database: " + m_db.lastError().text();
        qDebug() << error;
        emit databaseError(error);
        return false;
    }

    // Успешное открытие — выводим сообщение для лога, отладчик увидит, что соединение установлено.
    qDebug() << "[DatabaseService] Database opened OK";

    // Проверяем наличие всех необходимых таблиц, если нет — создаём (см. реализацию createTables).
    if (!createTables()) {
        qDebug() << "[DatabaseService] ERROR: Failed to create tables";
        return false;
    }

    // Миграция схемы: добавление новых колонок, исправление ошибок — отдельная функция для удобства поддержки.
    if (!migrateDatabase()) {
        qDebug() << "[DatabaseService] WARNING: Migration issues detected, continuing ...";
    }

    // Устанавливаем флаг успешной инициализации и выводим статистику базы в лог (количество записей, структур).
    m_initialized = true;
    printDatabaseStats();

    // Возвращаем истину — база полностью готова к работе.
    return true;
}


void DatabaseService::close() {
    // Проверяем текущее состояние; если соединение уже закрыто — выход.
    if (m_db.isOpen()) {
        // Закрытие physically disconnects DB-файл, освобождает ресурсы Qt/OS.
        m_db.close();
        // Флаг инициализации сбрасываем, чтобы сервис нельзя было использовать до новой инициализации.
        m_initialized = false;
        // Отладочная информация: база успешно закрыта.
        qDebug() << "[DatabaseService] Database closed";
    }
}


bool DatabaseService::isConnected() const {
    // Проверяем два условия: база должна быть открыта и флаг инициализации должен быть выставлен.
    return m_db.isOpen() && m_initialized;
}


bool DatabaseService::createTables() {
    QSqlQuery query(m_db);

    // Формируем SQL-запрос для создания основной таблицы сообщений:
    // все основные поля чата, UUID, служебные — используют стандартные типы SQLite.
    QString createMessagesTable = R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            server_id INTEGER UNIQUE,
            temp_id TEXT,
            from_user TEXT NOT NULL,
            to_user TEXT NOT NULL,
            payload TEXT NOT NULL,
            timestamp TEXT,
            server_timestamp TEXT,
            status INTEGER DEFAULT 0,
            is_edited INTEGER DEFAULT 0,
            reply_to_id INTEGER DEFAULT 0,
            is_outgoing INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    // Выполняем запрос на создание; если ошибка — логгируем причину и возвращаем false.
    if (!query.exec(createMessagesTable)) {
        qDebug() << "[DatabaseService] ERROR: Failed to create 'messages' table:" << query.lastError().text();
        return false;
    }
    // Успешное создание — лог для аудита процесса.
    qDebug() << "[DatabaseService] Table 'messages' is OK";

    // Групповой запрос для создания индексов: ускоряют выборки в диалогах, сортировку, фильтрацию статусов.
    QString createIndexes = R"(
        CREATE INDEX IF NOT EXISTS idx_from_to ON messages(from_user, to_user);
        CREATE INDEX IF NOT EXISTS idx_server_id ON messages(server_id);
        CREATE INDEX IF NOT EXISTS idx_temp_id ON messages(temp_id);
        CREATE INDEX IF NOT EXISTS idx_status ON messages(status);
    )";
    // По каждому индексу отдельно проверяем успешность — если возникают ошибки, выводим предупреждение.
    for (const QString &indexQuery : createIndexes.split(";")) {
        if (!indexQuery.trimmed().isEmpty()) {
            if (!query.exec(indexQuery)) {
                qDebug() << "[DatabaseService] WARNING: Index creation issue:" << query.lastError().text();
            }
        }
    }
    qDebug() << "[DatabaseService] Indexes are OK";

    // Запрос для создания таблицы чатов: Хранит пары пользователей, последние сообщения, уникальность по (user1,user2).
    QString createChatsTable = R"(
        CREATE TABLE IF NOT EXISTS chats (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user1 TEXT NOT NULL,
            user2 TEXT NOT NULL,
            last_message_id INTEGER,
            last_activity DATETIME DEFAULT CURRENT_TIMESTAMP,
            UNIQUE(user1, user2)
        );
    )";
    // Ошибка при создании чатов — логгируем и завершаем функцию.
    if (!query.exec(createChatsTable)) {
        qDebug() << "[DatabaseService] ERROR: Failed to create 'chats' table:" << query.lastError().text();
        return false;
    }
    qDebug() << "[DatabaseService] Table 'chats' is OK";

    // Таблица контактов: используется для хранения информации о каждом зарегистрированном пользователе.
    QString createContactsTable = R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            display_name TEXT,
            is_online INTEGER DEFAULT 0,
            last_seen TEXT,
            avatar_url TEXT,
            status_message TEXT,
            synced_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    // Проверяем успешность создания, логгируем финальное состояние.
    if (!query.exec(createContactsTable)) {
        qDebug() << "[DatabaseService] ERROR: Failed to create 'contacts' table:" << query.lastError().text();
        return false;
    }
    qDebug() << "[DatabaseService] Table 'contacts' is OK";

    // Все этапы успешны — возвращаем true.
    return true;
}


bool DatabaseService::migrateDatabase() {
    QSqlQuery query(m_db);

    // Получаем список всех колонок через PRAGMA table_info (SQLite специфично)
    QString checkColumn = "PRAGMA table_info(messages)";
    if (query.exec(checkColumn)) {
        QStringList columns;
        // Перебираем результат запроса — name хранения в value(1)
        while (query.next()) columns << query.value(1).toString();

        // Проверяем, есть ли колонка server_timestamp — если нет, то надо добавить.
        if (!columns.contains("server_timestamp")) {
            // Пытаемся добавить новую колонку через ALTER TABLE
            if (query.exec("ALTER TABLE messages ADD COLUMN server_timestamp TEXT")) {
                // Миграция успешна, выводим диагностическое сообщение
                qDebug() << "[DatabaseService] Migration: 'server_timestamp' added";
            } else {
                // Ошибка добавления — записываем причину
                qDebug() << "[DatabaseService] ERROR: Failed to migrate 'server_timestamp':" << query.lastError().text();
            }
        }
    }

    // Возвращаем истину — дальнейшие функции могут работать с обновлённой или даже частично изменённой структурой.
    return true;
}


bool DatabaseService::saveMessage(const ChatMessage &msg, const QString &currentUsername) {
    // Контроль подключения — операции над базой невозможны без рабочей сессии.
    if (!isConnected()) {
        qDebug() << "[DatabaseService] ERROR: Not connected";
        return false;
    }

    // Готовим SQL INSERT-запрос для добавления записи; используем именованные параметры для читаемости и безопасности.
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR IGNORE INTO messages (
            server_id, temp_id, from_user, to_user, payload,
            timestamp, server_timestamp, status, is_edited, reply_to_id, is_outgoing
        ) VALUES (
            :server_id, :temp_id, :from_user, :to_user, :payload,
            :timestamp, :server_timestamp, :status, :is_edited, :reply_to_id, :is_outgoing
        )
    )");

    // Привязка значений к параметрам запроса; если id==0, используется tempId для "предвременной" отправки.
    query.addBindValue(msg.id > 0 ? msg.id : QVariant()); // server_id (может быть пустым для новых/неподтверждённых сообщений)
    query.addBindValue(msg.tempId.isEmpty() ? QVariant() : msg.tempId); // temp_id — для клиентской синхронизации
    query.addBindValue(msg.fromUser);    // имя отправителя
    query.addBindValue(msg.toUser);      // имя получателя
    query.addBindValue(msg.payload);     // текст сообщения
    query.addBindValue(msg.timestamp);   // локальное время отправки
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));  // server_timestamp (полученное/текущее сервера)
    query.addBindValue((int)msg.status); // статус отправки/доставки
    query.addBindValue(msg.isEdited ? 1 : 0); // флаг редактирования
    query.addBindValue(msg.replyToId);       // ID исходного сообщения, если это reply
    query.addBindValue(msg.isOutgoing ? 1 : 0); // флаг направления

    // Выполняем INSERT, контролируем ошибки (например, нарушение уникальности, проблема записи, сбой БД).
    if (!query.exec()) {
        QString error = "[DatabaseService] ERROR: Failed to insert message:" + query.lastError().text();
        // Сигнализируем ошибку для UI или логгирования, возвращаем false.
        qDebug() << error;
        emit databaseError(error);
        return false;
    }

    // Успешный INSERT — выводим в лог id и первые 50 символов payload для дальнейшей проверки.
    qDebug() << "[DatabaseService] Message saved, id:" << msg.id << "payload:" << msg.payload.left(50);
    return true;
}


bool DatabaseService::updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus status) {
    // Проверяем, что есть живое соединение с БД
    if (!isConnected()) return false;

    // Готовим SQL-запрос на обновление — изменяем статус и время обновления для конкретного сообщения
    QSqlQuery query(m_db);
    query.prepare("UPDATE messages SET status = :status, updated_at = CURRENT_TIMESTAMP WHERE server_id = :id");
    query.addBindValue((int)status);
    query.addBindValue(messageId);

    // Исполняем запрос; в случае ошибки возвращаем false и логгируем её
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to update status for" << messageId << ":" << query.lastError().text();
        return false;
    }

    // Запись успешного обновления для последующей отладки и мониторинга пользовательских событий
    qDebug() << "[DatabaseService] Status updated for id:" << messageId << "to status:" << (int)status;
    return true;
}


bool DatabaseService::updateAllMessagesStatusForChat(const QString &withUser, const QString &currentUsername,
                                                     ChatMessage::MessageStatus status) {
    // Блокировка операции при отсутствии соединения
    if (!isConnected()) return false;

    // Массовый UPDATE всех сообщений c данным from_user и to_user, удовлетворяющих условию по статусу
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE messages
        SET status = :status, updated_at = CURRENT_TIMESTAMP
        WHERE from_user = :from_user AND to_user = :to_user AND status < :status
    )");
    query.addBindValue((int)status);
    query.addBindValue(withUser);
    query.addBindValue(currentUsername);
    query.addBindValue((int)status);

    // Проверка успешности выполнения запроса
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to bulk-update status:" << query.lastError().text();
        return false;
    }

    // Запись успешной массовой операции для отладки
    qDebug() << "[DatabaseService] Bulk status updated for chat:" << withUser;
    return true;
}


QList<ChatMessage> DatabaseService::loadRecentMessages(const QString &fromUser, const QString &toUser, int limit) {
    QList<ChatMessage> messages;
    // Проверяем соединение с базой — во избежание некорректных операций с памятью
    if (!isConnected()) {
        qDebug() << "[DatabaseService] ERROR: Not connected";
        return messages;
    }

    // Формируем запрос: все сообщения между fromUser и toUser (в обе стороны), сортировка по времени, лимит
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT server_id, temp_id, from_user, to_user, payload, timestamp,
               status, is_edited, reply_to_id, is_outgoing
        FROM messages
        WHERE (from_user = :user1 AND to_user = :user2) OR
              (from_user = :user2 AND to_user = :user1)
        ORDER BY timestamp DESC
        LIMIT :limit
    )");
    query.addBindValue(fromUser);
    query.addBindValue(toUser);
    query.addBindValue(limit);

    // Пытаемся выполнить запрос; в случае ошибки — логгируем причину, возвращаем пустой результат
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to load recent messages:" << query.lastError().text();
        return messages;
    }

    // Формируем результат — по каждой строке формируем ChatMessage, заполняем поля
    QList<ChatMessage> tempMessages;
    while (query.next()) {
        ChatMessage msg;
        msg.id = query.value(0).toLongLong();
        msg.tempId = query.value(1).toString();
        msg.fromUser = query.value(2).toString();
        msg.toUser = query.value(3).toString();
        msg.payload = query.value(4).toString();
        msg.timestamp = query.value(5).toString();
        msg.status = (ChatMessage::MessageStatus)query.value(6).toInt();
        msg.isEdited = query.value(7).toInt() == 1;
        msg.replyToId = query.value(8).toLongLong();
        msg.isOutgoing = query.value(9).toInt() == 1;
        tempMessages.prepend(msg); // prepend для правильного хронологического порядка
    }
    // Логгируем итоговое число выбранных сообщений
    qDebug() << "[DatabaseService] Loaded" << tempMessages.size() << "recent messages for chat";
    return tempMessages;
}


QList<ChatMessage> DatabaseService::loadOlderMessages(const QString &fromUser, const QString &toUser,
                                                      qint64 beforeId, int limit) {
    QList<ChatMessage> messages;
    // Контроль соединения с БД. Без этого нельзя выполнять запросы
    if (!isConnected()) return messages;

    // Готовим выборку: между двумя пользователями в любом направлении, id < beforeId (старые), лимит и сортировка
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT server_id, temp_id, from_user, to_user, payload, timestamp,
               status, is_edited, reply_to_id, is_outgoing
        FROM messages
        WHERE ((from_user = :user1 AND to_user = :user2) OR
               (from_user = :user2 AND to_user = :user1))
              AND server_id < :before_id
        ORDER BY timestamp DESC
        LIMIT :limit
    )");
    query.addBindValue(fromUser);
    query.addBindValue(toUser);
    query.addBindValue(beforeId);
    query.addBindValue(limit);

    // Выполнение SQL-запроса; при ошибке — детальное сообщение
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to load older messages:" << query.lastError().text();
        return messages;
    }

    // Преобразуем результат в список объектов ChatMessage (хронологический порядок через prepend)
    QList<ChatMessage> tempMessages;
    while (query.next()) {
        ChatMessage msg;
        msg.id = query.value(0).toLongLong();
        msg.tempId = query.value(1).toString();
        msg.fromUser = query.value(2).toString();
        msg.toUser = query.value(3).toString();
        msg.payload = query.value(4).toString();
        msg.timestamp = query.value(5).toString();
        msg.status = (ChatMessage::MessageStatus)query.value(6).toInt();
        msg.isEdited = query.value(7).toInt() == 1;
        msg.replyToId = query.value(8).toLongLong();
        msg.isOutgoing = query.value(9).toInt() == 1;
        tempMessages.prepend(msg);
    }
    // Логгируем загрузку: сколько сообщений загружено — для профилирования и отладки динамики чата
    qDebug() << "[DatabaseService] Loaded" << tempMessages.size() << "older messages for chat";
    return tempMessages;
}


QList<ChatMessage> DatabaseService::loadMessagesForUser(const QString &currentUsername) {
    QList<ChatMessage> messages;
    // Проверка подключения — чтобы не было крашей при ошибках работы с БД
    if (!isConnected()) return messages;

    // Готовим запрос: забрать все сообщения, где пользователь встречается как from_user или to_user
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT server_id, temp_id, from_user, to_user, payload, timestamp,
               status, is_edited, reply_to_id, is_outgoing
        FROM messages
        WHERE from_user = :username OR to_user = :username
        ORDER BY timestamp DESC
    )");
    query.addBindValue(currentUsername);

    // Исполнение запроса, контроль ошибок выборки
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to load messages for user:" << query.lastError().text();
        return messages;
    }

    // Обходим все строки результата и собираем их в итоговый список (append — сохранит сортировку DESC)
    while (query.next()) {
        ChatMessage msg;
        msg.id = query.value(0).toLongLong();
        msg.tempId = query.value(1).toString();
        msg.fromUser = query.value(2).toString();
        msg.toUser = query.value(3).toString();
        msg.payload = query.value(4).toString();
        msg.timestamp = query.value(5).toString();
        msg.status = (ChatMessage::MessageStatus)query.value(6).toInt();
        msg.isEdited = query.value(7).toInt() == 1;
        msg.replyToId = query.value(8).toLongLong();
        msg.isOutgoing = query.value(9).toInt() == 1;
        messages.append(msg);
    }
    // Логгируем итог: сколько всего найдено сообщений для пользователя (удобно для тестов и UI)
    qDebug() << "[DatabaseService] Loaded" << messages.size() << "messages for user" << currentUsername;
    return messages;
}


qint64 DatabaseService::getOldestMessageId(const QString &fromUser, const QString &toUser) {
    // Контроль соединения — возврат -1, если нельзя делать запросы к БД
    if (!isConnected()) return -1;

    // Запрос: находим минимальный server_id, используя параметры обоих участников (из and to)
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT MIN(server_id) FROM messages
        WHERE (from_user = :user1 AND to_user = :user2) OR (from_user = :user2 AND to_user = :user1)
    )");
    query.addBindValue(fromUser);
    query.addBindValue(toUser);

    // Если запрос выполнен и есть результат — берем значение, иначе -1
    if (query.exec() && query.next()) {
        qint64 result = query.value(0).toLongLong();
        // Логгируем найденное минимальное значение
        qDebug() << "[DatabaseService] Oldest message ID for chat:" << result;
        return result;
    }
    return -1;
}


bool DatabaseService::deleteMessage(qint64 messageId) {
    // Контроль подключения (базы может не быть, приложение может быть в состоянии перезапуска)
    if (!isConnected()) return false;

    // Готовим DELETE-запрос с bind-параметром (безопасно для любых id)
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM messages WHERE server_id = :id");
    query.addBindValue(messageId);

    // Исполнение, обработка ошибок
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to delete message:" << query.lastError().text();
        return false;
    }
    // Лог успешного удаления (id)
    qDebug() << "[DatabaseService] Message deleted, id:" << messageId;
    return true;
}


bool DatabaseService::editMessage(qint64 messageId, const QString &newPayload) {
    // Защита от отсутствия соединения с базой
    if (!isConnected()) return false;

    // Обновление payload, отметка что сообщение отредактировано, обновление времени
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE messages SET payload = :payload, is_edited = 1, updated_at = CURRENT_TIMESTAMP WHERE server_id = :id
    )");
    query.addBindValue(newPayload);
    query.addBindValue(messageId);

    // Проверка ошибки выполнения, возврат false если не выполнилось
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to edit message:" << query.lastError().text();
        return false;
    }
    // Лог успешного редактирования для аудита и истории изменений
    qDebug() << "[DatabaseService] Message edited, id:" << messageId;
    return true;
}


int DatabaseService::getUnreadCountForChat(const QString &fromUser, const QString &currentUsername) {
    // Проверка наличия подключения к БД
    if (!isConnected()) return 0;

    // SQL-запрос для подсчёта сообщений с статусом ниже 3 (не прочитано)
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT COUNT(*) FROM messages
        WHERE from_user = :from_user AND to_user = :to_user AND status < 3
    )");
    query.addBindValue(fromUser);
    query.addBindValue(currentUsername);

    // Если запрос прошёл успешно и есть результат — возвращаем число сообщений
    if (query.exec() && query.next())
        return query.value(0).toInt();

    return 0;
}


QMap<QString, int> DatabaseService::getAllUnreadCounts(const QString &currentUsername) {
    QMap<QString, int> unreadCounts;
    // Контроль подключения с быстрым выходом (пустой результат)
    if (!isConnected()) return unreadCounts;

    // SQL: подсчёт всех непрочитанных от разных пользователей (to_user = текущий пользователь, группируем по from_user)
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT from_user, COUNT(*) as count
        FROM messages
        WHERE to_user = :to_user AND status < 3
        GROUP BY from_user
    )");
    query.addBindValue(currentUsername);

    // Каждая строка результата — пара (user, count), записываем в результативный QMap
    if (query.exec()) {
        while (query.next()) {
            QString user = query.value(0).toString();
            int count = query.value(1).toInt();
            unreadCounts[user] = count;
        }
    }
    // Для информации в лог — сколько диалогов содержит непрочитанные сообщения
    qDebug() << "[DatabaseService] Loaded unread counts for" << unreadCounts.size() << "chats";
    return unreadCounts;
}


bool DatabaseService::clearAllData() {
    // Проверка соединения
    if (!isConnected()) return false;
    QSqlQuery query(m_db);
    QStringList tables = {"messages", "chats", "contacts"};
    // Проходим по всем ключевым таблицам и полностью их очищаем
    for (const QString &table : tables) {
        if (!query.exec("DELETE FROM " + table)) {
            // Логгируем причину сбоя для каждой конкретной таблицы
            qDebug() << "[DatabaseService] ERROR: Failed to clear table" << table << ":" << query.lastError().text();
            return false;
        }
    }
    // Все успешно — пишем в лог
    qDebug() << "[DatabaseService] All data cleared";
    return true;
}


void DatabaseService::printDatabaseStats() {
    if (!isConnected()) return;
    QSqlQuery query(m_db);

    // Получаем число сообщений
    query.exec("SELECT COUNT(*) FROM messages");
    query.next();
    int messageCount = query.value(0).toInt();

    // Получаем количество контактов
    query.exec("SELECT COUNT(*) FROM contacts");
    query.next();
    int contactCount = query.value(0).toInt();

    // Получаем количество чатов
    query.exec("SELECT COUNT(*) FROM chats");
    query.next();
    int chatCount = query.value(0).toInt();

    // Вывод статистики в отладочную консоль (можно использовать в тестах, профилировщике или UI)
    qDebug() << "[DatabaseService] === DATABASE STATS ===";
    qDebug() << "[DatabaseService] Messages:" << messageCount;
    qDebug() << "[DatabaseService] Contacts:" << contactCount;
    qDebug() << "[DatabaseService] Chats:" << chatCount;
    qDebug() << "[DatabaseService] === END DATABASE STATS ===";
}


bool DatabaseService::confirmSentMessageByTempId(const QString& tempId, const ChatMessage& confirmedMsg) {
    // Проверяем подключение к БД
    if (!isConnected()) return false;
    // Готовим запрос на UPDATE: записываем новый server_id, timestamp, status, убираем temp_id
    QSqlQuery query(m_db);
    query.prepare("UPDATE messages SET server_id = ?, timestamp = ?, status = ?, temp_id = NULL WHERE temp_id = ?");
    query.addBindValue(confirmedMsg.id);
    query.addBindValue(confirmedMsg.timestamp);
    query.addBindValue((int)confirmedMsg.status);
    query.addBindValue(tempId);
    // Исполняем запрос, логгируем ошибки при неудаче
    if (!query.exec()) {
        qDebug() << "[DatabaseService] ERROR: Failed to confirm message by tempId:" << query.lastError().text();
        return false;
    }
    // Лог успешного подтверждения для аудита client/server синхронизации
    qDebug() << "[DatabaseService] Confirmed message by tempId:" << tempId << "-> server_id:" << confirmedMsg.id;
    return true;
}


QString DatabaseService::executeSqlError(const QSqlQuery &query) {
    // Без обработки просто возвращаем lastError().text()
    return query.lastError().text();
}

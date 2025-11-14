#ifndef SERVER_H
#define SERVER_H

// Включения базовых классов и утилит Qt
#include <QObject>
#include <QHostAddress>

// Включения для сетевого взаимодействия
#include <QTcpServer>
#include <QWebSocketServer>

// Включения для структур данных
#include <QMap>
#include <QJsonObject>
#include "structures.h" // Кастомные структуры данных (User, ChatMessage).

// Прямые объявления (Forward Declarations) для использования указателей
// без включения полных заголовочных файлов.
class QTcpSocket;
class QWebSocket;

/**
 * @class Server
 * @brief Главный класс, реализующий логику чат-сервера.
 *
 * @details Этот класс управляет TCP и WebSocket серверами, обрабатывает подключения
 *          клиентов, аутентифицирует их, обрабатывает их запросы (отправка сообщений,
 *          поиск, управление контактами и т.д.), взаимодействует с базой данных SQLite
 *          для хранения информации и управляет состоянием онлайн-пользователей.
 */
class Server : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский объект.
     */
    explicit Server(QObject *parent = nullptr);

    /**
     * @brief Запускает сервер для прослушивания входящих подключений.
     * @param address IP-адрес для прослушивания (по умолчанию QHostAddress::Any - все интерфейсы).
     * @param tcpPort Порт для TCP-подключений.
     * @param wsPort Порт для WebSocket-подключений.
     * @return `true` если оба сервера успешно запущены, иначе `false`.
     */
    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 tcpPort = 1234, quint16 wsPort = 8080);

protected:
    // --- Методы-обработчики (handlers), вынесенные в protected для наглядности ---
    void initHandlers();
    void handleMessageDelivered(QObject* socket, const QJsonObject& request);
    void handleMessageRead(QObject* socket, const QJsonObject& request);
    void handleLogoutRequest(QObject* socket, const QJsonObject& request);

public slots:
    // --- Слоты для обработки событий TCP-сервера ---
    void onNewTcpConnection();
    void onTcpReadyRead();
    void onClientDisconnected(); // Общий слот для TCP и WebSocket

    // --- Слоты для обработки событий WebSocket-сервера ---
    void onNewWebSocketConnection();
    void onWebSocketTextMessageReceived(const QString &message);


private:
    /**
     * @brief Псевдоним для указателя на метод-обработчик команды.
     * @details Позволяет хранить указатели на методы в `QMap` для реализации паттерна "Команда".
     */
    using Handler = void (Server::*)(QObject*, const QJsonObject&);

    // --- Методы инициализации ---
    bool initDatabase();

    // --- Набор приватных методов-обработчиков для каждой команды от клиента ---
    void handleGetHistory(QObject* socket, const QJsonObject& request);
    void handleRegister(QObject* socket, const QJsonObject& request);
    void handleSearchUsers(QObject* socket, const QJsonObject& request);
    void handleAddContactRequest(QObject* socket, const QJsonObject& request);
    void handleLogin(QObject* socket, const QJsonObject& request);
    void handlePrivateMessage(QObject* fromUserSocket, const QJsonObject& request);
    void handleEditMessage(QObject* socket, const QJsonObject& request);
    void handleDeleteMessage(QObject* socket, const QJsonObject& request);
    void handleContactRequestResponse(QObject* socket, const QJsonObject& request);
    void handleTyping(QObject *socket, const QJsonObject &request);

    // --- Вспомогательные методы для отправки данных клиентам ---
    void sendJson(QObject* socket, const QJsonObject& response);
    void sendContactList(QObject* socket, const QString& username);
    void sendFullUserList(QObject* socket); // (Вероятно, устарел или для админ-панели)
    void broadcastUserList(); // Отправляет список онлайн-пользователей всем подключенным.
    void sendOfflineMessages(QObject* socket, const QString& username); // (Заготовка)
    void sendOnlineStatusList(QObject* clientSocket);
    void sendUnreadCounts(QObject* socket, const QString& username); // Отправляет счетчики непрочитанных.
    void sendPendingContactRequests(QObject* socket, const QString& username);
    void handleCallRequest(QObject* socket, const QJsonObject& request);
    void handleCallAccepted(QObject* socket, const QJsonObject& request);
    void handleCallRejected(QObject* socket, const QJsonObject& request);
    void handleCallEnd(QObject* socket, const QJsonObject& request);
    void handleGetCallHistory(QObject* socket, const QJsonObject& request);
    void handleGetCallStats(QObject* socket, const QJsonObject& request);
    void handleUpdateProfile(QObject* socket, const QJsonObject& request);

    void processJsonRequest(const QJsonObject& request, QObject* clientSocket);
    void removeClient(QObject* clientSocket); // (Заготовка/Не используется)
    void createCallRecord(const QString& callId, const QString& from,
                          const QString& to, const QString& fromIp, quint16 fromPort);

    void updateCallConnected(const QString& callId, const QString& toIp, quint16 toPort);

    void updateCallEnded(const QString& callId, const QString& status);
private:
    // --- Указатели на серверные объекты ---
    QTcpServer *m_tcpServer;
    QWebSocketServer *m_webSocketServer;

    // --- Структуры для управления состоянием онлайн-клиентов ---
    QMap<QString, QObject*> m_clients;       ///< Отображение `username` -> `указатель на сокет`. Для быстрого поиска сокета по имени.
    QMap<QObject*, QString> m_clientsReverse; ///< Отображение `указатель на сокет` -> `username`. Для быстрой идентификации клиента по сокету.
    // Карта активных звонков: callId -> {"from": "", "to": "", "fromSocket": ptr, "toSocket": ptr}
    QMap<QString, CallInfo> m_activeCalls;
    // --- Внутренние методы ---
    /**
     * @brief Общий метод-диспетчер, который вызывает нужный обработчик из `m_handlers`.
     * @param request JSON-запрос от клиента.
     * @param clientSocket Указатель на сокет, с которого пришел запрос.
     */


    // --- Специфично для TCP ---
    /**
     * @brief Хранит ожидаемый размер следующего TCP-пакета для каждого сокета.
     * @details Необходимо для корректной сборки пакетов переменной длины из TCP-потока.
     */
    QMap<QTcpSocket*, quint32> m_nextBlockSizes;

    // --- Карта обработчиков ---
    /**
     * @brief Карта, реализующая паттерн "Команда".
     * @details Ключ - строковое имя команды (из поля "type" в JSON).
     *          Значение - указатель на метод, который эту команду обрабатывает.
     */
    QMap<QString, Handler> m_handlers;
};

#endif // SERVER_H


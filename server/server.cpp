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

#include "structures.h"
#include "server.h"


Server::Server(QObject *parent) : QObject(parent){
    m_tcpServer = new QTcpServer(this);
    m_webSocketServer = new QWebSocketServer("MessengerServer", QWebSocketServer::NonSecureMode, this);

    connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewTcpConnection);
    connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &Server::onNewWebSocketConnection);

    if(!initDatabase()){
        qFatal("Fatal: Database initialization failed!");
    }
    initHandlers();
}
bool Server::listen(const QHostAddress &address, quint16 tcpPort, quint16 wsPort)
{
    bool tcpSuccess = m_tcpServer->listen(address, tcpPort);
    bool wsSuccess = m_webSocketServer->listen(address, wsPort);

    if (tcpSuccess && wsSuccess) {
        qDebug() << "TCP Server listening on port" << tcpPort;
        qDebug() << "WebSocket Server listening on port" << wsPort;
        return true;
    }
    if (!tcpSuccess) qDebug() << "TCP Server failed to start:" << m_tcpServer->errorString();
    if (!wsSuccess) qDebug() << "WebSocket Server failed to start:" << m_webSocketServer->errorString();
    return false;
}

void Server::onNewTcpConnection()
{
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    qDebug() << "New TCP client connected from:" << socket->peerAddress().toString();

    connect(socket, &QTcpSocket::readyRead, this, &Server::onTcpReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

    m_nextBlockSizes.insert(socket, 0);
}

void Server::onNewWebSocketConnection()
{
    QWebSocket *socket = m_webSocketServer->nextPendingConnection();
    qDebug() << "New WebSocket client connected from:" << socket->peerAddress().toString();

    connect(socket, &QWebSocket::textMessageReceived, this, &Server::onWebSocketTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &Server::onClientDisconnected);
}

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
}


void Server::handleMessageDelivered(QObject* socket, const QJsonObject& request){
    quint64 messageId = request["id"].toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE messages SET is_delivered = 1 WHERE id = :id");
    updateQuery.bindValue(":id", messageId);
    if (!updateQuery.exec()) {
        qDebug() << "[SERVER] Failed to mark INSTANT message as delivered:" << updateQuery.lastError().text();
    } else {
        qDebug() << "[SERVER] Marked message" << messageId << "as delivered to online user";
    }

    QSqlQuery query;
    query.prepare("SELECT fromUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);

    if (!query.exec()) {
        qDebug() << "DB Error: History request failed:" << query.lastError().text();
        return;
    }
    QString toUser = "";
    while(query.next()){
        toUser = query.record().value("fromUser").toString();
    }

    qDebug() << "[SERVER] message " << (double)messageId << "delivered, info from user, sending to" << toUser;
    QJsonObject deliveredCmd;
    deliveredCmd["type"] = "message_delivered";
    deliveredCmd["id"] = (double)messageId;

    sendJson(m_clients.value(toUser), deliveredCmd);
}

void Server::handleMessageRead(QObject* socket, const QJsonObject& request){
    quint64 messageId = request["id"].toInt();
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE messages SET is_read = 1 WHERE id = :id");
    updateQuery.bindValue(":id", messageId);
    if (!updateQuery.exec()) {
        qDebug() << "[SERVER] Failed to mark INSTANT message as read:" << updateQuery.lastError().text();
    } else {
        qDebug() << "[SERVER] Marked message" << messageId << "as read to online user" ;
    }

    QSqlQuery query;
    query.prepare("SELECT fromUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);

    if (!query.exec()) {
        qDebug() << "DB Error: History request failed:" << query.lastError().text();
        return;
    }
    QString toUser = "";
    while(query.next()){
        toUser = query.record().value("fromUser").toString();
    }
    qDebug() << "[SERVER] message " << (double)messageId << "read, info from user, sending tosending to" << toUser;

    QJsonObject readCmd;
    readCmd["type"] = "message_read";
    readCmd["id"] = (double)messageId;

    sendJson(m_clients.value(toUser), readCmd);
}

void Server::handleLogoutRequest(QObject* socket, const QJsonObject& request){
    QString fromUser = request["username"].toString();
    QString requestingUser = m_clientsReverse.value(socket);
    QJsonObject response;
    if(fromUser != requestingUser){
        qDebug() << "[SERVER]" << requestingUser << "trying to log out as"<<  fromUser;

        response["type"] = "logout_request_failure";
        response["reason"] = requestingUser + "trying to log out as" + fromUser;
        sendJson(socket, response);
        return;
    } else{
        response["type"] = "logout_request_success";
        sendJson(socket, response);
        QString username = m_clientsReverse.value(socket);
        m_clients.remove(username);

        m_clientsReverse.remove(socket);
        broadcastUserList();
    }
}

void Server::onTcpReadyRead(){
    auto socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);
    while(true){
        quint32 &nextBlockSize = m_nextBlockSizes[socket];
        if (nextBlockSize == 0){
            if(socket->bytesAvailable() < sizeof(quint32)){
                break;
            }
            in>>nextBlockSize;
        }
        if (socket->bytesAvailable() < nextBlockSize){
            break;
        }

        QByteArray messageData;
        in >> messageData;

        nextBlockSize = 0;
        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (!doc.isNull() && !doc.isObject()){
            processJsonRequest(doc.object(), socket);
        }

        QJsonObject request = doc.object();
        QString type = request["type"].toString();

        qDebug() << "[SERVER] Processing message of type:" << type;



        if (m_handlers.contains(type)) {
            Handler handler = m_handlers[type];
            (this->*handler)(socket, request);

        } else {
            qDebug() << "[SERVER] Unknown request type received:" << type;
            sendJson(socket, {{"type", "error"}, {"reason", "Unknown command: " + type}});
        }
    }
}

void Server::onWebSocketTextMessageReceived(const QString &message)
{
    auto socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        processJsonRequest(doc.object(), socket);
    }
}

void Server::processJsonRequest(const QJsonObject& request, QObject* socket)
{
    QString type = request["type"].toString();
    qDebug() << "[SERVER] Processing message of type:" << type << "from" << m_clientsReverse.value(socket);

    Handler handler = m_handlers[type];
    (this->*handler)(socket, request);
}

void Server::onClientDisconnected()
{
    auto socket = qobject_cast<QObject*>(sender());
    if (!socket) return;

    QString username = m_clientsReverse.value(socket);
    if (!username.isEmpty()) {
        qDebug() << "User" << username << "disconnected.";

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE users SET last_seen = :lastSeen WHERE username = :username");
        updateQuery.bindValue(":lastSeen", QDateTime::currentDateTime().toString(Qt::ISODate));
        updateQuery.bindValue(":username", username);
        if (!updateQuery.exec()) {
            qDebug() << "[SERVER][ERROR] Failed to update last_seen for user" << username << ":" << updateQuery.lastError().text();
        } else {
            qDebug() << "[SERVER] Updated last_seen for user" << username;
        }
        m_clients.remove(username);
        m_clientsReverse.remove(socket);

        broadcastUserList();
    }

    if (auto tcpSocket = qobject_cast<QTcpSocket*>(socket)) {
        m_nextBlockSizes.remove(tcpSocket);
    }

    sender()->deleteLater();
}

bool Server::initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.db");
    if (!db.open()) {
        qDebug() << "Error: connection with database failed:" << db.lastError().text();
        return false;
    }
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "display_name TEXT NOT NULL, "
                    "password_hash TEXT NOT NULL, "
                    "creation_date TEXT NOT NULL, "
                    "last_seen TEXT, "
                    "avatar_url TEXT, "
                    "status_message TEXT"
                    ");"))
    {
        qDebug() << "DB Error: failed to create 'users' table:" << query.lastError().text();
        return false;
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS messages ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "fromUser TEXT NOT NULL, "
                    "toUser TEXT NOT NULL, "
                    "payload TEXT NOT NULL, "
                    "timestamp TEXT NOT NULL, "
                    "is_delivered INTEGER NOT NULL DEFAULT 0, "
                    "is_read INTEGER NOT NULL DEFAULT 0, "
                    "is_edited INTEGER NOT NULL DEFAULT 0, "
                    "reply_to_id INTEGER, "
                    "forwarded_from TEXT, "
                    "message_type INTEGER NOT NULL DEFAULT 0, "
                    "media_url TEXT"
                    ");")) {
        qDebug() << "[SERVER] DB Error: failed to create 'messages' table:" << query.lastError().text();
        return false;
    }
    if (!query.exec("CREATE TABLE IF NOT EXISTS contacts ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "user_id_1 INTEGER NOT NULL, "
                    "user_id_2 INTEGER NOT NULL, "
                    "status INTEGER NOT NULL DEFAULT 0, "  
                    "creation_date TEXT NOT NULL, "
                    "FOREIGN KEY(user_id_1) REFERENCES users(id), "
                    "FOREIGN KEY(user_id_2) REFERENCES users(id), "
                    "UNIQUE(user_id_1, user_id_2), "
                    "CHECK(user_id_1 < user_id_2)"
                    ");"))
    {
        qDebug() << "DB Error: failed to create 'contacts' table:" << query.lastError().text();
        return false;
    }
    return true;
}

void Server::handleTyping(QObject *socket, const QJsonObject &request){
    QString fromUsername = m_clientsReverse.value(socket);

    QString toUsername = request["toUser"].toString();
    QObject* toSocket = m_clients.value(toUsername, nullptr);
    if (toSocket) {
        QJsonObject forwardMessage;
        forwardMessage["type"] = "typing";
        forwardMessage["fromUser"] = fromUsername;

        sendJson(toSocket, forwardMessage);
    }
}
void Server::handleGetHistory(QObject* socket, const QJsonObject& request){
    QString requestingUser = m_clientsReverse.value(socket);
    QString chatPartner = request["with_user"].toString();
    qint64 beforeId = request["before_id"].toDouble();

    qDebug() << "[SERVER] History request from" << requestingUser
             << "for chat with" << chatPartner
             << "before message ID:" << beforeId;


    QSqlQuery query;

    QString queryString =
        "SELECT id, fromUser, toUser, payload, timestamp, reply_to_id, is_read, is_edited, is_delivered FROM messages "
        "WHERE ((fromUser = :user1 AND toUser = :user2) OR (fromUser = :user2 AND toUser = :user1)) ";

    if (beforeId > 0) {
        queryString += "AND id < :beforeId ";
    }

    queryString += "ORDER BY id DESC LIMIT 20";

    query.prepare(queryString);
    query.bindValue(":user1", requestingUser);
    query.bindValue(":user2", chatPartner);

    if (beforeId > 0) {
        query.bindValue(":beforeId", beforeId);
    }

    if (!query.exec()) {
        qDebug() << "DB Error: History request failed:" << query.lastError().text();
        return;
    }
    QJsonArray historyArray;
    while (query.next()) {
        QSqlRecord record = query.record();
        QJsonObject messageObject;
        messageObject["type"] = "private_message";
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
    QJsonArray reversedArray;
    for (int i = historyArray.size() - 1; i >= 0; --i) {
        reversedArray.append(historyArray.at(i));
    }


    QJsonObject response;
    response["type"] = "old_history_data";
    if (beforeId == 0) {
        response["type"] = "history_data";
    }

    response["with_user"] = chatPartner;
    response["history"] = reversedArray;
    sendJson(socket, response);

}

void Server::handleRegister(QObject* socket, const QJsonObject& request){
    QString username = request["username"].toString();
    QString display_name = request["display_name"].toString();
    QString password = request["password"].toString();
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, display_name, password_hash, creation_date) "
                  "VALUES (:username, :display_name, :password_hash, :creation_date)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", QString(passwordHash));
    query.bindValue(":display_name", display_name);
    query.bindValue(":creation_date", QDateTime::currentDateTime().toString(Qt::ISODate));

    QJsonObject response;
    if (query.exec()) {
        response["type"] = "register_success";
        for (QObject *socket : m_clients.values()) {
            broadcastUserList();
        }
    } else {
        response["type"] = "register_failure";
        response["reason"] = "Username already exists.";
    }
    sendJson(socket, response);
}

void Server::handleSearchUsers(QObject* socket, const QJsonObject& request)
{
    QString searchTerm = request["term"].toString();
    QString currentUser = m_clientsReverse.value(socket);

    QSqlQuery query;
    query.prepare("SELECT username, display_name FROM users WHERE (username LIKE :term OR display_name LIKE :term) AND username != :currentUser LIMIT 20");
    query.bindValue(":term", "%" + searchTerm + "%");
    query.bindValue(":currentUser", currentUser);

    if (!query.exec()) {
        return;
    }

    QJsonArray usersFound;
    while (query.next()) {
        QJsonObject userObject;
        userObject["username"] = query.value(0).toString();
        userObject["displayname"] = query.value(1).toString();
        usersFound.append(userObject);
    }

    QJsonObject response;
    response["type"] = "search_results";
    response["users"] = usersFound;
    sendJson(socket, response);
}

void Server::sendContactList(QObject* socket,const QString& username){
    QSqlQuery userQuery;
    userQuery.prepare("SELECT id FROM users WHERE username = :username");
    userQuery.bindValue(":username", username);

    if (!userQuery.exec() || !userQuery.next()) return;
    qint64 userId = userQuery.value("id").toLongLong();

    QSqlQuery query;
    query.prepare(
        "SELECT u.username, u.display_name, u.last_seen FROM users u "
        "JOIN contacts c ON (u.id = c.user_id_1 OR u.id = c.user_id_2) "
        "WHERE (c.user_id_1 = :userId OR c.user_id_2 = :userId) "
        "AND c.status = 1 AND u.id != :userId"
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "[SERVER] Failed to get contact list:" << query.lastError().text();
        return;
    }

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
}

void Server::handleAddContactRequest(QObject* socket, const QJsonObject& request)
{
    QString fromUsername = m_clientsReverse.value(socket);
    QString toUsername = request["username"].toString();


    if (toUsername.isEmpty()) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "Invalid username provided."}});
        return;
    }

    if (fromUsername == toUsername) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "You cannot add yourself as a contact."}});
        return;
    }

    QSqlQuery idQuery;
    idQuery.prepare("SELECT id, username, display_name FROM users WHERE username = :from OR username = :to");
    idQuery.bindValue(":from", fromUsername);
    idQuery.bindValue(":to", toUsername);

    if (!idQuery.exec()) {
        qDebug() << "[SERVER] DB Error: Failed to find user IDs:" << idQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "A database error occurred."}});
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

    if (fromId == -1 || toId == -1) {
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "The requested user does not exist."}});
        return;
    }

    qint64 userId1 = std::min(fromId, toId);
    qint64 userId2 = std::max(fromId, toId);

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT status FROM contacts WHERE user_id_1 = :id1 AND user_id_2 = :id2");
    checkQuery.bindValue(":id1", userId1);
    checkQuery.bindValue(":id2", userId2);
    if (!checkQuery.exec()) {
        qDebug() << "[SERVER] DB Error: Failed to check for existing contact:" << checkQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "A database error occurred."}});
        return;
    }

    if (checkQuery.next()) {
        int status = checkQuery.value(0).toInt();
        QString reason = "A relationship with this user already exists.";
        if (status == 0) reason = "A contact request is already pending with this user.";
        if (status == 1) reason = "This user is already in your contacts.";
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", reason}});
        return;
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO contacts (user_id_1, user_id_2, status, creation_date) "
                        "VALUES (:id1, :id2, 0, :date)");
    insertQuery.bindValue(":id1", userId1);
    insertQuery.bindValue(":id2", userId2);
    insertQuery.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!insertQuery.exec()) {
        qDebug() << "[SERVER] DB Error: Failed to insert contact request:" << insertQuery.lastError().text();
        sendJson(socket, {{"type", "add_contact_failure"}, {"reason", "A database error occurred while sending the request."}});
        return;
    }


    QObject* toSocket = m_clients.value(toUsername, nullptr);;
    if (toSocket) {
        QJsonObject notification;
        notification["type"] = "incoming_contact_request";
        notification["fromUsername"] = fromUsername;
        notification["fromDisplayname"] = fromDisplayName;
        sendJson(toSocket, notification);
        qDebug() << "[SERVER] Sent incoming contact request notification to" << toUsername;
    }

    sendJson(socket, {{"type", "add_contact_success"}, {"reason", "Contact request sent successfully to " + toUsername + "."}});
    qDebug() << "[SERVER] User" << fromUsername << "sent a contact request to" << toUsername;
}

void Server::handleLogin(QObject* socket, const QJsonObject& request)
{

    QString username = request["username"].toString();
    QString password = request["password"].toString();
    qDebug() << request;
    qDebug() <<username << " " << password;
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;

    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    QJsonObject response;
    if (query.exec() && query.next()){
        QString storedHash = query.value(0).toString();
        if (storedHash == passwordHash){
            qDebug() << "LOgged in success";
            response["type"] = "login_success";

            m_clients[username] = socket;
            m_clientsReverse[socket] = username;

            sendJson(socket, response);
            sendContactList(socket, username);
            broadcastUserList();
            sendPendingContactRequests(socket, username);
            sendUnreadCounts(socket, username);
        }
        else{
            response["type"] = "login_failure";
            response["reason"] = "Invalid password.";
            sendJson(socket, response);
        }
    } else{
        response["type"] = "login_failure";
        response["reason"] = "User not found.";
        sendJson(socket, response);
    }
}

void Server::handlePrivateMessage(QObject* socket, const QJsonObject& request)
{
    QString fromUser = request["fromUser"].toString();
    QString toUser = request["toUser"].toString();
    QString payload = request["payload"].toString();
    qint64 replyToId = request["reply_to_id"].toVariant().toLongLong();
    QString tempId = request["temp_id"].toString();
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);


    if(fromUser != m_clientsReverse.value(socket)) {
        qWarning() << "[SERVER] SECURITY WARNING: User" << m_clientsReverse.value(socket)
                   << "tried to send a message as" << fromUser;
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO messages (fromUser, toUser, payload, timestamp, reply_to_id) "
                  "VALUES (:fromUser, :toUser, :payload, :timestamp, :reply_to_id)");
    query.bindValue(":fromUser", fromUser);
    query.bindValue(":toUser", toUser);
    query.bindValue(":payload", payload);
    query.bindValue(":timestamp", timestamp);
    query.bindValue(":reply_to_id", replyToId > 0 ? QVariant(replyToId) : QVariant());


    if(!query.exec()){
        qDebug() << "[SERVER] Failed to save message to DB:" << query.lastError().text();
    }

    quint64 messageId = query.lastInsertId().toULongLong();
    QJsonObject echoMessage;
    echoMessage["type"] = "private_message";
    echoMessage["id"] = (double)messageId;
    echoMessage["fromUser"] = fromUser;
    echoMessage["toUser"] = toUser;

    echoMessage["payload"] = payload;
    echoMessage["timestamp"] = timestamp;
    echoMessage["is_delivered"] = 1;
    echoMessage["is_read"] = 0;
    echoMessage["is_edited"] = 0;


    if (replyToId > 0) echoMessage["reply_to_id"] = replyToId;
    echoMessage["temp_id"] = tempId;
    sendJson(socket, echoMessage);

    echoMessage["temp_id"] = "";
    QObject *toUserSocket = m_clients.value(toUser, nullptr);


    if(toUserSocket){
        QJsonObject forwardMessage = echoMessage;

        sendJson(toUserSocket, forwardMessage);

        qDebug() << "[SERVER] Private message forwarded fromUser" << fromUser << "to" << toUser;
    }
}

void Server::sendJson(QObject* socket, const QJsonObject& json)
{
    if (!socket) return;

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    if (auto sendsocket = qobject_cast<QTcpSocket*>(socket)) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_2);
        out << (quint32)0;
        out << jsonData;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));
        sendsocket->write(block);

    } else if (auto sendsocket = qobject_cast<QWebSocket*>(socket)) {
        sendsocket->sendTextMessage(QString::fromUtf8(jsonData));
    }
}

void Server::sendFullUserList(QObject* socket)
{
    QSqlQuery query;
    query.prepare("SELECT username, display_name FROM users");
    if (!query.exec()) {
        qDebug() << "Failed to get full user list:" << query.lastError().text();
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
    qDebug() << "Sent full user list to a client:" << allUsers;
}

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

void Server::sendOfflineMessages(QObject* socket, const QString& username){  
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id, fromUser, payload, timestamp, reply_to_id, is_edited "
                        "FROM messages "
                        "WHERE to_user = :toUser AND is_delivered = 0 "
                        "ORDER BY id ASC");
    selectQuery.bindValue(":toUser", username);
}

void Server::handleEditMessage(QObject* clientSocket, const QJsonObject& request)
{
    QString requestingUser = m_clientsReverse.value(clientSocket);
    quint64 messageId = request["id"].toDouble();

    qDebug() << requestingUser << "wants to edit message with id:" << messageId;
    if (messageId == 0) return;

    if (requestingUser.isEmpty()) {
        qWarning() << "SECURITY: Edit request from an unauthenticated socket!";
        return;
    }


    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);
    if (query.exec() && query.next()) {
        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();

        qDebug() << "Requesting user:" << requestingUser << " fromUser in DB:" << fromUser << " toUser in DB:" << toUser;

        if (fromUser == requestingUser) {
            QSqlQuery updateQuery;
            QString newPayload = request["payload"].toString();
            updateQuery.prepare("UPDATE messages SET payload = :payload, is_edited = 1 WHERE id = :id");
            updateQuery.bindValue(":payload", newPayload);
            updateQuery.bindValue(":id", messageId);

            if (updateQuery.exec()) {
                qDebug() << "[SERVER] User" << requestingUser << "edited message" << messageId;

                QJsonObject editCmd;
                editCmd["type"] = "edit_message";
                editCmd["id"] = (double)messageId;
                editCmd["payload"] = newPayload;

                QObject* fromSocket = m_clients.value(fromUser, nullptr);
                if (fromSocket) {
                    editCmd["with_user"] = toUser;
                    sendJson(fromSocket, editCmd);
                }

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
void Server::handleDeleteMessage(QObject* clientSocket, const QJsonObject& request)
{
    QString requestingUser = m_clientsReverse.value(clientSocket);
    quint64 messageId = request["id"].toDouble();

    qDebug() << requestingUser << "wants to delete message with id:" << messageId;
    if (messageId == 0) return;

    if (requestingUser.isEmpty()) {
        qWarning() << "SECURITY: Delete request from an unauthenticated socket!";
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);

    if (query.exec() && query.next()) {
        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();

        if (requestingUser == fromUser) {
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM messages WHERE id = :id");
            deleteQuery.bindValue(":id", messageId);

            if (deleteQuery.exec()) {
                qDebug() << "[SERVER] User" << requestingUser << "deleted message" << messageId;

                QJsonObject deleteCmd;
                deleteCmd["type"] = "delete_message";
                deleteCmd["id"] = (double)messageId;

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
            qWarning() << "SECURITY: User" << requestingUser << "tried to delete a message they do not own (author:" << fromUser << ")";
        }
    }
}

void Server::handleContactRequestResponse(QObject* clientSocket, const QJsonObject& request)
{
    qDebug() << "[SERVER] Received contact_request_response:" << request;


    QString toUsername = m_clientsReverse.value(clientSocket);

    QString fromUsername = request["fromUsername"].toString();
    QString response = request["response"].toString();

    qDebug() << "[SERVER] Parsed response value:" << response;

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

    if (fromId == -1 || toId == -1) { return; }

    qint64 userId1 = std::min(fromId, toId);
    qint64 userId2 = std::max(fromId, toId);

    if (response == "accepted") {
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE contacts SET status = 1 WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        updateQuery.bindValue(":id1", userId1);
        updateQuery.bindValue(":id2", userId2);

        if (updateQuery.exec() && updateQuery.numRowsAffected() > 0) {
            qDebug() << "[SERVER]" << toUsername << "accepted contact request from" << fromUsername;

            QObject* fromSocket = m_clients.value(fromUsername, nullptr);
            QObject* toSocket = m_clients.value(toUsername, nullptr);

            if (fromSocket) {
                sendContactList(fromSocket, fromUsername);

            }
            if (toSocket) {
                sendContactList(toSocket, toUsername);
            }

            if (fromSocket) sendOnlineStatusList(fromSocket);
            if (toSocket) sendOnlineStatusList(toSocket);
        }
    } else if (response == "declined") {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM contacts WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        deleteQuery.bindValue(":id1", userId1);
        deleteQuery.bindValue(":id2", userId2);

        if (deleteQuery.exec()) {
            qDebug() << "[SERVER]" << toUsername << "declined contact request from" << fromUsername;
        }
    }
}
void Server::sendUnreadCounts(QObject* socket, const QString& username)
{
    qDebug() << "[SERVER][UNREAD] Собираем счетчики непрочитанных для пользователя:" << username;

     
    QSqlQuery idQuery;
    idQuery.prepare("SELECT id FROM users WHERE username = :username");
    idQuery.bindValue(":username", username);
    if (!idQuery.exec() || !idQuery.next()) {
        qDebug() << "[SERVER][UNREAD][ERROR] Не удалось найти ID для пользователя:" << username;
        return;
    }
    qint64 userId = idQuery.value(0).toLongLong();

     
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

    QJsonArray countsArray;
    while (query.next()) {
        QJsonObject countObject;
        countObject["username"] = query.value("fromUser").toString();
        countObject["count"] = query.value("unread_count").toInt();
        countsArray.append(countObject);
    }

    if (countsArray.isEmpty()) {
        qDebug() << "[SERVER][UNREAD] Непрочитанных сообщений для" << username << "не найдено.";
        return;
    }

     
    QJsonObject response;
    response["type"] = "unread_counts";
    response["counts"] = countsArray;

    qDebug() << "[SERVER][UNREAD] Отправка счетчиков для" << username << ":" << response;
    sendJson(socket, response);
}
void Server::sendOnlineStatusList(QObject* clientSocket)
{
    QStringList onlineUsers = m_clients.keys();

    QJsonObject message;
    message["type"] = "user_list";
    message["users"] = QJsonArray::fromStringList(onlineUsers);

    sendJson(clientSocket, message);
    qDebug() << "[SERVER] Sent online status list to a single client.";
}

void Server::sendPendingContactRequests(QObject* socket, const QString& username){
    qDebug() << "[SERVER][PENDING] Checking for pending requests for user:" << username;
    QSqlQuery userQuery;
    userQuery.prepare("SELECT id FROM users WHERE username = :username");
    userQuery.bindValue(":username", username);
    if (!userQuery.exec() || !userQuery.next()) {
        qDebug() << "[SERVER][PENDING][ERROR] Could not find ID for user:" << username;
        return;
    }
    qint64 userId = userQuery.value(0).toLongLong();
    qDebug() << "[SERVER][PENDING] User ID is:" << userId;
    QSqlQuery query;
    query.prepare(
        "SELECT u.username, u.display_name FROM users u "
        "JOIN contacts c ON u.id = (CASE WHEN c.user_id_1 = :userId THEN c.user_id_2 ELSE c.user_id_1 END) "
        "WHERE (c.user_id_1 = :userId OR c.user_id_2 = :userId) AND c.status = 0"
        );
    query.bindValue(":userId", QVariant(userId));

    if (!query.exec()) {
        qDebug() << "[SERVER][PENDING][ERROR] DB Error: Failed to fetch pending requests:" << query.lastError().text();
        return;
    }
    qDebug() << "[SERVER][PENDING] Main SQL query executed successfully. Processing results...";

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

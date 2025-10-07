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


Server::Server(QObject *parent) : QTcpServer(parent){
    if(!initDatabase()){
        qFatal("Fatal: Database initialization failed!");
    }
    initHandlers();
};

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

void Server::handleTyping(QTcpSocket* socket, const QJsonObject& request){
    return;
}

void Server::handleMessageDelivered(QTcpSocket* socket, const QJsonObject& request){
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

    sendJson(loggedInUsers.value(toUser), deliveredCmd);
}

void Server::handleMessageRead(QTcpSocket* socket, const QJsonObject& request){
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

    sendJson(loggedInUsers.value(toUser), readCmd);
}

void Server::handleLogoutRequest(QTcpSocket* socket, const QJsonObject& request){
    QString fromUser = request["username"].toString();
    QString requestingUser = loggedInUsers.key(socket);
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
        QString username = loggedInUsers.key(socket);
        loggedInUsers.remove(username);
        broadcastUserList();
    }
}
void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);

    qDebug() << "New client connected!";

    m_nextBlockSizes.insert(clientSocket, 0);


    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
}

void Server::onReadyRead(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_6_2);
    while(true){
        quint32 &nextBlockSize = m_nextBlockSizes[clientSocket];
        if (nextBlockSize == 0){
            if(clientSocket->bytesAvailable() < sizeof(quint32)){
                break;
            }
            in>>nextBlockSize;
        }
        if (clientSocket->bytesAvailable() < nextBlockSize){
            break;
        }

        QByteArray messageData;
        in >> messageData;

        nextBlockSize = 0;
        QJsonDocument doc = QJsonDocument::fromJson(messageData);
        if (doc.isNull() || !doc.isObject()){
            qDebug() << "[SERVER] FAILED TO PARSE JSON";
            continue;
        }

        QJsonObject request = doc.object();
        QString type = request["type"].toString();

        qDebug() << "[SERVER] Processing message of type:" << type;



        if (m_handlers.contains(type)) {
            Handler handler = m_handlers[type];
            (this->*handler)(clientSocket, request);

        } else {
            qDebug() << "[SERVER] Unknown request type received:" << type;
            sendJson(clientSocket, {{"type", "error"}, {"reason", "Unknown command: " + type}});
        }
    }
}

void Server::onDisconnected(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;


    m_nextBlockSizes.remove(clientSocket);

    QString username = loggedInUsers.key(clientSocket);
    if (!username.isEmpty()) {
        loggedInUsers.remove(username);
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

        broadcastUserList();
    }
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
                    "status INTEGER NOT NULL DEFAULT 0, " // 0: Pending, 1: Accepted, 2: Blocked
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

void Server::handleGetHistory(QTcpSocket* socket, const QJsonObject& request){
    QString requestingUser = loggedInUsers.key(socket);
    QString chatPartner = request["with_user"].toString();
    qDebug() << "[SERVER] History request from" << requestingUser << "for chat with" << chatPartner;

    QSqlQuery query;
    query.prepare("SELECT id, fromUser, payload, timestamp, reply_to_id, is_read, is_edited, is_delivered FROM messages WHERE "
                  "((fromUser = :user1 AND toUser = :user2) OR "
                  "(fromUser = :user2 AND toUser = :user1)) "
                  "ORDER BY id DESC LIMIT 50");
    query.bindValue(":user1", requestingUser);
    query.bindValue(":user2", chatPartner);

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
    response["type"] = "history_data";
    response["with_user"] = chatPartner;
    response["history"] = reversedArray;
    sendJson(socket, response);

}

void Server::handleRegister(QTcpSocket* socket, const QJsonObject& request){
    QString username = request["username"].toString();
    QString display_name = request["displayname"].toString();
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
        for (QTcpSocket *socket : loggedInUsers.values()) {
            broadcastUserList();
        }
    } else {
        response["type"] = "register_failure";
        response["reason"] = "Username already exists.";
    }
    sendJson(socket, response);
}

void Server::handleSearchUsers(QTcpSocket* socket, const QJsonObject& request)
{
    QString searchTerm = request["term"].toString();
    QString currentUser = loggedInUsers.key(socket);


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

void Server::sendContactList(QTcpSocket* socket,const QString& username){
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

void Server::handleAddContactRequest(QTcpSocket* socket, const QJsonObject& request)
{
    QString fromUsername = loggedInUsers.key(socket);
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
            fromDisplayName = idQuery.value("displayname").toString();
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


    QTcpSocket* toSocket = loggedInUsers.value(toUsername, nullptr);
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

void Server::handleLogin(QTcpSocket* socket, const QJsonObject& request)
{

    QString username = request["username"].toString();
    QString password = request["password"].toString();
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;

    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    QJsonObject response;
    if (query.exec() && query.next()){
        QString storedHash = query.value(0).toString();
        if (storedHash == passwordHash){
            response["type"] = "login_success";
            loggedInUsers[username]=socket;
            sendJson(socket, response);
            sendContactList(socket, username);
            broadcastUserList();
            sendPendingContactRequests(socket, username);
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

void Server::handlePrivateMessage(QTcpSocket* fromUserSocket, const QJsonObject& request)
{
    QString fromUser = request["fromUser"].toString();
    QString toUser = request["toUser"].toString();
    QString payload = request["payload"].toString();
    qint64 replyToId = request["reply_to_id"].toVariant().toLongLong();
    QString tempId = request["temp_id"].toString();
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);


    if(fromUser != loggedInUsers.key(fromUserSocket)){
        qWarning() << "[SERVER] SECURITY WARNING: User" << loggedInUsers.key(fromUserSocket)
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
    echoMessage["temp_id"] = tempId;
    echoMessage["payload"] = payload;
    echoMessage["timestamp"] = timestamp;
    echoMessage["is_delivered"] = 1;
    echoMessage["is_read"] = 0;
    echoMessage["is_edited"] = 0;


    if (replyToId > 0) echoMessage["reply_to_id"] = replyToId;
    sendJson(fromUserSocket, echoMessage);


    QTcpSocket *toUserSocket = loggedInUsers.value(toUser, nullptr);


    if(toUserSocket){
        QJsonObject forwardMessage = echoMessage;

        sendJson(toUserSocket, forwardMessage);

        qDebug() << "[SERVER] Private message forwarded fromUser" << fromUser << "to" << toUser;
    }
}

void Server::sendJson(QTcpSocket* socket, const QJsonObject& response)
{
    if(socket == nullptr){
        return;
    }
    QByteArray jsonData = QJsonDocument(response).toJson();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << (quint32)0;
    out << jsonData;
    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    socket->write(block);
}

void Server::sendFullUserList(QTcpSocket* socket)
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
    QStringList onlineUsers = loggedInUsers.keys();
    qDebug() << "Broadcasting ONLINE user list:" << onlineUsers;

    QJsonObject message;
    message["type"] = "user_list";
    message["users"] = QJsonArray::fromStringList(onlineUsers);


    for (QTcpSocket *socket : loggedInUsers.values()) {
        sendJson(socket, message);
    }
}

void Server::sendOfflineMessages(QTcpSocket* socket, const QString& username){ // доделать
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id, fromUser, payload, timestamp, reply_to_id, is_edited "
                        "FROM messages "
                        "WHERE to_user = :toUser AND is_delivered = 0 "
                        "ORDER BY id ASC");
    selectQuery.bindValue(":toUser", username);
}

void Server::handleEditMessage(QTcpSocket* socket, const QJsonObject& request){
    QString requestingUser = loggedInUsers.key(socket);
    quint64 messageId = request["id"].toDouble();
    qDebug() <<requestingUser <<" wanna edit message with id: " << messageId;
    if (messageId == 0) return;
    if(requestingUser != loggedInUsers.key(socket)){
        qWarning() << "SECURITY: Edit request from wrong user!";
        return;
    }
    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages where id = :id");
    query.bindValue(":id", messageId);
    if(query.exec() && query.next()){
        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();
        qDebug() <<"Requesting user: "<< requestingUser <<" fromUser: "<< fromUser << " toUser: " << toUser;

        if(fromUser == requestingUser){
            QSqlQuery updateQuery;
            QString newPayload = request["payload"].toString();
            updateQuery.prepare("UPDATE messages SET payload = :payload WHERE id = :id");
            updateQuery.bindValue(":payload", newPayload);
            updateQuery.bindValue(":id", messageId);

            if(updateQuery.exec()){
                qDebug() << "[SERVER] User" << requestingUser << "edited message" << messageId;

                QJsonObject editCmd;
                editCmd["type"] = "edit_message";
                editCmd["id"] = (double)messageId;
                editCmd["payload"] = newPayload;

                QTcpSocket* fromSocket = loggedInUsers.value(requestingUser, nullptr);
                if (fromSocket) {
                    editCmd["with_user"] = toUser;
                    sendJson(fromSocket, editCmd);
                }


                QTcpSocket* toSocket = loggedInUsers.value(toUser, nullptr);
                if (toSocket) {
                    editCmd["with_user"] = fromUser;
                    sendJson(toSocket, editCmd);
                }
            }


            updateQuery.prepare("UPDATE messages SET is_edited = :is_edited WHERE id = :id");
            updateQuery.bindValue(":is_edited", 1);
            updateQuery.bindValue(":id", messageId);

            if(updateQuery.exec()){
                qDebug() << "[SERVER] Database record parameter is edited changed";
            }
        }
    }
}

void Server::handleDeleteMessage(QTcpSocket* socket, const QJsonObject& request){
    QString requestingUser = loggedInUsers.key(socket);
    quint64 messageId = request["id"].toDouble();
    qDebug() <<requestingUser <<" wanna delete message with id: " << messageId;
    if (messageId == 0) return;
    if(requestingUser != loggedInUsers.key(socket)){
        qWarning() << "SECURITY: Edit request from wrong user!";
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT fromUser, toUser FROM messages WHERE id = :id");
    query.bindValue(":id", messageId);

    if(query.exec()&& query.next() ){
        qDebug() << "User" << requestingUser << "deleted message" << messageId;

        QString fromUser = query.value("fromUser").toString();
        QString toUser = query.value("toUser").toString();

        if(requestingUser == fromUser){
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM messages WHERE id = :id");
            deleteQuery.bindValue(":id", messageId);

            if(deleteQuery.exec()){
                QJsonObject deleteCmd;
                deleteCmd["type"] = "delete_message";
                deleteCmd["id"] = (double)messageId;;
                QTcpSocket* fromSocket = loggedInUsers.value(requestingUser, nullptr);
                if (fromSocket) {
                    deleteCmd["with_user"] = toUser;
                    sendJson(fromSocket, deleteCmd);
                }
                QTcpSocket* toSocket = loggedInUsers.value(toUser, nullptr);
                if (toSocket) {
                    deleteCmd["with_user"] = fromUser;
                    sendJson(toSocket, deleteCmd);
                }
            }

        }
    }
}

void Server::handleContactRequestResponse(QTcpSocket* socket, const QJsonObject& request){

    qDebug() << "[SERVER] Received contact_request_response:" << request;

    QString toUsername = loggedInUsers.key(socket);

    QString fromUsername = request["fromUsername"].toString();
    QString response = request["response"].toString();


    QSqlQuery idQuery;
    idQuery.prepare("SELECT id, username FROM users WHERE username = :from OR username = :to");
    idQuery.bindValue(":from", fromUsername);
    idQuery.bindValue(":to", toUsername);

    if (!idQuery.exec()) {
        return;
    }

    qint64 fromId = -1, toId = -1;
    while (idQuery.next()) {
        if (idQuery.value("username").toString() == fromUsername) {
            fromId = idQuery.value("id").toLongLong();
        } else {
            toId = idQuery.value("id").toLongLong();
        }
    }

    if (fromId == -1 || toId == -1) {
        return;
    }

    qint64 userId1 = std::min(fromId, toId);
    qint64 userId2 = std::max(fromId, toId);
    qDebug() << response;
    if (response == "accepted") {
        QSqlQuery debugSelect;
        debugSelect.prepare("SELECT status FROM contacts WHERE user_id_1 = :id1 AND user_id_2 = :id2");
        debugSelect.bindValue(":id1", userId1);
        debugSelect.bindValue(":id2", userId2);
        if (debugSelect.exec() && debugSelect.next()) {
            qDebug() << "[SERVER][DEBUG] Current status before update is:" << debugSelect.value(0).toInt();
        } else {
            qDebug() << "[SERVER][DEBUG] No contact record found before update.";
        }

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE contacts SET status = 1 "
                            "WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        updateQuery.bindValue(":id1", userId1);
        updateQuery.bindValue(":id2", userId2);

        if (updateQuery.exec() && updateQuery.numRowsAffected() > 0) {
            qDebug() << "[SERVER]" << toUsername << "accepted contact request from" << fromUsername;
            QTcpSocket* fromSocket = loggedInUsers.value(fromUsername, nullptr);
            QTcpSocket* toSocket = loggedInUsers.value(toUsername, nullptr);

            if (fromSocket) {

                sendContactList(fromSocket, fromUsername);
            }
            if (toSocket) {
                sendContactList(toSocket, toUsername);
            }
            broadcastUserList();
        }
    } else if (response == "declined") {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM contacts "
                            "WHERE user_id_1 = :id1 AND user_id_2 = :id2 AND status = 0");
        deleteQuery.bindValue(":id1", userId1);
        deleteQuery.bindValue(":id2", userId2);

        if (deleteQuery.exec()) {
            qDebug() << "[SERVER]" << toUsername << "declined contact request from" << fromUsername;
        }
    }
}

void Server::sendPendingContactRequests(QTcpSocket* socket, const QString& username){
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

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
#include "structures.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject *parent = nullptr) : QTcpServer(parent){
        if(!initDatabase()){
            qFatal("Fatal: Database initialization failed!");
        }
    }
protected:
    void incomingConnection(qintptr socketDescriptor) override
    {
        QTcpSocket *clientSocket = new QTcpSocket(this);
        clientSocket->setSocketDescriptor(socketDescriptor);

        qDebug() << "New client connected!";

        m_nextBlockSizes.insert(clientSocket, 0);


        connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    }

private slots:
    void onReadyRead(){
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;

        QDataStream in(clientSocket);
        in.setVersion(QDataStream::Qt_6_2);
        //[SIZE][JSON]
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

            if (type == "register"){
                handleRegister(clientSocket, request);
            } else if(type == "login"){
                handleLogin(clientSocket, request);
            } else if (type == "private_message") {
                handlePrivateMessage(clientSocket, request);
            } else if (type == "get_history") {
                handleGetHistory(clientSocket, request);
            } else if (type == "typing") {
                QString fromUser = loggedInUsers.key(clientSocket);
                QString to = request["toUser"].toString();
                QTcpSocket *toUserSocket = loggedInUsers.value(to, nullptr);
                if (toUserSocket) {
                    QJsonObject forwardMessage = request;
                    forwardMessage["fromUser"] = fromUser;
                    sendJson(toUserSocket, forwardMessage);
                }
            } else if (type == "delete_message") {
                handleDeleteMessage(clientSocket, request);
            } else if (type == "edit_message") {
                handleEditMessage(clientSocket, request);
            } else if(type == "message_delivered"){
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
            } else if(type == "message_read"){
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

        }
    }
    void onDisconnected(){
        QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (!clientSocket) return;


        m_nextBlockSizes.remove(clientSocket);

        QString username = loggedInUsers.key(clientSocket);
        if (!username.isEmpty()) {
            loggedInUsers.remove(username);
            qDebug() << "User" << username << "disconnected.";
            broadcastUserList();
        }
    }
private:
    bool initDatabase()
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
        return true;
    }
    void handleGetHistory(QTcpSocket* socket, const QJsonObject& request){
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
    void handleRegister(QTcpSocket* socket, const QJsonObject& request){
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
            for (QTcpSocket *socket : loggedInUsers.values()) {
                broadcastUserList();
            }
        } else {
            response["type"] = "register_failure";
            response["reason"] = "Username already exists.";
        }
        sendJson(socket, response);
    }
    void handleLogin(QTcpSocket* socket, const QJsonObject& request)
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
                broadcastUserList();
                sendOfflineMessages(socket, username);
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
    void handlePrivateMessage(QTcpSocket* fromUserSocket, const QJsonObject& request)
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
        //добавить остальные поля при необходимости
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
    void sendJson(QTcpSocket* socket, const QJsonObject& response)
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
    void sendFullUserList(QTcpSocket* socket)
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
            userObject["display_name"] = query.value(1).toString();
            allUsers.append(userObject);
        }

        QJsonObject message;
        message["type"] = "full_user_list";
        message["users"] = allUsers;
        sendJson(socket, message);
        qDebug() << "Sent full user list to a client:" << allUsers;
    }
    void broadcastUserList(){
        QStringList onlineUsers = loggedInUsers.keys();
        qDebug() << "Broadcasting ONLINE user list:" << onlineUsers;

        QJsonObject message;
        message["type"] = "user_list";
        message["users"] = QJsonArray::fromStringList(onlineUsers);

        for (QTcpSocket *socket : loggedInUsers.values()) {
            sendFullUserList(socket);
            sendJson(socket, message);
        }
    }
    void sendOfflineMessages(QTcpSocket* socket, const QString& username){ // доделать
        QSqlQuery selectQuery;
        selectQuery.prepare("SELECT id, fromUser, payload, timestamp, reply_to_id, is_edited "
                            "FROM messages "
                            "WHERE to_user = :toUser AND is_delivered = 0 "
                            "ORDER BY id ASC");
        selectQuery.bindValue(":toUser", username);
    }
    void handleEditMessage(QTcpSocket* socket, const QJsonObject& request){
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
    void handleDeleteMessage(QTcpSocket* socket, const QJsonObject& request){
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
private:
    QMap<QString, QTcpSocket*> loggedInUsers;
    QMap<QTcpSocket*, quint32> m_nextBlockSizes;

};

#include "main.moc"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "Server could not start!";
        return 1;
    }
    qInfo() << "Server started on port 1234.";
    return a.exec();
}

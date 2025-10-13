#ifndef SERVER_H
#define SERVER_H

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

#include <QWebSocketServer>
#include <QWebSocket>

#include "structures.h"

class QTcpSocket;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 tcpPort = 1234, quint16 wsPort = 8080);

protected:
    void initHandlers();
    void handleMessageDelivered(QObject* socket, const QJsonObject& request);
    void handleMessageRead(QObject* socket, const QJsonObject& request);
    void handleLogoutRequest(QObject* socket, const QJsonObject& request);


private slots:

    void onNewTcpConnection();
    void onTcpReadyRead();
    void onClientDisconnected();

    void onNewWebSocketConnection();
    void onWebSocketTextMessageReceived(const QString &message);

private:
    using Handler = void (Server::*)(QObject*, const QJsonObject&);
    bool initDatabase();
    void handleGetHistory(QObject* socket, const QJsonObject& request);
    void handleRegister(QObject* socket, const QJsonObject& request);
    void handleSearchUsers(QObject* socket, const QJsonObject& request);
    void sendContactList(QObject* socket,const QString& username);
    void handleAddContactRequest(QObject* socket, const QJsonObject& request);
    void handleLogin(QObject* socket, const QJsonObject& request);
    void handlePrivateMessage(QObject* fromUserSocket, const QJsonObject& request);
    void handleEditMessage(QObject* socket, const QJsonObject& request);
    void handleDeleteMessage(QObject* socket, const QJsonObject& request);
    void handleContactRequestResponse(QObject* socket, const QJsonObject& request);
    void handleTyping(QObject *socket, const QJsonObject &request);
    void sendJson(QObject* socket, const QJsonObject& response);
    void sendFullUserList(QObject* socket);
    void broadcastUserList();
    void sendOfflineMessages(QObject* socket, const QString& username);
    void sendOnlineStatusList(QObject* clientSocket);

    void sendPendingContactRequests(QObject* socket, const QString& username);

private:
    QTcpServer *m_tcpServer;
    QWebSocketServer *m_webSocketServer;

    QMap<QString, QObject*> m_clients;
    QMap<QObject*, QString> m_clientsReverse;

    void processJsonRequest(const QJsonObject& request, QObject* clientSocket);
    void removeClient(QObject* clientSocket);

    QMap<QTcpSocket*, quint32> m_nextBlockSizes;

    QMap<QString, Handler> m_handlers;

};


#endif  

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

#include "structures.h"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject *parent = nullptr);

protected:
    void initHandlers();
    void handleTyping(QTcpSocket* socket, const QJsonObject& request);
    void handleMessageDelivered(QTcpSocket* socket, const QJsonObject& request);
    void handleMessageRead(QTcpSocket* socket, const QJsonObject& request);
    void handleLogoutRequest(QTcpSocket* socket, const QJsonObject& request);

    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    using Handler = void (Server::*)(QTcpSocket*, const QJsonObject&);
    bool initDatabase();
    void handleGetHistory(QTcpSocket* socket, const QJsonObject& request);
    void handleRegister(QTcpSocket* socket, const QJsonObject& request);
    void handleSearchUsers(QTcpSocket* socket, const QJsonObject& request);
    void sendContactList(QTcpSocket* socket,const QString& username);
    void handleAddContactRequest(QTcpSocket* socket, const QJsonObject& request);
    void handleLogin(QTcpSocket* socket, const QJsonObject& request);
    void handlePrivateMessage(QTcpSocket* fromUserSocket, const QJsonObject& request);
    void sendJson(QTcpSocket* socket, const QJsonObject& response);
    void sendFullUserList(QTcpSocket* socket);
    void broadcastUserList();
    void sendOfflineMessages(QTcpSocket* socket, const QString& username); // доделать
    void handleEditMessage(QTcpSocket* socket, const QJsonObject& request);
    void handleDeleteMessage(QTcpSocket* socket, const QJsonObject& request);
    void handleContactRequestResponse(QTcpSocket* socket, const QJsonObject& request);
    void sendPendingContactRequests(QTcpSocket* socket, const QString& username);

private:
    QMap<QString, QTcpSocket*> loggedInUsers;
    QMap<QString, Handler> m_handlers;
    QMap<QTcpSocket*, quint32> m_nextBlockSizes;

};


#endif // SERVER_H

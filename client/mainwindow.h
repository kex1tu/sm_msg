#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchresultspopup.h"
#include <QMainWindow>
#include <QTcpSocket>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QTimer>
#include <QScrollBar>
#include <QMap>
#include <structures.h>
#include <optional>
#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onConnected();
    void onReadyRead();
    void onDisconnected();

    void handleGoToRegPageButtonClick();
    void handleGoToLogPageButtonClick();
    void handleLoginButtonClick();
    void handleRegisterButtonClick();
    void handleLogOutButtonClick();
    void performSearch();




    void handleUserSelectionChanged(QListWidgetItem *current);
    void onChatContextMenuRequested(const QPoint &pos);
    void onMessageDoubleClicked(QListWidgetItem *item);
    void onChatScroll(int value);
    void handleSendButtonClick();


    void onMessageTextChanged(const QString &text);

private:
    using ResponseHandler = void (MainWindow::*)(const QJsonObject&);
    QMap<QString, ResponseHandler> m_responseHandlers;

    void initResponseHandlers();
    void handleLoginSuccess(const QJsonObject& response);
    void handleLoginFailure(const QJsonObject& response);
    void handleRegisterSuccess(const QJsonObject& response);
    void handleRegisterFailure(const QJsonObject& response);
    void handleContactList(const QJsonObject& response);
    void handleHistoryData(const QJsonObject& response);
    void handleOldHistoryData(const QJsonObject& response);
    void handlePrivateMessage(const QJsonObject& response);
    void handleUserList(const QJsonObject& response);
    void handleMessageDelivered(const QJsonObject& response);
    void handleMessageRead(const QJsonObject& response);
    void handleEditMessage(const QJsonObject& response);
    void handleDeleteMessage(const QJsonObject& response);
    void handleSearchResults(const QJsonObject& response);
    void handleAddContactSuccess(const QJsonObject& response);
    void handleAddContactFailure(const QJsonObject& response);
    void handleIncomingContactRequest(const QJsonObject& response);
    void handlePendingRequestsList(const QJsonObject& response);
    void handleLogoutSuccess(const QJsonObject& response);
    void handleLogoutFailure(const QJsonObject& response);
    void handleTypingResponse(const QJsonObject& response);
    void showChatSearchUI();
    void hideChatSearchUI();
    void onChatSearchTriggered(const QString &text);

    void updateChatHeader();

    QString formatLastSeen(const User &user);

    Ui::MainWindow *ui;
    QTcpSocket *socket;

    QByteArray m_buffer;
    quint32 m_nextBlockSize;

    QString m_currentUsername;
    User m_currentChatPartner;
    QMap<QString, User> m_userCache;
    QMap<qint64, ChatMessage> m_currentChatMessages;
    QMap<QString, QTimer*> m_typingStatusTimers;
    QTimer *m_searchTimer;
    QTimer *m_typingTimer;
    QMap<QString, ChatMessage> m_pendingMessages;

    SearchResultsPopup *m_searchResultsPopup;

    qint64 m_replyToMessageId;
    qint64 m_editingMessageId;
    qint64 m_oldestMessageId;
    bool m_isLoadingHistory;
    QString m_forwardedFromUsername;

    void connectToServer();

    void sendJson(const QJsonObject& json);
    void addMessageToChat(const QJsonObject &messageObject);
    void displayMessage(const ChatMessage &message, int position = -1);
    void displaySendingMessage(const ChatMessage &message, int position = -1);
    void removeMessageById(qint64 messageId);
    void editMessageById(qint64 messageId, const QString newPayload);
    void updateUserListWidget();
    void updateMessageWidget(QListWidgetItem* item, const ChatMessage &msg);
    QListWidgetItem* findItemById(qint64 messageId,  std::optional<std::reference_wrapper<quint64>> posInWidget_optional = std::nullopt);
    QListWidgetItem* findItemByTempId(QString tempId);
    QString createTempId();
    void showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H

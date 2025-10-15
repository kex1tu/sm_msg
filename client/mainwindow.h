#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "chatfilterproxymodel.h"
#include "structures.h"
#include "contactlistdelegate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTcpSocket;
class QJsonObject;
class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QTimer;
class QPoint;
QT_END_NAMESPACE

class LoginWidget;
class ChatViewWidget;
class ProfileViewWidget;
class ChatMessageModel;
class SearchResultsPopup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();

    void onLoginRequested(const QString& username, const QString& password);
    void onRegisterRequested(const QString& username, const QString& displayName, const QString& password);
    void onSendMessageRequested(const QString& text);
    void onUserSelectionChanged(QListWidgetItem *current);
    void onLogoutButtonClicked();
    void onAddContactRequested(const QString& username);
    void onEditMessageRequested(qint64 messageId, const QString& oldText);
    void onDeleteMessageRequested(qint64 messageId);
    void onChatSearchTriggered(const QString &text);
    void showProfileView();
    void onReplyToMessage(qint64 messageId);
    void onSendMessageReadReceipt(qint64 messageId);
    void handleUnreadCounts(const QJsonObject& response);

    void updateMessageStatusInCacheAndModel(qint64 messageId, ChatMessage::MessageStatus newStatus);

    void processVisibleMessages();


    void onGlobalSearchTriggered();
    void onChatScroll(int value);
    void onTypingNotificationFired();
signals:
    void newMessageForCurrentChat();
private:
    void buildMainUI();
    void setupConnections();
    void initResponseHandlers();
    void connectToServer();
    void resetApplicationState();
    void updateContactItem(const QString& username);

    using ResponseHandler = void (MainWindow::*)(const QJsonObject&);
    QMap<QString, ResponseHandler> m_responseHandlers;
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
    void sendJson(const QJsonObject& json);
    void updateUserList();
    void updateChatHeader();
    void showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName);
    QMap<QString, int> m_unreadCounts;  
    QString formatLastSeen(const User &user);

private:
    QMap<QString, ChatCache> m_chatHistoryCache;
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    quint32 m_nextBlockSize;

    QString m_currentUsername;
    User m_currentChatPartner;
    bool m_isLoadingHistory = false;
    qint64 m_oldestMessageId = 0;
    qint64 m_editingMessageId = 0;
    qint64 m_replyToMessageId = 0;

    QMap<QString, User> m_userCache;

    LoginWidget* m_loginWidget;
    QWidget* m_mainChatWidget;

    QWidget* m_chatListPanel;
    QLineEdit* m_searchLineEdit;
    QListWidget* m_userListWidget;
    QPushButton* m_logoutButton;

    QWidget* m_rightSideContainer;
    QStackedLayout* m_rightSideLayout;
    QWidget* m_placeholderWidget;
    ChatViewWidget* m_chatViewWidget;
    ProfileViewWidget* m_profileViewWidget;

    ChatMessageModel* m_chatModel;
     

    SearchResultsPopup* m_searchResultsPopup;
    QTimer* m_globalSearchTimer;
    QTimer* m_typingSendTimer;
    QMap<QString, QTimer*> m_typingReceiveTimers;

    bool m_isChatSearchActive = false;
};

#endif  

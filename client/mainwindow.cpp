#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QJsonArray>
#include <QDataStream>
#include <QScrollBar>
#include <QMenu>
#include <QTcpSocket>
#include <QTimer>
#include <QUuid>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedLayout>
#include <QTextEdit>

#include "searchresultspopup.h"
#include "chatfilterproxymodel.h"
#include "loginwidget.h"
#include "chatviewwidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatmessagedelegate.h"
#include "chatmessagemodel.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_chatModel = new ChatMessageModel(this);

    m_loginWidget = new LoginWidget(this);
    buildMainUI();


    ui->rootStackedWidget->addWidget(m_loginWidget);
    ui->rootStackedWidget->addWidget(m_mainChatWidget);

    socket = new QTcpSocket(this);
    m_nextBlockSize = 0;

    m_globalSearchTimer = new QTimer(this);
    m_globalSearchTimer->setSingleShot(true);
    m_globalSearchTimer->setInterval(300);

    m_typingSendTimer = new QTimer(this);
    m_typingSendTimer->setSingleShot(true);
    m_typingSendTimer->setInterval(2000);

    m_searchResultsPopup = new SearchResultsPopup(this);
    qApp->installEventFilter(this);

    initResponseHandlers();

    setupConnections();

    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);

    connectToServer();
}


void MainWindow::buildMainUI()
{
    m_mainChatWidget = new QWidget();
    auto* mainLayout = new QHBoxLayout(m_mainChatWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(1);

    QWidget* chatListPanel = new QWidget();
    chatListPanel->setObjectName("chatListPanel");

    auto* leftLayout = new QVBoxLayout(chatListPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Поиск контактов...");

    m_userListWidget = new QListWidget();

    auto* contactDelegate = new ContactListDelegate(
        &m_userCache, &m_chatHistoryCache, &m_unreadCounts, &m_currentChatPartner.username, this
        );
    m_userListWidget->setItemDelegate(contactDelegate);

    m_logoutButton = new QPushButton("Выйти");
    m_logoutButton->setObjectName("logoutButton");

    leftLayout->addWidget(m_searchLineEdit);
    leftLayout->addWidget(m_userListWidget);
    leftLayout->addWidget(m_logoutButton);

    chatListPanel->setFixedWidth(320);

    mainLayout->addWidget(chatListPanel);

    m_rightSideContainer = new QWidget();
    m_rightSideContainer->setObjectName("rightSideContainer");

    m_rightSideLayout = new QStackedLayout();

    m_placeholderWidget = new QWidget();
    auto* placeholderLayout = new QVBoxLayout(m_placeholderWidget);
    auto* placeholderLabel = new QLabel("Выберите чат, чтобы начать общение");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setObjectName("placeholderLabel");
    placeholderLayout->addWidget(placeholderLabel);

    m_chatViewWidget = new ChatViewWidget();

    m_rightSideLayout->addWidget(m_placeholderWidget);
    m_rightSideLayout->addWidget(m_chatViewWidget);

    m_rightSideContainer->setLayout(m_rightSideLayout);

    mainLayout->addWidget(m_rightSideContainer, 1);


     

    QListView* chatView = m_chatViewWidget->chatHistoryView();

    chatView->setModel(m_chatModel);


    ChatMessageDelegate* delegate = new ChatMessageDelegate(m_chatModel, this);
    chatView->setItemDelegate(delegate);
}

void MainWindow::setupConnections()
{
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);


    connect(m_loginWidget, &LoginWidget::loginRequested, this, &MainWindow::onLoginRequested);
    connect(m_loginWidget, &LoginWidget::registerRequested, this, &MainWindow::onRegisterRequested);

    connect(m_chatViewWidget, &ChatViewWidget::sendMessageRequested, this, &MainWindow::onSendMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::headerClicked, this, &MainWindow::showProfileView);
    connect(m_chatViewWidget->messageTextEdit(), &QTextEdit::textChanged, this, &MainWindow::onTypingNotificationFired);
    connect(m_chatViewWidget->chatHistoryView()->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onChatScroll);
    connect(m_chatViewWidget, &ChatViewWidget::replyToMessageRequested, this, &MainWindow::onReplyToMessage);
    connect(m_chatViewWidget, &ChatViewWidget::editMessageRequested, this, &MainWindow::onEditMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::deleteMessageRequested, this, &MainWindow::onDeleteMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::replyCancelled, this, [this](){
        m_replyToMessageId = 0;
    });




    connect(m_userListWidget, &QListWidget::currentItemChanged, this, &MainWindow::onUserSelectionChanged);

    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        if (text.isEmpty()) m_globalSearchTimer->stop();
        else m_globalSearchTimer->start();
    });
    connect(m_globalSearchTimer, &QTimer::timeout, this, &MainWindow::onGlobalSearchTriggered);

    connect(m_searchResultsPopup, &SearchResultsPopup::userSelected, this, &MainWindow::onAddContactRequested);

    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);

    connect(this, &MainWindow::newMessageForCurrentChat, m_chatViewWidget, &ChatViewWidget::onNewMessageReceived);

    connect(m_chatModel, &ChatMessageModel::messageNeedsReadReceipt, this, &MainWindow::onSendMessageReadReceipt);
}
void MainWindow::onEditMessageRequested(qint64 messageId, const QString& oldText)
{
    qDebug() << "MainWindow: Caught editMessageRequested signal for ID:" << messageId;

    m_editingMessageId = messageId;
    m_chatViewWidget->setEditMode(true, oldText);
}

void MainWindow::onDeleteMessageRequested(qint64 messageId)
{
    QJsonObject request;
    request["type"] = "delete_message";
    request["id"] = messageId;
    sendJson(request);
}
void MainWindow::onAddContactRequested(const QString& username)
{
    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Добавить контакт",
                                  "Отправить запрос на добавление в контакты пользователю " + username + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "[CLIENT] Sending 'add_contact_request' for user:" << username;

        QJsonObject request;
        request["type"] = "add_contact_request";
        request["username"] = username;
        sendJson(request);
    }
}

void MainWindow::onTypingNotificationFired()
{
    if (m_currentChatPartner.username.isEmpty()) return;

    if (m_chatViewWidget->messageTextEdit()->toPlainText().isEmpty()) return;

    if (m_typingSendTimer->isActive()) return;

    QJsonObject typingRequest;
    typingRequest["type"] = "typing";
    typingRequest["toUser"] = m_currentChatPartner.username;
    sendJson(typingRequest);

    m_typingSendTimer->start();
}
void MainWindow::onReplyToMessage(qint64 messageId)
{
    qDebug() << "[CLIENT] Setting reply context to message ID:" << messageId;
    ChatMessage msg;
    if (m_chatModel->getMessageById(messageId, msg)) {
        m_replyToMessageId = messageId;
        m_chatViewWidget->showReplyUI(msg.fromUser, msg.payload);
    }
}


void MainWindow::onChatSearchTriggered(const QString &text)
{
     
}

void MainWindow::connectToServer(){
    const QString host = "127.0.0.1";
    const quint16 port = 1234;
    socket->connectToHost(host, port);
    qDebug()<< "[Client] Attempting to connect to host on " << host << ":" << port;
}
void MainWindow::onConnected(){
    qDebug() << "[CLIENT] Socket successfully connected!";

    if (ui->statusbar) {
        ui->statusbar->showMessage("Подключено", 2000);
    }

    m_loginWidget->setUiEnabled(true);
}
void MainWindow::onDisconnected(){
    qDebug() << "[CLIENT] Socket disconnected!";

    if (ui->statusbar) {
        ui->statusbar->showMessage("Отключено");
    }

    m_loginWidget->setUiEnabled(false);

    if (ui->rootStackedWidget->currentWidget() == m_mainChatWidget) {
    }
}
void MainWindow::sendJson(const QJsonObject& json)
{
    qDebug() << "---------------------------------";
    qDebug() << "[CLIENT SENDING] Preparing to send JSON of type:" << json["type"].toString();
    qDebug() << "[CLIENT SENDING] Full JSON content:" << json;
    qDebug() << "---------------------------------";
    QByteArray jsonData = QJsonDocument(json).toJson();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << (quint32)0;
    out << jsonData;
    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    socket->write(block);
}
void MainWindow::onReadyRead(){
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);
    while(true){
        if (m_nextBlockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32)){
                break;
            }
            in >> m_nextBlockSize;
        }

        if(socket->bytesAvailable() < m_nextBlockSize){
            break;
        }

        QByteArray JsonData;
        in >> JsonData;
        m_nextBlockSize = 0;


        QJsonDocument doc = QJsonDocument::fromJson(JsonData);
        if(doc.isNull() || !doc.isObject()){
            qDebug() << "[CLIENT] Failed to parse JSON or it's not an object.";
            continue;
        }

        QJsonObject response = doc.object();
        QString type = response["type"].toString();
        qDebug() << "[CLIENT] Processing message of type" << type;


        if (m_responseHandlers.contains(type)) {
            ResponseHandler handler = m_responseHandlers[type];
            (this->*handler)(response);

        } else {
            qDebug() << "[CLIENT] No handler found for message type:" << type;
        }
    }

}

void MainWindow::initResponseHandlers()
{
    m_responseHandlers["login_success"] = &MainWindow::handleLoginSuccess;
    m_responseHandlers["login_failure"] = &MainWindow::handleLoginFailure;
    m_responseHandlers["register_success"] = &MainWindow::handleRegisterSuccess;
    m_responseHandlers["register_failure"] = &MainWindow::handleRegisterFailure;
    m_responseHandlers["contact_list"] = &MainWindow::handleContactList;
    m_responseHandlers["history_data"] = &MainWindow::handleHistoryData;
    m_responseHandlers["old_history_data"] = &MainWindow::handleOldHistoryData;
    m_responseHandlers["private_message"] = &MainWindow::handlePrivateMessage;
    m_responseHandlers["user_list"] = &MainWindow::handleUserList;
    m_responseHandlers["message_delivered"] = &MainWindow::handleMessageDelivered;
    m_responseHandlers["message_read"] = &MainWindow::handleMessageRead;
    m_responseHandlers["edit_message"] = &MainWindow::handleEditMessage;
    m_responseHandlers["delete_message"] = &MainWindow::handleDeleteMessage;
    m_responseHandlers["search_results"] = &MainWindow::handleSearchResults;
    m_responseHandlers["add_contact_success"] = &MainWindow::handleAddContactSuccess;
    m_responseHandlers["add_contact_failure"] = &MainWindow::handleAddContactFailure;
    m_responseHandlers["incoming_contact_request"] = &MainWindow::handleIncomingContactRequest;
    m_responseHandlers["pending_requests_list"] = &MainWindow::handlePendingRequestsList;
    m_responseHandlers["logout_request_success"] = &MainWindow::handleLogoutSuccess;
    m_responseHandlers["logout_request_failure"] = &MainWindow::handleLogoutFailure;
    m_responseHandlers["typing"] = &MainWindow::handleTypingResponse;
    m_responseHandlers["unread_counts"] = &MainWindow::handleUnreadCounts;
}
void MainWindow::handleLoginSuccess(const QJsonObject& response){
    m_currentUsername = m_loginWidget->username();
    qDebug() << "[CLIENT] Login successful for user:" << m_currentUsername;

    m_loginWidget->clearFields();

    ui->rootStackedWidget->setCurrentWidget(m_mainChatWidget);

    this->setWindowTitle(m_currentUsername);
}
void MainWindow::handleUnreadCounts(const QJsonObject& response)
{
    qDebug() << "[CLIENT] Received initial unread counts from server.";
    QJsonArray countsArray = response["counts"].toArray();

     
    m_unreadCounts.clear();

    for (const QJsonValue &value : countsArray) {
        QJsonObject countObj = value.toObject();
        QString username = countObj["username"].toString();
        int count = countObj["count"].toInt();

        if (count > 0) {
            m_unreadCounts[username] = count;
        }
    }

     
     
     
     
    updateUserList();
}

void MainWindow::handleLoginFailure(const QJsonObject& response){
    QMessageBox::warning(this, "error login", response["reason"].toString());
}
void MainWindow::handleRegisterSuccess(const QJsonObject& response){
    qDebug() << "[CLIENT] Registration successful.";

    m_loginWidget->onRegistrationSuccess();
}
void MainWindow::handleRegisterFailure(const QJsonObject& response){
    QMessageBox::warning(this, "error registration", response["reason"].toString());
}
void MainWindow::handleContactList(const QJsonObject& response)
{
    qDebug() << "contact_list recieved";
    QJsonArray usersFromServer = response["users"].toArray();
    m_userCache.clear();

    for (const QJsonValue &value : usersFromServer) {
        QJsonObject userObj = value.toObject();
        User user;
        user.username = userObj["username"].toString();
        user.displayName = userObj["displayname"].toString();
        user.lastSeen = userObj["last_seen"].toString();
        qDebug() << user.username;

        if (user.username != m_currentUsername) {
            m_userCache.insert(user.username, user);
        }
    }

    updateUserList();
}

void MainWindow::handleHistoryData(const QJsonObject& response)
{
    QString historyForUser = response["with_user"].toString();
    if (historyForUser != m_currentChatPartner.username) return;

    m_chatModel->clearMessages();

    QJsonArray history = response["history"].toArray();
    qDebug() << "[CLIENT] Displaying initial" << history.count() << "messages for" << historyForUser;

    QList<ChatMessage> messages;
    for (const QJsonValue &value : history) {
        ChatMessage msg;
        QJsonObject msgObj = value.toObject();
        msg.id = msgObj["id"].toDouble();
        msg.fromUser = msgObj["fromUser"].toString();
        msg.toUser = msgObj["toUser"].toString();
        msg.payload = msgObj["payload"].toString();
        msg.timestamp = msgObj["timestamp"].toString();
        msg.replyToId = msgObj["reply_to_id"].toDouble();
        msg.isOutgoing = (msg.fromUser == m_currentUsername);
        msg.isEdited = msgObj["is_edited"].toInt();
        if(msgObj["is_delivered"].toInt() == 1){
            msg.status = ChatMessage::Delivered;
        }
        else{
            msg.status = ChatMessage::Sent;
        }
        if(msgObj["is_read"].toInt() == 1){
            msg.status = ChatMessage::Read;
        }
        if(msg.isOutgoing == false){
            if(msg.status != ChatMessage::Delivered && msg.status != ChatMessage::Read ){
                QJsonObject deliveredCmd;
                deliveredCmd["type"] = "message_delivered";
                deliveredCmd["id"] = (double)msg.id;
                qDebug() << "[CLIENT] message " << (double)msg.id << "delivered, sending this info to server";
                sendJson(deliveredCmd);
                msg.status = ChatMessage::Delivered;
            }
            if(msg.status != ChatMessage::Read){
                if (msg.fromUser == m_currentChatPartner.username) {

                    QJsonObject readCmd;
                    readCmd["type"] = "message_read";
                    readCmd["id"] = (double)msg.id;
                    qDebug() << "[CLIENT] message " << (double)msg.id << "read, sending this info to server";
                    sendJson(readCmd);
                    msg.status = ChatMessage::Read;
                }
            }
        }
        messages.append(msg);
    }

    m_chatModel->addMessages(messages);
    ChatCache& cache = m_chatHistoryCache[historyForUser];
    cache.messages = messages;  
    if (!messages.isEmpty()) {
        cache.oldestMessageId = messages.first().id;
    } else {
        cache.allMessagesLoaded = true;  
    }
     
    m_chatModel->clearMessages();
    m_chatModel->addMessages(messages);
    m_oldestMessageId = cache.oldestMessageId;

    QMetaObject::invokeMethod(m_chatViewWidget->chatHistoryView(), "scrollToBottom", Qt::QueuedConnection);

    if (!history.isEmpty()) {
        m_oldestMessageId = history.first().toObject()["id"].toDouble();
    } else {
        m_oldestMessageId = 0;
    }
    m_isLoadingHistory = false;
}
void MainWindow::handlePrivateMessage(const QJsonObject& response){

    QString tempId = response["temp_id"].toString();
    qDebug() << tempId;
    if (!tempId.isEmpty()) {

        qDebug() << "[CLIENT] Received ECHO for temp_id:" << tempId;

        ChatMessage msg;
        msg.id = response["id"].toDouble();
        msg.tempId = tempId;
        msg.fromUser = response["fromUser"].toString();
        msg.toUser = response["toUser"].toString();
        msg.payload = response["payload"].toString();
        msg.timestamp = response["timestamp"].toString();
        msg.replyToId = response["reply_to_id"].toDouble();
        msg.isOutgoing = true;
        msg.status = ChatMessage::MessageStatus::Sent;
        msg.isEdited = false;

        m_chatModel->confirmMessage(tempId, msg);
         
         
        QString chatPartner = msg.toUser;  
        if (m_chatHistoryCache.contains(chatPartner)) {
             
            QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
            for (int i = 0; i < messagesInCache.size(); ++i) {
                if (messagesInCache[i].tempId == tempId) {
                    messagesInCache[i] = msg;
                    qDebug() << "[CACHE] Confirmed message with temp_id:" << tempId << "in cache for" << chatPartner;
                    break;  
                }
            }
        }
         
        return;
    }

    ChatMessage incomingMsg;
    incomingMsg.id = response["id"].toDouble();
    incomingMsg.fromUser = response["fromUser"].toString();
    incomingMsg.toUser = response["toUser"].toString();
    incomingMsg.payload = response["payload"].toString();
    incomingMsg.timestamp = response["timestamp"].toString();
    incomingMsg.replyToId = response["reply_to_id"].toDouble();
    incomingMsg.isOutgoing = false;
    incomingMsg.isEdited = false;
    updateContactItem(incomingMsg.fromUser);
    if(response["is_delivered"].toInt() == 1){
        incomingMsg.status = ChatMessage::Delivered;
    }
    else{
        incomingMsg.status = ChatMessage::Sent;
    }
    if(response["is_read"].toInt() == 1){
        incomingMsg.status = ChatMessage::Read;
    }

    QString chatPartner = incomingMsg.fromUser;
    if (m_chatHistoryCache.contains(chatPartner)) {
        m_chatHistoryCache[chatPartner].messages.append(incomingMsg);
    }

    QJsonObject deliveredCmd;
    deliveredCmd["type"] = "message_delivered";
    deliveredCmd["id"] = (double)incomingMsg.id;
    qDebug() << "[CLIENT] message " << (double)incomingMsg.id << "delivered, sending this info to server";
    sendJson(deliveredCmd);


    if (incomingMsg.fromUser == m_currentChatPartner.username) {
        bool wasScrolledToBottom = m_chatViewWidget->isScrolledToBottom();

         
        m_chatModel->addMessage(incomingMsg);
        if (wasScrolledToBottom) {
             
            QMetaObject::invokeMethod(m_chatViewWidget, "scrollToBottom", Qt::QueuedConnection);
        } else {
             
            emit newMessageForCurrentChat();
        }
    } else {
         
        m_unreadCounts[incomingMsg.fromUser]++;

         
        for (int i = 0; i < m_userListWidget->count(); ++i) {
            QListWidgetItem* item = m_userListWidget->item(i);
            if (item->data(Qt::UserRole).toString() == incomingMsg.fromUser) {
                m_userListWidget->update(m_userListWidget->indexFromItem(item));
                break;
            }
        }
    }

    if (incomingMsg.fromUser != m_currentChatPartner.username || isMinimized() || !isActiveWindow()) {
        QApplication::alert(this);
    }
}
void MainWindow::handleUserList(const QJsonObject& response){
    QJsonArray onlineUsernamesArray = response["users"].toArray();
    QSet<QString> onlineUsers;
    for (const QJsonValue &value : onlineUsernamesArray) {
        onlineUsers.insert(value.toString());
        qDebug()<< value.toString();
    }

    for (auto it = m_userCache.begin(); it != m_userCache.end(); ++it) {
        if (onlineUsers.contains(it.value().username)) {
            it->isOnline = true;
        } else {
            it->isOnline = false;
        }
    }
    updateUserList();

}
void MainWindow::handleMessageDelivered(const QJsonObject& response)
{


    qint64 messageId = response["id"].toDouble();
    qDebug() << "[CLIENT] Message" << messageId << "delivered.";

    updateMessageStatusInCacheAndModel(messageId, ChatMessage::Delivered);
}

void MainWindow::handleMessageRead(const QJsonObject& response)
{
    qint64 messageId = response["id"].toDouble();
    qDebug() << "[CLIENT] Message" << messageId << "read.";

    updateMessageStatusInCacheAndModel(messageId, ChatMessage::Read);
}
void MainWindow::handleEditMessage(const QJsonObject& response)
{

    QString chatPartner = response["with_user"].toString();
    qint64 messageId = response["id"].toDouble();
    QString newPayload = response["payload"].toString();

    qDebug() << "[CLIENT] Received command to edit message" << messageId;
    if (m_chatHistoryCache.contains(chatPartner)) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                messagesInCache[i].payload = newPayload;
                messagesInCache[i].isEdited = true;
                qDebug() << "[CACHE] Сообщение" << messageId << "отредактировано в кэше для" << chatPartner;

                break;
            }
        }
    }
     

     
    if (chatPartner == m_currentChatPartner.username) {
        m_chatModel->editMessage(messageId, newPayload);
    }
     
     
     
    updateContactItem(chatPartner);
     

     
}

void MainWindow::handleDeleteMessage(const QJsonObject& response){
    qint64 messageId = response["id"].toDouble();
    QString chatUser = response["with_user"].toString();
    QString currentUser = m_currentUsername;

     
    QString chatPartner = (chatUser == currentUser) ? m_currentChatPartner.username : chatUser;

    qDebug() << "[CLIENT] Received command to delete message" << messageId << "in chat with user" << chatPartner;

     
     
    if (m_chatHistoryCache.contains(chatPartner)) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                messagesInCache.removeAt(i);
                qDebug() << "[CACHE] Сообщение" << messageId << "удалено из кэша для" << chatPartner;
                break;
            }
        }
    }
     

     
    if (chatPartner == m_currentChatPartner.username) {
        m_chatModel->removeMessage(messageId);
    }
     
     
     
    updateContactItem(chatPartner);
     

}
void MainWindow::updateMessageStatusInCacheAndModel(qint64 messageId, ChatMessage::MessageStatus newStatus)
{
    QString foundInChatWith;

     
     
    int foundAtIndex = -1;

    for (auto it = m_chatHistoryCache.begin(); it != m_chatHistoryCache.end(); ++it) {
        QList<ChatMessage>& messages = it.value().messages;
        for (int i = 0; i < messages.size(); ++i) {
            if (messages[i].id == messageId) {
                if (messages[i].status == newStatus) return;
                messages[i].status = newStatus;
                foundInChatWith = it.key();  
                qDebug() << "[CACHE] Обновлен статус для сообщения" << messageId << "в кэше чата с" << foundInChatWith;
                if (!foundInChatWith.isEmpty() && foundInChatWith == m_currentChatPartner.username) {
                    m_chatModel->updateMessageStatus(messageId, newStatus);
                }

                foundAtIndex = i;
                if (foundAtIndex != -1 && foundInChatWith == m_currentChatPartner.username) {
                     
                     
                    QModelIndex modelIndex = m_chatModel->index(foundAtIndex, 0);

                     
                     
                    m_chatModel->setData(modelIndex, QVariant::fromValue(m_chatHistoryCache[foundInChatWith].messages[foundAtIndex]), Qt::UserRole);

                     
                     
                }
                return;

            }
        }

    }
}

void MainWindow::handleSearchResults(const QJsonObject& response)
{
    QJsonArray users = response["users"].toArray();


    QWidget *searchBar = m_searchLineEdit;

    m_searchResultsPopup->move(searchBar->mapToGlobal(QPoint(0, searchBar->height())));
    m_searchResultsPopup->setFixedWidth(searchBar->width());

    m_searchResultsPopup->showResults(users);

    QTimer::singleShot(0, this, [this]() {
        m_searchLineEdit->setFocus();
    });
}

void MainWindow::handleAddContactSuccess(const QJsonObject& response){
    QMessageBox::information(this, "Success", response["reason"].toString());
}
void MainWindow::handleAddContactFailure(const QJsonObject& response){
    QMessageBox::warning(this, "Error", response["reason"].toString());
}
void MainWindow::handleIncomingContactRequest(const QJsonObject& response){
    QString fromUsername = response["fromUsername"].toString();
    QString fromDisplayName = response["fromDisplayname"].toString();
    showContactRequestPrompt(fromUsername, fromDisplayName);
}
void MainWindow::handlePendingRequestsList(const QJsonObject& response){
    QJsonArray requests = response["requests"].toArray();

    for (const QJsonValue &value : requests) {
        QJsonObject reqObj = value.toObject();
        QString fromUsername = reqObj["fromUsername"].toString();
        QString fromDisplayName = reqObj["fromDisplayname"].toString();
        showContactRequestPrompt(fromUsername, fromDisplayName);
    }
}
void MainWindow::handleLogoutSuccess(const QJsonObject& response){
    qDebug() << "[CLIENT] Logout successful. Resetting application state.";

    QMessageBox::warning(this, "Success", "Congrats");

    resetApplicationState();

}

void MainWindow::resetApplicationState()
{
    m_currentUsername.clear();
    m_currentChatPartner = User();

    m_chatHistoryCache.clear();  
    m_userCache.clear();
    m_chatModel->clearMessages();
    m_userListWidget->clear();

    qDeleteAll(m_typingReceiveTimers);
    m_typingReceiveTimers.clear();

    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }
    m_oldestMessageId = 0;
    m_isLoadingHistory = false;

    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);

}

void MainWindow::handleLogoutFailure(const QJsonObject& response){
    QMessageBox::warning(this, "Error", response["reason"].toString());
}
void MainWindow::onLoginRequested(const QString& username, const QString& password){
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя пользователя и пароль не могут быть пустыми.");
        return;
    }

    qDebug() << "[CLIENT] MainWindow: Login requested for user:" << username;

    QJsonObject loginRequest;
    loginRequest["type"] = "login";
    loginRequest["username"] = username;
    loginRequest["password"] = password;
    sendJson(loginRequest);
}

void MainWindow::onRegisterRequested(const QString& username, const QString& displayName, const QString& password){
    if (username.isEmpty() || password.isEmpty() || displayName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены.");
        return;
    }

    qDebug() << "[CLIENT] MainWindow: Register requested for user:" << username;

    QJsonObject registerRequest;
    registerRequest["type"] = "register";
    registerRequest["username"] = username;
    registerRequest["password"] = password;
    registerRequest["display_name"] = displayName;
    sendJson(registerRequest);
}
void MainWindow::onSendMessageRequested(const QString& text)
{
    if (text.isEmpty() || m_currentChatPartner.username.isEmpty()) {
        return;
    }
    if (m_editingMessageId > 0) {
         
        qDebug() << "[CLIENT] Sending 'edit_message' request for ID:" << m_editingMessageId;

        QJsonObject request;
        request["type"] = "edit_message";
        request["id"] = m_editingMessageId;
        request["payload"] = text;
        sendJson(request);

         
        m_editingMessageId = 0;
        m_chatViewWidget->setEditMode(false);
    } else {

        ChatMessage msg;
        msg.fromUser = m_currentUsername;
        msg.toUser = m_currentChatPartner.username;
        msg.payload = text;
        msg.status = ChatMessage::Sending;
        msg.isOutgoing = true;
        msg.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        msg.tempId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        msg.replyToId = m_replyToMessageId;
        m_chatModel->addMessage(msg);
        m_chatViewWidget->chatHistoryView()->scrollToBottom();
        QString chatPartner = m_currentChatPartner.username;
        if (m_chatHistoryCache.contains(chatPartner)) {
            m_chatHistoryCache[chatPartner].messages.append(msg);
            qDebug() << "[CACHE] Добавлено временное сообщение с temp_id:" << msg.tempId << "в кэш для" << chatPartner;
        }

        QJsonObject request;
        request["type"] = "private_message";
        request["fromUser"] = msg.fromUser;
        request["toUser"] = msg.toUser;
        request["payload"] = msg.payload;
        request["reply_to_id"] = msg.replyToId;
        request["temp_id"] = msg.tempId;
        sendJson(request);

        if (m_replyToMessageId > 0) {
            m_replyToMessageId = 0;
            m_chatViewWidget->hideReplyUI();
        }

    }
    updateContactItem(m_currentChatPartner.username);



}

void MainWindow::onLogoutButtonClicked()
{
    qDebug() << "[CLIENT] MainWindow: Logout button clicked.";

    QJsonObject logoutRequest;
    logoutRequest["type"] = "logout_request";
    logoutRequest["username"] = m_currentUsername;
    sendJson(logoutRequest);
}

void MainWindow::onUserSelectionChanged(QListWidgetItem *current)
{
    qDebug() << "--- onUserSelectionChanged START ---";
    if (m_isChatSearchActive) {
    }
    if (m_replyToMessageId > 0) {
        m_replyToMessageId = 0;
        m_chatViewWidget->hideReplyUI();
    }
    if (!current) {
        qDebug() << "Current item is null, resetting view.";
        if(m_rightSideLayout) m_rightSideLayout->setCurrentWidget(m_placeholderWidget);
        m_currentChatPartner = User();
        qDebug() << "--- onUserSelectionChanged END (reset) ---";
        return;
    }

    qDebug() << "Step 1: Item selected. Text:" << current->text();

    QString selectedUsername = current->data(Qt::UserRole).toString();
     
    qDebug() << "Step 2: Got username from UserRole:" << selectedUsername;



    if (m_unreadCounts.value(selectedUsername, 0) > 0) {
        m_unreadCounts[selectedUsername] = 0;
         
        m_userListWidget->update(m_userListWidget->indexFromItem(current));
    }

    if (selectedUsername.isEmpty() || !m_userCache.contains(selectedUsername)) {
        qWarning() << "CRITICAL: Selected user not found in cache or username is empty!";
        qDebug() << "--- onUserSelectionChanged END (error) ---";
        return;
    }

    qDebug() << "Step 3: User found in cache.";
    m_currentChatPartner = m_userCache.value(selectedUsername);
    qDebug() << "Step 4: m_currentChatPartner is set to:" << m_currentChatPartner.displayName;
    updateUserList();
     


    m_isLoadingHistory = true;
    m_oldestMessageId = -1;
    qDebug() << "Step 5: Pagination state reset.";

    if (!m_chatViewWidget) {
        qWarning() << "CRITICAL: m_chatViewWidget is a nullptr!";
        return;
    }
    m_chatViewWidget->updateHeader(m_currentChatPartner);
    updateUserList();
    qDebug() << "Step 6: ChatView header updated.";

    if (!m_chatModel) {
        qWarning() << "CRITICAL: m_chatModel is a nullptr!";
        return;
    }
    m_chatModel->clearMessages();
    qDebug() << "Step 7: Chat model cleared.";

    if (!m_rightSideLayout) {
        qWarning() << "CRITICAL: m_rightSideLayout is a nullptr!";
        return;
    }
    m_rightSideLayout->setCurrentWidget(m_chatViewWidget);
    qDebug() << "Step 8: Switched to ChatViewWidget.";

     
    if (m_chatHistoryCache.contains(selectedUsername)) {
         
        qDebug() << "[CACHE] Hit for user:" << selectedUsername << ". Loading from memory.";
        const ChatCache& cache = m_chatHistoryCache.value(selectedUsername);

        m_chatModel->clearMessages();
        m_chatModel->addMessages(cache.messages);  

        m_oldestMessageId = cache.oldestMessageId;  
        m_isLoadingHistory = false;

        QMetaObject::invokeMethod(m_chatViewWidget->chatHistoryView(), "scrollToBottom", Qt::QueuedConnection);

    } else {
         
        qDebug() << "[CACHE] Miss for user:" << selectedUsername << ". Requesting from server.";
        m_isLoadingHistory = true;
        m_oldestMessageId = -1;
        m_chatModel->clearMessages();

        QJsonObject request;
        request["type"] = "get_history";
        request["with_user"] = m_currentChatPartner.username;
        sendJson(request);
    }

    qDebug() << "--- onUserSelectionChanged END (success) ---";
}

void MainWindow::showProfileView()
{
    qDebug() << "[CLIENT] Showing profile for" << m_currentChatPartner.username;
    QMessageBox::information(this, "Профиль", "Здесь будет показан профиль пользователя " + m_currentChatPartner.displayName);
}



void MainWindow::showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName)
{
    QString questionText = QString("Пользователь %1 (@%2) хочет добавить вас в список контактов. Принять запрос?")
                               .arg(fromDisplayName, fromUsername);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Запрос на добавление в контакты", questionText,
                                  QMessageBox::Yes | QMessageBox::No);

    QJsonObject contactResponse;
    contactResponse["type"] = "contact_request_response";
    contactResponse["fromUsername"] = fromUsername;

    if (reply == QMessageBox::Yes) {
        contactResponse["response"] = "accepted";
    } else {
        contactResponse["response"] = "declined";
    }
    sendJson(contactResponse);
}
 
void MainWindow::updateUserList()
{
     
     
    QMap<QString, QListWidgetItem*> itemsByUsername;
    for (int i = 0; i < m_userListWidget->count(); ++i) {
        QListWidgetItem* item = m_userListWidget->item(i);
        itemsByUsername.insert(item->data(Qt::UserRole).toString(), item);
    }

     
    for (const User& user : m_userCache) {
        if (itemsByUsername.contains(user.username)) {
             
            QListWidgetItem* item = itemsByUsername.value(user.username);
            item->setText(user.displayName);

             
            QFont font = item->font();
            if (user.isOnline) {
                font.setBold(true);
                item->setForeground(QColor(Qt::white));
            } else {
                font.setBold(false);
                item->setForeground(QColor(Qt::gray));
            }
            item->setFont(font);

             
             

            m_userListWidget->update(m_userListWidget->indexFromItem(item));

             
            itemsByUsername.remove(user.username);
        } else {
             
            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, user.username);
            m_userListWidget->addItem(item);
        }
    }

     
     
    for (QListWidgetItem* staleItem : itemsByUsername.values()) {
        delete staleItem;  
    }
}

 


void MainWindow::onGlobalSearchTriggered()
{
    QString query = m_searchLineEdit->text().trimmed();

    if (query.isEmpty()) {
        if (m_searchResultsPopup && m_searchResultsPopup->isVisible()) {
            m_searchResultsPopup->hide();
        }
        return;
    }

    qDebug() << "[CLIENT] Triggered global user search for:" << query;

    QJsonObject request;
    request["type"] = "search_users";
    request["term"] = query;
    sendJson(request);
}
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{

    if (event->type() == QEvent::MouseButtonPress && m_searchResultsPopup->isVisible()) {
        if (!m_searchResultsPopup->geometry().contains(QCursor::pos())) {
            m_searchResultsPopup->hide();
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
void MainWindow::onChatScroll(int value){
    if (m_isChatSearchActive) {
        return;
    }
    if (m_isLoadingHistory) {
        return;
    }

    if (value == 0 && !m_isLoadingHistory && m_oldestMessageId != 0) {
        qDebug() << "[CLIENT] Scrolled to top. Requesting older history before ID:" << m_oldestMessageId;


        m_isLoadingHistory = true;

        QJsonObject request;
        request["type"] = "get_history";
        request["with_user"] = m_currentChatPartner.username;
        request["before_id"] = m_oldestMessageId;
        sendJson(request);
    }
}

void MainWindow::handleOldHistoryData(const QJsonObject& response){
    QString historyForUser = response["with_user"].toString();

    if (historyForUser != m_currentChatPartner.username) {

        m_isLoadingHistory = false;
        return;
    }

    QJsonArray history = response["history"].toArray();
    if (history.isEmpty()) {
        m_oldestMessageId = 0;
        m_isLoadingHistory = false;
        return;
    }
    qDebug() << "[CLIENT] Displaying" << history.count() << "history messages for" << historyForUser;



    QListView* chatView = m_chatViewWidget->chatHistoryView();
    QScrollBar* scrollBar = chatView->verticalScrollBar();

     
    int oldScrollMax = scrollBar->maximum();
    QList<ChatMessage> messages;

    for (int i = 0; i < history.count(); ++i) {
        const QJsonValue &value = history[i];
        ChatMessage msg;
        QJsonObject msgObj = value.toObject();
        msg.id = msgObj["id"].toDouble();
        msg.fromUser = msgObj["fromUser"].toString();
        msg.toUser = msgObj["toUser"].toString();
        msg.payload = msgObj["payload"].toString();
        msg.timestamp = msgObj["timestamp"].toString();
        msg.replyToId = msgObj["reply_to_id"].toDouble();
        msg.isOutgoing = (msg.fromUser == m_currentUsername);
        msg.isEdited = msgObj["is_edited"].toInt();
        if(msgObj["is_delivered"].toInt() == 1){
            msg.status = ChatMessage::Delivered;
        }
        else{
            msg.status = ChatMessage::Sent;
        }
        if(msgObj["is_read"].toInt() == 1){
            msg.status = ChatMessage::Read;
        }
        if(msg.isOutgoing == false){
            if(msg.status != ChatMessage::Delivered && msg.status != ChatMessage::Read ){
                QJsonObject deliveredCmd;
                deliveredCmd["type"] = "message_delivered";
                deliveredCmd["id"] = (double)msg.id;
                qDebug() << "[CLIENT] message " << (double)msg.id << "delivered, sending this info to server";
                sendJson(deliveredCmd);
                msg.status = ChatMessage::Delivered;

            }
            if(msg.status != ChatMessage::Read){
                if (msg.fromUser == m_currentChatPartner.username) {

                    QJsonObject readCmd;
                    readCmd["type"] = "message_read";
                    readCmd["id"] = (double)msg.id;
                    qDebug() << "[CLIENT] message " << (double)msg.id << "read, sending this info to server";
                    sendJson(readCmd);
                    msg.status = ChatMessage::Read;
                }
            }
        }
        messages.append(msg);
    }



    ChatCache& cache = m_chatHistoryCache[historyForUser];

     
    for (int i = messages.count() - 1; i >= 0; --i) {
        cache.messages.prepend(messages.at(i));
    }

    if (!history.isEmpty()) {
        cache.oldestMessageId = history.first().toObject()["id"].toDouble();
    } else {
        cache.allMessagesLoaded = true;
    }

    m_chatModel->prependMessages(messages);
    m_oldestMessageId = cache.oldestMessageId;

    qDebug() << m_oldestMessageId;


    qDebug() << oldScrollMax;
    QApplication::processEvents();

    int newScrollMax = scrollBar->maximum();
    scrollBar->setValue(newScrollMax - oldScrollMax);

    m_isLoadingHistory = false;
     
}
void MainWindow::updateContactItem(const QString& username)
{
    for (int i = 0; i < m_userListWidget->count(); ++i) {
        QListWidgetItem* item = m_userListWidget->item(i);
        if (item->data(Qt::UserRole).toString() == username) {
            m_userListWidget->update(m_userListWidget->indexFromItem(item));
            return;
        }
    }
}


void MainWindow::handleTypingResponse(const QJsonObject& response)
{
    QString fromUser = response["fromUser"].toString();

     
    if (!m_userCache.contains(fromUser)) return;

     
    m_userCache[fromUser].isTyping = true;


     
     
    if (fromUser == m_currentChatPartner.username) {
        m_currentChatPartner =  m_userCache[fromUser];
        m_chatViewWidget->updateHeader(m_currentChatPartner);
    } else{
        updateContactItem(fromUser);
    }
     


     
    if (!m_typingReceiveTimers.contains(fromUser)) {
        m_typingReceiveTimers[fromUser] = new QTimer(this);
        m_typingReceiveTimers[fromUser]->setInterval(2000);
        m_typingReceiveTimers[fromUser]->setSingleShot(true);

        connect(m_typingReceiveTimers[fromUser], &QTimer::timeout, this, [this, fromUser](){
            if (m_userCache.contains(fromUser)) {
                 
                m_userCache[fromUser].isTyping = false;

                 
                if (fromUser == m_currentChatPartner.username) {
                    m_currentChatPartner =  m_userCache[fromUser];
                    m_chatViewWidget->updateHeader(m_currentChatPartner);
                }
                updateContactItem(fromUser);
            }
        });
    }

     
    m_typingReceiveTimers[fromUser]->start();
}
void MainWindow::onSendMessageReadReceipt(qint64 messageId)
{
    qDebug() << "[CLIENT] Received receipt signal for message ID:" << messageId << ". Sending to server.";
    QJsonObject readCmd;
    readCmd["type"] = "message_read";
    readCmd["id"] = (double)messageId;
    updateMessageStatusInCacheAndModel(messageId, ChatMessage::MessageStatus::Read);

    sendJson(readCmd);
}
MainWindow::~MainWindow()
{
    delete ui;
}

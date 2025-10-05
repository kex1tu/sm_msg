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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    m_nextBlockSize = 0;
    m_replyToMessageId = 0;
    m_editingMessageId = 0;
    m_oldestMessageId = -1;

    connect(socket, &QTcpSocket::connected, this,&MainWindow::onConnected);
    connect(socket, &QTcpSocket::readyRead, this,&MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this,&MainWindow::onDisconnected);



    connect(ui->goToRegisterButton, &QPushButton::clicked, this, &MainWindow::handleGoToRegPageButtonClick);
    connect(ui->goToLoginButton, &QPushButton::clicked, this, &MainWindow::handleGoToLogPageButtonClick);

    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::handleLoginButtonClick);
    connect(ui->registerButton, &QPushButton::clicked, this, &MainWindow::handleRegisterButtonClick);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::handleSendButtonClick);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &MainWindow::handleSendButtonClick);

    connect(ui->userListWidget, &QListWidget::currentItemChanged, this, &MainWindow::handleUserSelectionChanged);
    connect(ui->chatHistoryWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::onChatContextMenuRequested);
    connect(ui->chatHistoryWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onMessageDoubleClicked);
    connect(ui->chatHistoryWidget->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onChatScroll);


    connect(ui->messageLineEdit, &QLineEdit::textChanged, this, &MainWindow::onMessageTextChanged);

    ui->stackedWidget->setCurrentIndex(0);

    connectToServer();
}

void MainWindow::connectToServer(){
    const QString host = "127.0.0.1"; //loaclhost
    const quint16 port = 1234;
    socket->connectToHost(host, port);
    qDebug()<< "[Client] Attempting to connect to host on "<<host <<":" <<port;
}
void MainWindow::onConnected(){
    qDebug() << "[CLIENT] Socket successfully connected!";

    ui->statusbar->showMessage("Connected successfully");

    ui->loginUsernameEdit->setEnabled(true);
    ui->loginPasswordEdit->setEnabled(true);
    ui->loginButton->setEnabled(true);
    ui->goToRegisterButton->setEnabled(true);
}
void MainWindow::onDisconnected(){
    qDebug() << "[CLIENT] Socket disconnected!";

    ui->statusbar->showMessage("disconnected");

    ui->loginUsernameEdit->setEnabled(false);
    ui->loginPasswordEdit->setEnabled(false);
    ui->loginButton->setEnabled(false);
    ui->goToRegisterButton->setEnabled(false);
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
    //[SIZE][JSON]
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

        if (type == "login_success") {
            m_currentUsername = ui->loginUsernameEdit->text().trimmed();
            qDebug() << "[CLIENT] m_currentUsername: " << m_currentUsername;
            //ui->loginUsernameEdit->clear();
            ui->loginPasswordEdit->clear();
            ui->stackedWidget->setCurrentIndex(2);

            this->setWindowTitle(m_currentUsername);

        } else if (type == "login_failure") {
            QMessageBox::warning(this, "error login", response["reason"].toString());

        } else if (type == "register_success") {
            QMessageBox::information(this, "registration succes", "U ve been registred");
            ui->registerPasswordEdit->clear();
            ui->registerUsernameEdit->clear();
            ui->stackedWidget->setCurrentWidget(0);

        } else if (type == "register_failure") {
            QMessageBox::warning(this, "error registration", response["reason"].toString());

        } else if (type == "full_user_list") {
            qDebug() << "full_user_list recieved";
            QJsonArray usersFromServer = response["users"].toArray();
            m_userCache.clear();
            qDebug() << usersFromServer.count();
            for (const QJsonValue &value : usersFromServer) {
                QJsonObject userObj = value.toObject();
                User user;
                user.username = userObj["username"].toString();
                qDebug() << user.username;
                user.displayName = userObj["display_name"].toString();
                qDebug() << user.displayName;

                if (user.username != m_currentUsername) {
                    m_userCache.insert(user.displayName, user);
                }
            }

            updateUserListWidget();
        } else if (type == "history_data") {
            QString historyForUser = response["with_user"].toString();

            if (historyForUser != m_currentChatPartner.username) {
                qDebug() << "[CLIENT] Received outdated history for" << historyForUser
                         << ". Current chat is with" << m_currentChatPartner.username << ". Ignoring.";
                return;
            }
            ui->chatHistoryWidget->clear();
            m_currentChatMessages.clear();
            ui->chatHistoryWidget->addItem("--- Chat with " + historyForUser + " ---");

            QJsonArray history = response["history"].toArray();
            qDebug() << "[CLIENT] Displaying" << history.count() << "history messages for" << historyForUser;

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
                m_currentChatMessages.insert(msg.id, msg);
                displayMessage(msg, -1);
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

            }

        } else if (type == "private_message") {
            QString tempId = response["temp_id"].toString();
            qDebug() << tempId;
            if (!tempId.isEmpty() && m_pendingMessages.contains(tempId)) {

                qDebug() << "[CLIENT] Received ECHO for temp_id:" << tempId;
                QListWidgetItem* item = findItemByTempId(tempId);
                if (!item) {
                    m_pendingMessages.remove(tempId);
                    return;
                }

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

                m_currentChatMessages[msg.id] = msg;

                item->setData(Qt::UserRole, msg.id);

                updateMessageWidget(item, msg);
                m_pendingMessages.remove(tempId);

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
            if(response["is_delivered"].toInt() == 1){
                incomingMsg.status = ChatMessage::Delivered;
            }
            else{
                incomingMsg.status = ChatMessage::Sent;
            }
            if(response["is_read"].toInt() == 1){
                incomingMsg.status = ChatMessage::Read;
            }

            QJsonObject deliveredCmd;
            deliveredCmd["type"] = "message_delivered";
            deliveredCmd["id"] = (double)incomingMsg.id;
            qDebug() << "[CLIENT] message " << (double)incomingMsg.id << "delivered, sending this info to server";
            sendJson(deliveredCmd);

            if (incomingMsg.fromUser == m_currentChatPartner.username) {
                m_currentChatMessages.insert(incomingMsg.id, incomingMsg);
                displayMessage(incomingMsg, -1);

                QJsonObject readCmd;
                readCmd["type"] = "message_read";
                readCmd["id"] = (double)incomingMsg.id;
                qDebug() << "[CLIENT] message " << (double)incomingMsg.id << "read, sending this info to server";
                sendJson(readCmd);
            }

            if (incomingMsg.fromUser != m_currentChatPartner.username || isMinimized() || !isActiveWindow()) {
                QApplication::alert(this);
            }
        } else if (type == "user_list") {
            QJsonArray onlineUsernamesArray = response["users"].toArray();
            QSet<QString> onlineUsers;
            for (const QJsonValue &value : onlineUsernamesArray) {
                onlineUsers.insert(value.toString());
            }

            for (auto it = m_userCache.begin(); it != m_userCache.end(); ++it) {
                if (onlineUsers.contains(it.key())) {
                    it->isOnline = true;
                } else {
                    it->isOnline = false;
                }
            }

            updateUserListWidget();
        } else if (type == "typing") {

        } else if (type == "message_delivered") {
            ChatMessage incomingMsg;
            incomingMsg.id = response["id"].toDouble();

            m_currentChatMessages[incomingMsg.id].status = ChatMessage::Delivered;
            qDebug() << "[CLIENT] message " << (double)incomingMsg.id << "delivered, info from server";
            QListWidgetItem* item = findItemById(incomingMsg.id, std::nullopt);
            if (item) {
                updateMessageWidget(item,m_currentChatMessages[incomingMsg.id]);
            }
        } else if (type == "message_read") {
            ChatMessage incomingMsg;
            incomingMsg.id = response["id"].toDouble();

            m_currentChatMessages[incomingMsg.id].status = ChatMessage::Read;
            qDebug() << "[CLIENT] message " << (double)incomingMsg.id << "read, info from server";
            QListWidgetItem* item = findItemById(incomingMsg.id, std::nullopt);
            if (item) {
                updateMessageWidget(item,m_currentChatMessages[incomingMsg.id]);
            }
        } else if (type == "edit_message"){
            QString editMessageChatUser = response["with_user"].toString();
            qDebug() << "[CLIENT] Received command to edit message from" << editMessageChatUser;

            if(editMessageChatUser == m_currentChatPartner.username){
                editMessageById(response["id"].toDouble(), response["payload"].toString());
            }
        } else if (type == "delete_message"){
            QString deleteMessageChatUser = response["with_user"].toString();
            qDebug() << "[CLIENT] Received command to delete message from" << deleteMessageChatUser;

            if(deleteMessageChatUser == m_currentChatPartner.username){
                removeMessageById(response["id"].toDouble());
            }
        }

    }

}
void MainWindow::handleGoToRegPageButtonClick(){
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::handleGoToLogPageButtonClick(){
    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::handleLoginButtonClick(){
    QString username = ui->loginUsernameEdit->text().trimmed();
    QString password = ui->loginPasswordEdit->text();

    if(username.isEmpty() || password.isEmpty()) return;

    QJsonObject loginRequest;
    loginRequest["type"] = "login";
    loginRequest["username"] = username;
    loginRequest["password"] = password;
    sendJson(loginRequest);
    ui->stackedWidget->setCurrentIndex(2);// перенести в login success
}
void MainWindow::handleRegisterButtonClick(){
    QString username = ui->registerUsernameEdit->text().trimmed();
    QString display_name = ui->registerDisplayNameEdit->text().trimmed();
    QString password = ui->registerPasswordEdit->text();

    if(username.isEmpty() || password.isEmpty()) return;

    QJsonObject registerRequest;
    registerRequest["type"] = "register";
    registerRequest["username"] = username;
    registerRequest["password"] = password;
    registerRequest["display_name"] = display_name;
    sendJson(registerRequest);
}
void MainWindow::handleSendButtonClick(){
    QString payload = ui->messageLineEdit->text().trimmed();
    if (payload.isEmpty()) return;

    if (m_currentChatPartner.username.isEmpty()) return;

    QJsonObject request;
    if (m_editingMessageId > 0){

        request["type"] = "edit_message";
        request["fromUser"] = m_currentUsername;
        request["payload"] = payload;
        request["id"] = m_editingMessageId;


        m_currentChatMessages[m_editingMessageId].isEdited = true;
        sendJson(request);

        ui->messageLineEdit->setFocus();
        m_editingMessageId = 0;
        ui->sendButton->setText("Send");
        ui->messageLineEdit->setPlaceholderText("");
    } else {
        ChatMessage msg;
        msg.fromUser = m_currentUsername;
        msg.toUser = m_currentChatPartner.username;
        msg.replyToId = m_replyToMessageId;
        msg.isEdited = false;
        msg.isOutgoing = true;
        msg.tempId = createTempId();
        msg.payload = payload;
        msg.status = ChatMessage::MessageStatus::Sending;
        msg.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

        request["type"] = "private_message";
        request["fromUser"] = msg.fromUser;
        request["toUser"] = msg.toUser;
        request["payload"] = msg.payload;
        request["reply_to_id"] = msg.replyToId;
        request["temp_id"] = msg.tempId;

        sendJson(request);

        m_pendingMessages.insert(msg.tempId, msg);
        displaySendingMessage(msg);
        m_replyToMessageId = 0;
        ui->messageLineEdit->setPlaceholderText("");
    }
    ui->messageLineEdit->clear();
    ui->messageLineEdit->setFocus();
}
void MainWindow::updateUserListWidget()
{
    QString previouslySelectedUser;
    if (ui->userListWidget->currentItem()) {
        previouslySelectedUser = ui->userListWidget->currentItem()->data(Qt::UserRole).toString();
    }
    ui->userListWidget->clear();

    QList<User> users = m_userCache.values();

    std::sort(users.begin(), users.end(), [](const User& a, const User& b) {
        return a.displayName < b.displayName;
    });

    for (const User &user : m_userCache) {

        QListWidgetItem* item = new QListWidgetItem(user.displayName);

        item->setData(Qt::UserRole, user.displayName);

        QFont font = item->font();
        if (user.isOnline) {
            font.setBold(true);
            item->setForeground(Qt::black);
        } else {
            font.setBold(false);
            item->setForeground(Qt::gray);
        }
        item->setFont(font);

        ui->userListWidget->addItem(item);

        if (user.username == previouslySelectedUser) {
            ui->userListWidget->setCurrentItem(item);
        }
    }
}
QListWidgetItem* MainWindow::findItemById(qint64 messageId, std::optional<std::reference_wrapper<quint64>> posInWidget_optional)
{
    if (messageId <= 0) return nullptr;
    for (int i = 0; i < ui->chatHistoryWidget->count(); ++i) {
        QListWidgetItem* item = ui->chatHistoryWidget->item(i);
        if (item && item->data(Qt::UserRole).toLongLong() == messageId) {
            if(posInWidget_optional){
                posInWidget_optional->get() = i;
            }

            return item;
        }
    }
    return nullptr;
}
QListWidgetItem* MainWindow::findItemByTempId(QString tempId){
    if (tempId.isEmpty()) return nullptr;
    for (int i = 0; i < ui->chatHistoryWidget->count(); ++i) {
        QListWidgetItem* item = ui->chatHistoryWidget->item(i);
        if (item && item->data(Qt::UserRole).toString() == tempId) {
            return item;
        }
    }
    return nullptr;
}
void MainWindow::updateMessageWidget(QListWidgetItem* item, const ChatMessage &msg)
{
    if (!item) return;


    QString statusIcon;
    if (msg.isOutgoing) {
        switch (msg.status) {
        case ChatMessage::Sending:    statusIcon = " 🕒"; break;
        case ChatMessage::Sent:       statusIcon = " ✔"; break;
        case ChatMessage::Delivered:  statusIcon = " ✔✔"; break;
        case ChatMessage::Read:       statusIcon = " ✔✔"; item->setForeground(Qt::blue); break;
        case ChatMessage::Error:      statusIcon = " ❗"; item->setForeground(Qt::red); break;
        default: break;
        }
    }

    QString displayText = QDateTime::fromString(msg.timestamp, Qt::ISODate).toString("hh:mm")
                          + " | " + msg.fromUser + ": " + msg.payload + statusIcon;

    if (msg.isEdited == true) {
        displayText += " (изм.)";
    }

    item->setText(displayText);
    ui->chatHistoryWidget->scrollToBottom();
}
void MainWindow::displayMessage(const ChatMessage &msg, int position)
{
    m_currentChatMessages[msg.id] = msg;

    if (msg.replyToId > 0 && m_currentChatMessages.contains(msg.replyToId)) {
        const ChatMessage& quotedMsg = m_currentChatMessages.value(msg.replyToId);
        QString quotedText = " ↪ " + quotedMsg.fromUser + ": " + quotedMsg.payload.left(50) + "...";

        QListWidgetItem* quoteItem = new QListWidgetItem(quotedText);
        QFont quoteFont = quoteItem->font();
        quoteFont.setPointSize(quoteFont.pointSize() - 2);
        quoteItem->setFont(quoteFont);
        quoteItem->setForeground(Qt::gray);

        if (position == -1) {
            ui->chatHistoryWidget->addItem(quoteItem);
        } else {
            ui->chatHistoryWidget->insertItem(position++, quoteItem);
        }
    }

    QListWidgetItem* mainItem = new QListWidgetItem();

    updateMessageWidget(mainItem, msg);
    mainItem->setData(Qt::UserRole, msg.id);


    if (position == -1) {
        ui->chatHistoryWidget->addItem(mainItem);
    } else {
        ui->chatHistoryWidget->insertItem(position, mainItem);
    }
    ui->chatHistoryWidget->scrollToBottom();
}

void MainWindow::displaySendingMessage(const ChatMessage &msg, int position){
    qDebug() <<"displaySendingMessage";
    m_pendingMessages[msg.tempId] = msg;

    if (msg.replyToId > 0 && m_currentChatMessages.contains(msg.replyToId)) {
        const ChatMessage& quotedMsg = m_currentChatMessages.value(msg.replyToId);
        QString quotedText = " ↪ " + quotedMsg.fromUser + ": " + quotedMsg.payload.left(50) + "...";

        QListWidgetItem* quoteItem = new QListWidgetItem(quotedText);
        QFont quoteFont = quoteItem->font();
        quoteFont.setPointSize(quoteFont.pointSize() - 2);
        quoteItem->setFont(quoteFont);
        quoteItem->setForeground(Qt::gray);

        if (position == -1) {
            ui->chatHistoryWidget->addItem(quoteItem);
        } else {
            ui->chatHistoryWidget->insertItem(position++, quoteItem);
        }
    }
    QListWidgetItem* mainItem = new QListWidgetItem();

    updateMessageWidget(mainItem, msg);
    mainItem->setData(Qt::UserRole, msg.tempId);


    if (position == -1) {
        ui->chatHistoryWidget->addItem(mainItem);
    } else {
        ui->chatHistoryWidget->insertItem(position, mainItem);
    }
}

void MainWindow::handleUserSelectionChanged(QListWidgetItem *current){

    if (!current) {
        ui->chatHistoryWidget->clear();
        m_currentChatMessages.clear();
        m_currentChatPartner.username.clear();

        m_replyToMessageId = 0;
        m_editingMessageId = 0;
        m_oldestMessageId = -1;


        ui->sendButton->setText("Отправить");
        ui->messageLineEdit->setPlaceholderText("");
        return;
    }


    QString selectedUser = current->text();

    qDebug() << "Selected user:" << selectedUser << ". Requesting latest history...";

    ui->chatHistoryWidget->clear();
    m_currentChatMessages.clear();
    m_oldestMessageId = -1;
    m_editingMessageId = 0;
    m_replyToMessageId = 0;
    m_currentChatPartner = m_userCache.value(selectedUser);
    ui->sendButton->setText("Отправить");
    ui->messageLineEdit->setPlaceholderText("");
    ui->chatHistoryWidget->addItem("--- Chat with " + selectedUser + " ---");


    QJsonObject request;
    request["type"] = "get_history";
    request["with_user"] = m_currentChatPartner.username;
    sendJson(request);
}
QString MainWindow::createTempId(){// переписать ибо такие tempId могут совпадать
    QString tempId = QDateTime::currentDateTime().toString(Qt::ISODate) + "_tempId";
    qDebug() << "[CLIENT] Created temporary id for outcoming message, tempId: " << tempId;
    return tempId;
};
void MainWindow::onChatContextMenuRequested(const QPoint &pos){
    QListWidgetItem *item = ui->chatHistoryWidget->itemAt(pos);
    if(!item){
        return;
    }
    qint64 messageId = item->data(Qt::UserRole).toLongLong();
    const ChatMessage &msg = m_currentChatMessages.value(messageId);

    QMenu contextMenu(this);
    QAction *replyAction = contextMenu.addAction("Reply");
    QAction *editAction = contextMenu.addAction("Edit");
    QAction *deleteAction = contextMenu.addAction("Delete");
    if (msg.isOutgoing && msg.fromUser == m_currentUsername) {
        editAction->setEnabled(true);
        deleteAction->setEnabled(true);
    } else {
        editAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }
    QAction *selectedAction = contextMenu.exec(ui->chatHistoryWidget->mapToGlobal(pos));

    if (selectedAction == replyAction) {
        onMessageDoubleClicked(item);
    }
    else if (selectedAction == editAction) {
        m_editingMessageId = messageId;
        ui->messageLineEdit->setText(msg.payload);
        ui->sendButton->setText("Сохранить");
        ui->messageLineEdit->setFocus();
        qDebug() << "[CLIENT] User wants to EDIT message with ID:" << messageId;
    }
    else if (selectedAction == deleteAction) {
        QJsonObject request;
        request["type"] = "delete_message";
        request["id"] = messageId;
        sendJson(request);
        qDebug() << "[CLIENT] User wants to DELETE message with ID:" << messageId;
    }
};
void MainWindow::onMessageDoubleClicked(QListWidgetItem *item){
    m_replyToMessageId = item->data(Qt::UserRole).toLongLong();
    if (m_replyToMessageId > 0) {
        ui->messageLineEdit->setPlaceholderText("Ответ на: " + item->text().left(20) + "...");
        ui->messageLineEdit->setFocus();
    }
};
void MainWindow::removeMessageById(qint64 messageId){

    quint64 posInWidget;
    QListWidgetItem *deletingMessageElement = findItemById(messageId, posInWidget);
    if(posInWidget > 0){// чуть переписать условие на то что указатель не nullptr
        QListWidgetItem* previousItem = ui->chatHistoryWidget->item(posInWidget - 1);
        if(previousItem->data(Qt::UserRole).isNull()){
            delete ui->chatHistoryWidget->takeItem(posInWidget - 1);
            --posInWidget;
        }
    }
    delete deletingMessageElement;

    qDebug() << "[CLIENT] Successfully removed message with ID:" << messageId;
    m_currentChatMessages.remove(messageId);
    return;

}
void MainWindow::editMessageById(qint64 messageId, const QString newPayload){
    if (m_currentChatMessages.contains(messageId)) {
        m_currentChatMessages[messageId].payload = newPayload;
        m_currentChatMessages[messageId].isEdited = true;
    }  else {
        qDebug() << "[CLIENT] Cannot edit, message" << messageId << "not in cache.";
        return;
    }

    QListWidgetItem* editingMessageElement = findItemById(messageId, std::nullopt);
    if(editingMessageElement!=nullptr){
        updateMessageWidget(editingMessageElement, m_currentChatMessages[messageId]);
    }


    qDebug() << "[CLIENT] Successfully edited message with ID:" << messageId;
    return;
}
void MainWindow::onChatScroll(int value){};

void MainWindow::onMessageTextChanged(const QString &text){};
MainWindow::~MainWindow()
{
    delete ui;
}

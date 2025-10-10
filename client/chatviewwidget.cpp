#include "chatviewwidget.h"
#include "ui_chatviewwidget.h"


#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QSpacerItem>
#include <QDateTime>
#include <QListView>
#include <QMenu>
#include <QAction>

ChatViewWidget::ChatViewWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatViewWidget)
{
    ui->setupUi(this);
    setupHeaderUI();

    connect(ui->sendButton, &QPushButton::clicked, this, [this](){
        QString text = ui->messageLineEdit->text().trimmed();
        if (!text.isEmpty()) {
            emit sendMessageRequested(text);
            ui->messageLineEdit->clear();
        }
    });
    ui->chatHistoryView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->chatHistoryView, &QWidget::customContextMenuRequested, this, &ChatViewWidget::onChatContextMenuRequested);
    connect(ui->chatHistoryView, &QListView::doubleClicked, this, &ChatViewWidget::onMessageDoubleClicked);
}

void ChatViewWidget::setupHeaderUI()
{
    m_nameLabel = new QLabel("Имя собеседника");
    m_nameLabel->setObjectName("chatPartnerNameLabel");

    m_statusLabel = new QLabel("статус");
    m_statusLabel->setObjectName("chatPartnerStatusLabel");

    m_searchButton = new QToolButton();
    m_searchButton->setObjectName("searchInChatButton");
    m_searchButton->setText("🔍");

    m_callButton = new QToolButton();
    m_callButton->setObjectName("callButton");
    m_callButton->setText("📞");

    m_videoCallButton = new QToolButton();
    m_videoCallButton->setObjectName("videoCallButton");
    m_videoCallButton->setText("📹");

    m_moreOptionsButton = new QToolButton();
    m_moreOptionsButton->setObjectName("moreOptionsButton");
    m_moreOptionsButton->setText("...");

    QVBoxLayout* userInfoLayout = new QVBoxLayout();
    userInfoLayout->addWidget(m_nameLabel);
    userInfoLayout->addWidget(m_statusLabel);
    userInfoLayout->setSpacing(0);
    userInfoLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout* headerLayout = qobject_cast<QHBoxLayout*>(ui->headerWidget->layout());
    if (!headerLayout) return;

    headerLayout->addLayout(userInfoLayout);
    headerLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    headerLayout->addWidget(m_searchButton);
    headerLayout->addWidget(m_callButton);
    headerLayout->addWidget(m_videoCallButton);
    headerLayout->addWidget(m_moreOptionsButton);
}
void ChatViewWidget::onMessageDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();

    ui->messageLineEdit->setPlaceholderText("Ответ на: " + msg.payload.left(30) + "...");
    ui->messageLineEdit->setFocus();

    emit replyToMessageRequested(msg.id);
}
ChatViewWidget::~ChatViewWidget()
{
    delete ui;
}
void ChatViewWidget::onChatContextMenuRequested(const QPoint &pos){
    QModelIndex index = ui->chatHistoryView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
    QMenu contextMenu(this);
    QAction *replyAction = contextMenu.addAction("reply");
    QAction *editAction = contextMenu.addAction("edit");
    QAction *deleteAction = contextMenu.addAction("delete");





    if (msg.isOutgoing) {
        editAction->setEnabled(true);
        deleteAction->setEnabled(true);
    } else {
        editAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }

    QAction *selectedAction = contextMenu.exec(ui->chatHistoryView->viewport()->mapToGlobal(pos));

    if (selectedAction == replyAction) {
        emit replyToMessageRequested(msg.id);
    } else if (selectedAction == editAction) {
        emit editMessageRequested(msg.id, msg.payload);
    } else if (selectedAction == deleteAction) {
        emit deleteMessageRequested(msg.id);
    }
}


QListView* ChatViewWidget::chatHistoryView() const { return ui->chatHistoryView; }
QLineEdit* ChatViewWidget::messageLineEdit() const { return ui->messageLineEdit; }

void ChatViewWidget::updateHeader(const User& chatPartner, bool isTyping)
{
    m_nameLabel->setText(chatPartner.displayName);

    if (isTyping) {
        m_statusLabel->setText("печатает...");
        m_statusLabel->setStyleSheet("color: #F4ABC4;");
    } else {
        QString statusText = formatLastSeen(chatPartner);
        m_statusLabel->setText(statusText);
        if (chatPartner.isOnline) {
            m_statusLabel->setStyleSheet("color: #4CAF50;");
        } else {
            m_statusLabel->setStyleSheet("color: #a0a0a0;");
        }
    }
}

QString pluralize(int n, const QString& form1, const QString& form2, const QString& form5) {
    n = abs(n) % 100;
    int n1 = n % 10;
    if (n > 10 && n < 20) return form5;
    if (n1 > 1 && n1 < 5) return form2;
    if (n1 == 1) return form1;
    return form5;
}

QString ChatViewWidget::formatLastSeen(const User &user)
{
    if (user.isOnline) {
        return "в сети";
    }

    if (user.lastSeen.isEmpty()) {
        return "не в сети";
    }

    QDateTime lastSeenTime = QDateTime::fromString(user.lastSeen, Qt::ISODate);
    if (!lastSeenTime.isValid()) {
        return "не в сети";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 diffSeconds = lastSeenTime.secsTo(now);

    if (diffSeconds < 60) {
        return "был(а) только что";
    }
    else if (diffSeconds < 3600) {
        int minutes = diffSeconds / 60;
        return QString("был(а) %1 %2 назад")
            .arg(minutes)
            .arg(pluralize(minutes, "минуту", "минуты", "минут"));
    }
    else if (lastSeenTime.date() == now.date()) {
        return "был(а) сегодня в " + lastSeenTime.toString("HH:mm");
    }
    else if (lastSeenTime.date() == now.date().addDays(-1)) {
        return "был(а) вчера в " + lastSeenTime.toString("HH:mm");
    }
    else {
        return "был(а) " + QLocale::system().toString(lastSeenTime, QLocale::ShortFormat);
    }
}



void ChatViewWidget::onSearchTriggered(const QString& text)
{
    qDebug() << "Search in chat triggered:" << text;
}

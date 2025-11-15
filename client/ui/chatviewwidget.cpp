#include "chatviewwidget.h"
#include "chatmessagedelegate.h"
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
#include <QScrollBar>
#include <QResizeEvent>
#include <QTextEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QMouseEvent>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <algorithm>


ChatViewWidget::ChatViewWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatViewWidget)
{
    ui->setupUi(this);

    // Инициализация header-части (аватар, имя, кнопки)
    setupHeaderUI();

    // Анимация раскрытия replyWidget
    m_replyAnimation = new QPropertyAnimation(ui->replyWidget, "maximumHeight", this);
    m_replyAnimation->setDuration(200);   // 200мс — плавно
    m_replyAnimation->setEasingCurve(QEasingCurve::OutCubic);
    ui->replyWidget->hide();

    // Обновление состояния replyWidget, debug и кнопка прокрутки вниз
    connect(m_replyAnimation, &QPropertyAnimation::finished, this, [this]() {
        qDebug() << "Reply widget visibility:" << ui->replyWidget->isVisible();
        updateScrollToBottomButton();
    });

    // Адаптивная высота поля ввода (не больше 200px)
    ui->messageTextEdit->setFixedHeight(std::max(ui->sendButton->height(), 40));
    ui->messageTextEdit->installEventFilter(this);
    connect(ui->messageTextEdit, &QTextEdit::textChanged, this, [this](){
        int contentHeight = ui->messageTextEdit->document()->size().height();
        int minH = ui->sendButton->height();
        int maxH = 200;
        int newHeight = std::min(std::max(contentHeight, minH), maxH);
        ui->messageTextEdit->setFixedHeight(newHeight);
    });

    // Кнопка "прокрутить вниз" (прячется если не нужна)
    m_scrollToBottomButton = new QToolButton(this);
    m_scrollToBottomButton->setObjectName("scrollToBottomButton");
    m_scrollToBottomButton->setIcon(QIcon(":/icons/icons/down_arrow.png"));
    m_scrollToBottomButton->setIconSize(QSize(24, 24));
    m_scrollToBottomButton->setFixedSize(40, 40);
    m_scrollToBottomButton->hide();

    // Лейбл количества непрочитанных сообщений (UI элемент)
    m_unreadCountLabel = new QLabel(this);
    m_unreadCountLabel->setObjectName("unreadCountLabel");
    m_unreadCountLabel->setAlignment(Qt::AlignCenter);
    m_unreadCountLabel->setFixedSize(22, 22);
    m_unreadCountLabel->hide();

    // Кнопка закрытия блока reply
    m_closeReplyButton = ui->closeReplyButton;
    connect(m_closeReplyButton, &QToolButton::clicked, this, &ChatViewWidget::hideReplyUI);

    // Отправка сообщений кнопкой (trim, debug)
    connect(ui->sendButton, &QPushButton::clicked, this, [this](){
        QString text = ui->messageTextEdit->toPlainText().trimmed();
        if (!text.isEmpty()) {
            emit sendMessageRequested(text);
            ui->messageTextEdit->clear();
        }
    });

    // Контекстное меню истории, даблклик по истории, прокрутка вниз, отслеживание скролла
    ui->chatHistoryView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->chatHistoryView, &QWidget::customContextMenuRequested, this, &ChatViewWidget::onChatContextMenuRequested);
    connect(ui->chatHistoryView, &QListView::doubleClicked, this, &ChatViewWidget::onMessageDoubleClicked);
    connect(m_scrollToBottomButton, &QToolButton::clicked, this, &ChatViewWidget::onScrollDownButtonClicked);
    connect(ui->chatHistoryView->verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatViewWidget::onChatScrolled);

    connect(m_searchButton, &QToolButton::clicked, this, &ChatViewWidget::showSearchUI);

    // Плавная анимация прокрутки (400мс)
    m_scrollAnimation = new QPropertyAnimation(this);
    m_scrollAnimation->setTargetObject(ui->chatHistoryView->verticalScrollBar());
    m_scrollAnimation->setPropertyName("value");
    m_scrollAnimation->setDuration(400);
    m_scrollAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    qDebug() << "[ChatViewWidget] constructed and UI initialized";
}


ChatViewWidget::~ChatViewWidget()
{
    delete ui;
    qDebug() << "[ChatViewWidget] destroyed";
}


void ChatViewWidget::onScrollDownButtonClicked()
{
    if (m_unreadMessageCount > 0) {
        qDebug() << "[ChatViewWidget] Скроллим к непрочитанным сообщениям: " << m_unreadMessageCount;
        emit scrollToUnreadRequested();
    } else {
        qDebug() << "[ChatViewWidget] Скроллим в конец истории чата";
        emit scrollToBottomRequested();
    }
}



bool ChatViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    // Перехват Enter для чата: Shift+Enter — перевод строки, просто Enter — отправка сообщения
    if (watched == ui->messageTextEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                ui->sendButton->click();
                return true;
            }
        }
    }

    // Клик по user info в заголовке чата — эмитим headerClicked()
    if (watched == m_userInfoWidget && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            qDebug() << "User info widget clicked, emitting headerClicked()";
            emit headerClicked();
            return true;
        }
    }

    // Всё остальное — стандартная обработка
    return QWidget::eventFilter(watched, event);
}


void ChatViewWidget::showReplyUI(const QString& name, const QString& text)
{
    ui->replyNameLabel->setText("В ответ " + name);
    QFontMetrics fm(ui->replyTextLabel->font());
    QString elidedText = fm.elidedText(text, Qt::ElideRight, ui->replyTextLabel->width());
    ui->replyTextLabel->setText(elidedText);
    ui->replyWidget->show();

    m_replyAnimation->setStartValue(0);
    m_replyAnimation->setEndValue(50);
    m_replyAnimation->setDirection(QAbstractAnimation::Forward);
    m_replyAnimation->start();

    ui->messageTextEdit->setFocus();
    qDebug() << "[ChatViewWidget] showReplyUI for" << name;
}


void ChatViewWidget::hideReplyUI()
{
    clearReplyUI();
    emit replyCancelled();
    ui->replyWidget->hide();
    QTimer::singleShot(50, this, &ChatViewWidget::updateScrollToBottomButton);
}


void ChatViewWidget::setupHeaderUI()
{
    // Инициализация header-стека и подвиджетов
    m_headerStack = new QStackedWidget();
    m_normalHeaderWidget = new QWidget();
    m_searchHeaderWidget = new QWidget();

    // Отображение информации о контакте — имя, статус, аватар
    m_userInfoWidget = new QWidget();
    QVBoxLayout* userInfoLayout = new QVBoxLayout(m_userInfoWidget);
    userInfoLayout->setSpacing(0);
    userInfoLayout->setContentsMargins(8, 0, 0, 0); // Левый внешний отступ

    QHBoxLayout* normalHeaderLayout = new QHBoxLayout();
    m_normalHeaderWidget->setLayout(normalHeaderLayout);
    normalHeaderLayout->setContentsMargins(0, 5, 5, 5);

    m_nameLabel = new QLabel("Имя собеседника");
    m_nameLabel->setObjectName("chatPartnerNameLabel");

    m_statusLabel = new QLabel("статус");
    m_statusLabel->setObjectName("chatPartnerStatusLabel");

    userInfoLayout->addWidget(m_nameLabel);
    userInfoLayout->addWidget(m_statusLabel);
    userInfoLayout->setSpacing(0);
    userInfoLayout->setContentsMargins(0,0,0,0);

    // Реагировать на клики — аватар и имя становятся кликабельными
    m_userInfoWidget->installEventFilter(this);
    m_userInfoWidget->setCursor(Qt::PointingHandCursor);

    // Кнопки действий: поиск, аудио/видео вызов, опции
    m_searchButton = new QToolButton();
    m_searchButton->setObjectName("searchInChatButton");
    m_searchButton->setIcon(QIcon(":/icons/icons/search.png"));

    m_callButton = new QToolButton();
    m_callButton->setObjectName("callButton");
    m_callButton->setIcon(QIcon(":/icons/icons/audioCall.png"));

    m_videoCallButton = new QToolButton();
    m_videoCallButton->setObjectName("videoCallButton");
    m_videoCallButton->setIcon(QIcon(":/icons/icons/videoCall.png"));

    m_moreOptionsButton = new QToolButton();
    m_moreOptionsButton->setObjectName("moreOptionsButton");
    m_moreOptionsButton->setIcon(QIcon(":/icons/icons/dotsVertical.png"));

    // Компоновка: инфо+кнопки
    normalHeaderLayout->addWidget(m_userInfoWidget);
    normalHeaderLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    normalHeaderLayout->addWidget(m_searchButton);
    normalHeaderLayout->addWidget(m_callButton);
    normalHeaderLayout->addWidget(m_videoCallButton);
    normalHeaderLayout->addWidget(m_moreOptionsButton);

    // Search header: строка поиска, закрытие
    QHBoxLayout* searchHeaderLayout = new QHBoxLayout(m_searchHeaderWidget);
    searchHeaderLayout->setContentsMargins(5, 5, 5, 5);
    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Поиск в чате...");
    m_closeSearchButton = new QToolButton();
    m_closeSearchButton->setIcon(QIcon(":/icons/icons/cross.png"));
    searchHeaderLayout->addWidget(m_searchLineEdit);
    searchHeaderLayout->addWidget(m_closeSearchButton);

    // Стек заголовков (обычный + поисковый) — для переключения между режимами
    m_headerStack->addWidget(m_normalHeaderWidget);
    m_headerStack->addWidget(m_searchHeaderWidget);

    // Правильно подключаем к headerWidget — если layout уже есть, добавляем; иначе создаём новый
    if (ui->headerWidget->layout()) {
        ui->headerWidget->layout()->addWidget(m_headerStack);
    } else {
        QHBoxLayout* mainHeaderLayout = new QHBoxLayout(ui->headerWidget);
        mainHeaderLayout->setContentsMargins(0,0,0,0);
        mainHeaderLayout->addWidget(m_headerStack);
    }

    // Сигналы/слоты для поиска и кнопок
    connect(m_closeSearchButton, &QToolButton::clicked, this, &ChatViewWidget::hideSearchUI);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &ChatViewWidget::searchTextEntered);
    connect(m_searchButton, &QToolButton::clicked, this, &ChatViewWidget::showSearchUI);
    connect(m_callButton, &QToolButton::clicked, this, &ChatViewWidget::onCallButtonClicked);

    qDebug() << "[ChatViewWidget] Header UI инициализирован";
}


void ChatViewWidget::showSearchUI() {
    m_headerStack->setCurrentWidget(m_searchHeaderWidget);
    m_searchLineEdit->setFocus();
    qDebug() << "[ChatViewWidget] showSearchUI triggered";
}


void ChatViewWidget::onCallButtonClicked() {
    qDebug() << "Call button clicked";
    emit callRequested();
}


void ChatViewWidget::hideSearchUI() {
    m_searchLineEdit->clear();
    m_headerStack->setCurrentWidget(m_normalHeaderWidget);
    qDebug() << "[ChatViewWidget] hideSearchUI, header restored";
}


void ChatViewWidget::clearReplyUI()
{
    ui->messageTextEdit->setPlaceholderText("Напишите сообщение...");
}


void ChatViewWidget::onMessageDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
    showReplyUI(msg.fromUser, msg.payload);
    emit replyToMessageRequested(msg.id);
}


void ChatViewWidget::onChatContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->chatHistoryView->indexAt(pos);
    if (!index.isValid()) return;
    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
    QMenu contextMenu(this);
    QAction *copyAction = contextMenu.addAction("Копировать текст");
    QAction *replyAction = contextMenu.addAction("Ответить");
    QAction *editAction = contextMenu.addAction("Редактировать");
    QAction *deleteAction = contextMenu.addAction("Удалить");

    // Разрешить/запретить edit/delete для входящих
    if (!msg.isOutgoing) {
        editAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }
    // Отключить "Копировать" для пустых сообщений
    if (msg.payload.isEmpty()) {
        copyAction->setEnabled(false);
    }
    QAction *selectedAction = contextMenu.exec(ui->chatHistoryView->viewport()->mapToGlobal(pos));
    if (selectedAction == copyAction) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(msg.payload);
    } else if (selectedAction == replyAction) {
        onMessageDoubleClicked(index);
    } else if (selectedAction == editAction) {
        emit editMessageRequested(msg.id, msg.payload);
    } else if (selectedAction == deleteAction) {
        emit deleteMessageRequested(msg.id);
    }
}


QListView* ChatViewWidget::chatHistoryView() const { return ui->chatHistoryView; }


QTextEdit* ChatViewWidget::messageTextEdit() const { return ui->messageTextEdit; }


void ChatViewWidget::updateHeader(const User& chatPartner)
{
    qDebug() << "Header update" << chatPartner.username << " : " << chatPartner.isOnline;
    m_nameLabel->setText(chatPartner.displayName);

    if (chatPartner.isTyping) {
        m_statusLabel->setText("печатает...");
        m_statusLabel->setStyleSheet("color: #F4ABC4;");
    } else {
        QString statusText = formatLastSeen(chatPartner);
        m_statusLabel->setText(statusText);
        m_statusLabel->setStyleSheet(chatPartner.isOnline ? "color: #4CAF50;" : "color: #a0a0a0;");
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


QString formatLastSeen(const User &user)
{
    if (user.isOnline) return "в сети";
    if (user.lastSeen.isEmpty()) return "не в сети";
    QDateTime lastSeenTime = QDateTime::fromString(user.lastSeen, Qt::ISODate);
    if (!lastSeenTime.isValid()) return "не в сети";
    QDateTime now = QDateTime::currentDateTime();
    qint64 diffSeconds = lastSeenTime.secsTo(now);

    if (diffSeconds < 60) {
        return "был(а) только что";
    } else if (diffSeconds < 3600) {
        int minutes = diffSeconds / 60;
        return QString("был(а) %1 %2 назад").arg(minutes).arg(pluralize(minutes, "минуту", "минуты", "минут"));
    } else if (lastSeenTime.date() == now.date()) {
        return "был(а) сегодня в " + lastSeenTime.toString("HH:mm");
    } else if (lastSeenTime.date() == now.date().addDays(-1)) {
        return "был(а) вчера в " + lastSeenTime.toString("HH:mm");
    } else {
        return "был(а) " + QLocale::system().toString(lastSeenTime, QLocale::ShortFormat);
    }
}


void ChatViewWidget::onNewMessageReceived()
{
    if (!isScrolledToBottom()) {
        m_unreadMessageCount++;
        updateScrollToBottomButton();
    }
}


void ChatViewWidget::scrollToBottom()
{
    emit ui->chatHistoryView->model()->dataChanged(QModelIndex(), QModelIndex());
    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(ui->chatHistoryView->verticalScrollBar()->value());
    m_scrollAnimation->setEndValue(ui->chatHistoryView->verticalScrollBar()->maximum());
    m_scrollAnimation->start();

    m_unreadMessageCount = 0;
    updateScrollToBottomButton();
}


void ChatViewWidget::onChatScrolled(int value)
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    if (value == scrollBar->maximum()) {
        m_unreadMessageCount = 0;
        updateScrollToBottomButton();
    } else {
        updateScrollToBottomButton();
    }
}


void ChatViewWidget::scrollToMessage(const QModelIndex& index)
{
    if (!index.isValid()) {
        scrollToBottom();
        return;
    }

    QRect messageRect = ui->chatHistoryView->visualRect(index);
    int targetValue = ui->chatHistoryView->verticalScrollBar()->value() + messageRect.top();

    m_scrollAnimation->stop();
    m_scrollAnimation->setStartValue(ui->chatHistoryView->verticalScrollBar()->value());
    m_scrollAnimation->setEndValue(targetValue);
    m_scrollAnimation->start();

    m_unreadMessageCount = 0;
    updateScrollToBottomButton();
}


bool ChatViewWidget::isScrolledToBottom() const
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    return scrollBar->value() >= scrollBar->maximum() - 5;
}


void ChatViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Если изменилась ширина — сбрасываем кэш делегата
    if (event->oldSize().width() != event->size().width()) {
        auto delegate = qobject_cast<ChatMessageDelegate*>(ui->chatHistoryView->itemDelegate());
        if (delegate) {
            delegate->clearSizeHintCache();
            delegate->clearCaches();
        }
        ui->chatHistoryView->doItemsLayout();
    }
    updateScrollToBottomButton();
}


void ChatViewWidget::updateScrollToBottomButton()
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    const int scrollThreshold = 1000;
    bool showButton = (m_unreadMessageCount > 0) || (scrollBar->maximum() - scrollBar->value() > scrollThreshold);

    if (showButton) {
        int margin = 15;
        QPoint buttonPos(width() - m_scrollToBottomButton->width() - margin, height() - m_scrollToBottomButton->height() - ui->messageInputWidget->height() - margin);
        m_scrollToBottomButton->move(buttonPos);

        if (m_unreadMessageCount > 0) {
            m_unreadCountLabel->setText(QString::number(m_unreadMessageCount));
            QPoint labelPos(buttonPos.x() + (m_scrollToBottomButton->width() / 2), buttonPos.y() - m_unreadCountLabel->height() / 2);
            m_unreadCountLabel->move(labelPos);
            m_unreadCountLabel->show();
        } else {
            m_unreadCountLabel->hide();
        }
        m_scrollToBottomButton->show();
    } else {
        m_scrollToBottomButton->hide();
        m_unreadCountLabel->hide();
    }
}


void ChatViewWidget::onSearchTriggered(const QString& text)
{
    qDebug() << "Search in chat triggered:" << text;
}


void ChatViewWidget::setEditMode(bool enabled, const QString& text)
{
    if (enabled) {
        ui->sendButton->setText("Сохранить");
        ui->messageTextEdit->setText(text);
        ui->messageTextEdit->setFocus();
    } else {
        ui->sendButton->setText("Отправить");
        ui->messageTextEdit->clear();
        ui->messageTextEdit->setPlaceholderText("Напишите сообщение...");
    }
}

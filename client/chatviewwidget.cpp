/**
 * @file chatviewwidget.cpp
 * @brief Реализация виджета, отображающего интерфейс одного чата.
 * @details Этот класс инкапсулирует всю визуальную часть и логику взаимодействия
 *          пользователя с одним конкретным чатом: заголовком, историей сообщений
 *          и панелью ввода.
 * @see ChatViewWidget
 * @author kex1tu
 */

#include "chatviewwidget.h"
#include "ui_chatviewwidget.h"

// Включения всех необходимых для работы виджета классов Qt
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
#include <algorithm> // Для std::min и std::max

/**
 * @brief Конструктор ChatViewWidget.
 * @details Инициализирует UI из формы, созданной в Qt Designer.
 *          Настраивает дополнительные UI-компоненты (заголовок, анимации, поле ввода).
 *          Устанавливает фильтры событий и соединяет все необходимые сигналы и слоты.
 * @param parent Родительский виджет.
 */
ChatViewWidget::ChatViewWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatViewWidget)
{
    // Инициализация UI из сгенерированного uic файла (chatviewwidget.ui).
    ui->setupUi(this);

    // 1. Программное создание и настройка заголовка чата.
    setupHeaderUI();

    // 2. Настройка анимации для плавного появления/скрытия панели ответа.
    m_replyAnimation = new QPropertyAnimation(ui->replyWidget, "maximumHeight", this);
    m_replyAnimation->setDuration(200); // Длительность анимации 200 мс.
    m_replyAnimation->setEasingCurve(QEasingCurve::OutCubic); // Плавное замедление в конце.
    ui->replyWidget->hide(); // Изначально панель ответа скрыта.

    // (Для отладки) Соединение сигнала завершения анимации с лямбда-функцией.
    connect(m_replyAnimation, &QPropertyAnimation::finished, this, [this]() {
        qDebug() << "Reply widget visibility:" << ui->replyWidget->isVisible();
    });

    // 3. Настройка многострочного поля ввода (QTextEdit).
    ui->messageTextEdit->setFixedHeight(ui->sendButton->height()); // Начальная высота равна высоте кнопки.
    ui->messageTextEdit->installEventFilter(this); // Установка фильтра событий для перехвата нажатия Enter.

    // Соединение сигнала изменения текста с логикой автоматического изменения высоты поля ввода.
    connect(ui->messageTextEdit, &QTextEdit::textChanged, this, [this](){
        // Получаем высоту содержимого документа внутри QTextEdit.
        int contentHeight = ui->messageTextEdit->document()->size().height();

        // Определяем границы высоты.
        int minH = ui->sendButton->height(); // Минимальная высота - как у кнопки.
        int maxH = 150; // Максимальная высота, чтобы поле не растягивалось до бесконечности.

        // Ограничиваем высоту в заданных пределах.
        int newHeight = std::min(std::max(contentHeight, minH), maxH);

        // Устанавливаем новую высоту для поля ввода.
        ui->messageTextEdit->setFixedHeight(newHeight);
    });

    // 4. Инициализация UI для прокрутки вниз и счетчика непрочитанных.
    m_scrollToBottomButton = new QToolButton(this);
    m_scrollToBottomButton->setObjectName("scrollToBottomButton");
    m_scrollToBottomButton->setIcon(QIcon(":/icons/down_arrow.png"));
    m_scrollToBottomButton->setIconSize(QSize(24, 24));
    m_scrollToBottomButton->setFixedSize(40, 40);
    m_scrollToBottomButton->hide(); // Изначально скрыт.

    m_unreadCountLabel = new QLabel(this);
    m_unreadCountLabel->setObjectName("unreadCountLabel");
    m_unreadCountLabel->setAlignment(Qt::AlignCenter);
    m_unreadCountLabel->setFixedSize(22, 22);
    m_unreadCountLabel->hide(); // Изначально скрыт.

    // 5. Соединение всех сигналов и слотов виджета.
    connect(ui->closeReplyButton, &QToolButton::clicked, this, &ChatViewWidget::hideReplyUI);
    connect(ui->sendButton, &QPushButton::clicked, this, [this](){
        QString text = ui->messageTextEdit->toPlainText().trimmed(); // Получаем текст и убираем пробелы.
        if (!text.isEmpty()) {
            emit sendMessageRequested(text); // Отправляем сигнал наружу (в MainWindow).
            ui->messageTextEdit->clear();     // Очищаем поле ввода.
        }
    });

    // Настройка политики вызова контекстного меню.
    ui->chatHistoryView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->chatHistoryView, &QWidget::customContextMenuRequested, this, &ChatViewWidget::onChatContextMenuRequested);
    connect(ui->chatHistoryView, &QListView::doubleClicked, this, &ChatViewWidget::onMessageDoubleClicked);
    connect(m_scrollToBottomButton, &QToolButton::clicked, this, &ChatViewWidget::scrollToBottom);
    connect(ui->chatHistoryView->verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatViewWidget::onChatScrolled);
}

/**
 * @brief Деструктор.
 */
ChatViewWidget::~ChatViewWidget()
{
    delete ui;
}


bool ChatViewWidget::eventFilter(QObject *watched, QEvent *event)
{
    // Проверяем, что событие пришло от нашего поля ввода и что это нажатие клавиши.
    if (watched == ui->messageTextEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Если нажат Enter или Return...
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // ...и при этом НЕ зажата клавиша Shift...
            if (!(keyEvent->modifiers() & Qt::ShiftModifier)) {
                ui->sendButton->click(); // ...симулируем клик по кнопке "Отправить".
                return true;  // Поглощаем событие, чтобы стандартный обработчик QTextEdit не вставил символ новой строки.
            }
            // Если Shift зажат, `return true` не сработает, и QTextEdit обработает событие, вставив перенос строки.
        }
    }

    // Для всех остальных событий вызываем обработчик базового класса.
    return QWidget::eventFilter(watched, event);
}



/**
 * @brief Показывает панель ответа с плавной анимацией "разворачивания".
 * @param name Имя автора цитируемого сообщения.
 * @param text Текст цитируемого сообщения.
 */
void ChatViewWidget::showReplyUI(const QString& name, const QString& text)
{
    // Заполняем метки данными цитаты.
    ui->replyNameLabel->setText("В ответ " + name);
    QFontMetrics fm(ui->replyTextLabel->font());
    QString elidedText = fm.elidedText(text, Qt::ElideRight, ui->replyTextLabel->width()); // Обрезаем длинный текст.
    ui->replyTextLabel->setText(elidedText);

    ui->replyWidget->show(); // Делаем виджет видимым.

    // Настраиваем и запускаем анимацию.
    m_replyAnimation->setStartValue(0);      // Начальная высота.
    m_replyAnimation->setEndValue(50);       // Целевая высота.
    m_replyAnimation->setDirection(QAbstractAnimation::Forward);
    m_replyAnimation->start();

    ui->messageTextEdit->setFocus(); // Устанавливаем фокус на поле ввода.
}

void ChatViewWidget::hideReplyUI()
{
    clearReplyUI(); // Сбрасываем placeholder в поле ввода.
    emit replyCancelled(); // Уведомляем MainWindow, что режим ответа отменен.
    ui->replyWidget->hide(); // Пока что скрываем мгновенно.
}

void ChatViewWidget::setupHeaderUI()
{
    // Создаем все необходимые UI-элементы.
    m_nameLabel = new QLabel("Имя собеседника");
    m_nameLabel->setObjectName("chatPartnerNameLabel");
    m_statusLabel = new QLabel("статус");
    m_statusLabel->setObjectName("chatPartnerStatusLabel");
    m_searchButton = new QToolButton();
    m_searchButton->setObjectName("searchInChatButton");
    m_searchButton->setIcon(QIcon(":/icons/search.png"));
    m_callButton = new QToolButton();
    m_callButton->setObjectName("callButton");
    m_callButton->setIcon(QIcon(":/icons/audioCall.png"));
    m_videoCallButton = new QToolButton();
    m_videoCallButton->setObjectName("videoCallButton");
    m_videoCallButton->setIcon(QIcon(":/icons/videoCall.png"));
    m_moreOptionsButton = new QToolButton();
    m_moreOptionsButton->setObjectName("moreOptionsButton");
    m_moreOptionsButton->setIcon(QIcon(":/icons/dotsVertical.png"));

    // Компонуем созданные виджеты с помощью QVBoxLayout и QHBoxLayout.
    QVBoxLayout* userInfoLayout = new QVBoxLayout();
    userInfoLayout->addWidget(m_nameLabel);
    userInfoLayout->addWidget(m_statusLabel);
    userInfoLayout->setSpacing(0);
    userInfoLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout* headerLayout = qobject_cast<QHBoxLayout*>(ui->headerWidget->layout());
    if (!headerLayout) return; // Проверка на случай, если макет не был установлен в .ui файле.


    headerLayout->addLayout(userInfoLayout);
    headerLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    headerLayout->addWidget(m_searchButton);
    headerLayout->addWidget(m_callButton);
    headerLayout->addWidget(m_videoCallButton);
    headerLayout->addWidget(m_moreOptionsButton);

}

/**
 * @brief Сбрасывает текст-подсказку (placeholder) в поле ввода.
 */
void ChatViewWidget::clearReplyUI()
{
    ui->messageTextEdit->setPlaceholderText("Напишите сообщение...");
}

/**
 * @brief Слот, вызываемый по двойному клику на сообщении в списке.
 * @details Инициирует режим ответа на выбранное сообщение.
 * @param index Индекс сообщения, по которому был произведен двойной клик.
 */
void ChatViewWidget::onMessageDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
    showReplyUI(msg.fromUser, msg.payload);
    emit replyToMessageRequested(msg.id);
}

/**
 * @brief Слот, вызываемый по правому клику на списке сообщений.
 * @details Создает и показывает контекстное меню (Ответить, Редактировать, Удалить).
 * @param pos Позиция курсора в координатах виджета, где был сделан клик.
 */
void ChatViewWidget::onChatContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->chatHistoryView->indexAt(pos);
    if (!index.isValid()) return;

    ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
    QMenu contextMenu(this);
    QAction *replyAction = contextMenu.addAction("Ответить");
    QAction *editAction = contextMenu.addAction("Редактировать");
    QAction *deleteAction = contextMenu.addAction("Удалить");

    // Разрешаем/запрещаем действия в зависимости от того, наше ли это сообщение.
    if (!msg.isOutgoing) {
        editAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }

    // Показываем меню и ждем выбора пользователя.
    QAction *selectedAction = contextMenu.exec(ui->chatHistoryView->viewport()->mapToGlobal(pos));

    // Обрабатываем выбор пользователя, эмитируя соответствующие сигналы.
    if (selectedAction == replyAction) {
        onMessageDoubleClicked(index); // Используем ту же логику, что и для двойного клика.
    } else if (selectedAction == editAction) {
        emit editMessageRequested(msg.id, msg.payload);
    } else if (selectedAction == deleteAction) {
        emit deleteMessageRequested(msg.id);
    }
}

/**
 * @brief Геттер, возвращающий указатель на `QListView` истории чата.
 */
QListView* ChatViewWidget::chatHistoryView() const { return ui->chatHistoryView; }

/**
 * @brief Геттер, возвращающий указатель на `QTextEdit` поля ввода сообщения.
 */
QTextEdit* ChatViewWidget::messageTextEdit() const { return ui->messageTextEdit; }

/**
 * @brief Обновляет информацию в заголовке чата (имя, онлайн-статус, "печатает...").
 * @param chatPartner Объект `User` с актуальными данными о собеседнике.
 */
void ChatViewWidget::updateHeader(const User& chatPartner)
{
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

/**
 * @brief Вспомогательная функция для правильного склонения слов в зависимости от числа.
 */
QString pluralize(int n, const QString& form1, const QString& form2, const QString& form5) {
    n = abs(n) % 100;
    int n1 = n % 10;
    if (n > 10 && n < 20) return form5;
    if (n1 > 1 && n1 < 5) return form2;
    if (n1 == 1) return form1;
    return form5;
}

/**
 * @brief Форматирует строку о последнем времени визита пользователя.
 * @param user Объект пользователя.
 * @return QString Отформатированная строка (например, "в сети", "был(а) 5 минут назад", "был(а) сегодня в 14:30").
 */
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

/**
 * @brief Слот, вызываемый, когда приходит новое сообщение, а пользователь прокрутил чат вверх.
 */
void ChatViewWidget::onNewMessageReceived()
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    if (scrollBar->value() < scrollBar->maximum()) {
        m_unreadMessageCount++;
        updateScrollToBottomButton(); // Показываем кнопку "вниз" и счетчик.
    }
}

/**
 * @brief Слот для принудительной прокрутки чата в самый низ.
 */
void ChatViewWidget::scrollToBottom()
{
    ui->chatHistoryView->scrollToBottom();
    m_unreadMessageCount = 0; // Сбрасываем счетчик.
    updateScrollToBottomButton(); // Скрываем кнопку.
}

/**
 * @brief Слот, отслеживающий прокрутку чата пользователем.
 * @param value Новое значение позиции скроллбара.
 */
void ChatViewWidget::onChatScrolled(int value)
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    // Если пользователь сам докрутил до низа, сбрасываем счетчик непрочитанных.
    if (value == scrollBar->maximum() && m_unreadMessageCount > 0) {
        m_unreadMessageCount = 0;
        updateScrollToBottomButton();
    }
}

/**
 * @brief Проверяет, находится ли скроллбар в самом низу.
 * @return `true`, если скроллбар внизу (с небольшим допуском), иначе `false`.
 */
bool ChatViewWidget::isScrolledToBottom() const
{
    QScrollBar* scrollBar = ui->chatHistoryView->verticalScrollBar();
    return scrollBar->value() >= scrollBar->maximum() - 5;
}

/**
 * @brief Обработчик события изменения размера виджета.
 * @param event Указатель на событие.
 */
void ChatViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // При изменении размера окна обновляем позицию кнопки "вниз",
    // чтобы она всегда оставалась в правом нижнем углу над полем ввода.
    updateScrollToBottomButton();
}

/**
 * @brief Обновляет видимость и позицию кнопки "прокрутить вниз" и счетчика.
 */
void ChatViewWidget::updateScrollToBottomButton()
{
    if (m_unreadMessageCount > 0) {
        // Расчет позиции кнопки и счетчика.
        int margin = 15;
        QPoint buttonPos(width() - m_scrollToBottomButton->width() - margin, height() - m_scrollToBottomButton->height() - ui->messageInputWidget->height() - margin);
        m_scrollToBottomButton->move(buttonPos);

        m_unreadCountLabel->setText(QString::number(m_unreadMessageCount));
        QPoint labelPos(buttonPos.x() + (m_scrollToBottomButton->width() / 2), buttonPos.y() - m_unreadCountLabel->height() / 2);
        m_unreadCountLabel->move(labelPos);

        // Показываем виджеты.
        m_scrollToBottomButton->show();
        m_unreadCountLabel->show();
    } else {
        // Скрываем виджеты, если непрочитанных нет.
        m_scrollToBottomButton->hide();
        m_unreadCountLabel->hide();
    }
}

/**
 * @brief (Заготовка) Слот для обработки поиска по чату.
 * @param text Текст для поиска.
 */
void ChatViewWidget::onSearchTriggered(const QString& text)
{
    qDebug() << "Search in chat triggered:" << text;
}

/**
 * @brief Переключает виджет в режим редактирования сообщения.
 *
 * @details Этот публичный слот управляет состоянием панели ввода сообщения,
 *          переключая ее между режимом отправки нового сообщения и режимом
 *          редактирования существующего.
 *
 * @param enabled `true` для входа в режим редактирования, `false` для выхода.
 * @param text Исходный текст сообщения для редактирования. Используется только,
 *             когда `enabled` равно `true`.
 */
void ChatViewWidget::setEditMode(bool enabled, const QString& text)
{
    if (enabled) {
        // --- Вход в режим редактирования ---
        ui->sendButton->setText("Сохранить");
        ui->messageTextEdit->setText(text);
        ui->messageTextEdit->setFocus();
    } else {
        // --- Выход из режима редактирования (возврат к обычному состоянию) ---
        ui->sendButton->setText("Отправить");
        ui->messageTextEdit->clear();
        ui->messageTextEdit->setPlaceholderText("Напишите сообщение...");
    }
}

/**
 * @file mainwindow.cpp
 * @brief Реализация главного окна и центрального контроллера приложения.
 * @see MainWindow
 * @author kex1tu
 */


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
#include <QSplitter>
#include <QResizeEvent>

#include "searchresultspopup.h"
#include "chatfilterproxymodel.h"
#include "loginwidget.h"
#include "chatviewwidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatmessagedelegate.h"
#include "contactlistmodel.h"
#include "contactlistdelegate.h"
#include "chatmessagemodel.h"

/**
 * @brief Конструктор главного окна MainWindow.
 *
 * @details Этот конструктор выполняет всю основную работу по инициализации приложения:
 * 1.  Настраивает UI, сгенерированный из .ui файла.
 * 2.  Создает экземпляры ключевых моделей (ChatMessageModel).
 * 3.  Программно собирает основной интерфейс чата (`buildMainUI`).
 * 4.  Инициализирует сетевые компоненты (QTcpSocket).
 * 5.  Создает и настраивает таймеры для отложенных действий (поиск, "печатает").
 * 6.  Инициализирует карту обработчиков серверных команд (`initResponseHandlers`).
 * 7.  Соединяет все необходимые сигналы и слоты (`setupConnections`).
 * 8.  Устанавливает начальный виджет (экран входа) и инициирует подключение к серверу.
 *
 * @param parent Родительский виджет.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // 1. Инициализация UI из формы, созданной в Qt Designer (mainwindow.ui).
    ui->setupUi(this);

    // 2. Создание основных моделей данных.
    m_chatModel = new ChatMessageModel(this); // Модель для сообщений текущего чата.

    // 3. Создание виджетов, которые будут добавлены в QStackedWidget.
    m_loginWidget = new LoginWidget(this);
    buildMainUI(); // Вызов метода для программной сборки основного интерфейса чата.

    // 4. Добавление виджетов в корневой QStackedWidget для переключения между ними.
    ui->rootStackedWidget->addWidget(m_loginWidget);
    ui->rootStackedWidget->addWidget(m_mainChatWidget);

    // 5. Инициализация сетевого сокета.
    socket = new QTcpSocket(this);
    m_nextBlockSize = 0; // Для корректного чтения TCP-пакетов.

    // 6. Настройка таймеров.
    // Таймер для отложенного глобального поиска пользователей (срабатывает через 300мс после прекращения ввода).
    m_globalSearchTimer = new QTimer(this);
    m_globalSearchTimer->setSingleShot(true);
    m_globalSearchTimer->setInterval(300);

    // Таймер для отправки статуса "печатает" (не чаще, чем раз в 2 секунды).
    m_typingSendTimer = new QTimer(this);
    m_typingSendTimer->setSingleShot(true);
    m_typingSendTimer->setInterval(2000);

    // 7. Инициализация вспомогательных виджетов и систем.
    m_searchResultsPopup = new SearchResultsPopup(this);
    // Установка фильтра событий для отслеживания кликов вне `m_searchResultsPopup`.
    qApp->installEventFilter(this);

    // 8. Заполнение карты обработчиков команд от сервера.
    initResponseHandlers();

    // 9. Установка всех соединений "сигнал-слот".
    setupConnections();

    // 10. Отображение начального экрана.
    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);

    // 11. Запуск процесса подключения к серверу.
    connectToServer();
}

/**
 * @brief Программно собирает и компонует основной пользовательский интерфейс чата.
 *
 * @details Этот метод отвечает за создание и настройку всех виджетов,
 *          составляющих главный экран приложения (список контактов, панель чата),
 *          и их размещение с помощью менеджеров компоновки.
 */
void MainWindow::buildMainUI()
{
    // --- 1. Создание корневого виджета для всего интерфейса чата ---
    m_mainChatWidget = new QWidget();
    auto* mainLayout = new QHBoxLayout(m_mainChatWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Убираем отступы по краям.
    mainLayout->setSpacing(1); // Маленький отступ между левой и правой панелями.

    // --- 2. Создание и настройка левой панели (список контактов) ---
    m_chatListPanel = new QWidget();
    m_chatListPanel->setObjectName("chatListPanel"); // Для применения стилей из QSS.

    auto* leftLayout = new QVBoxLayout(m_chatListPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Поиск контактов...");

    // Используем связку QListView + ContactListModel для списка контактов.
    m_userListView = new QListView(this);
    m_contactModel = new ContactListModel(this, this);
    m_userListView->setModel(m_contactModel);
    // Устанавливаем кастомный делегат для отрисовки элементов.
    m_userListView->setItemDelegate(new ContactListDelegate(this));
    m_userListView->setFixedWidth(200);
    m_userListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    m_logoutButton = new QPushButton("Выйти");
    m_logoutButton->setObjectName("logoutButton");

    // Добавляем виджеты на левую панель.
    leftLayout->addWidget(m_searchLineEdit);
    leftLayout->addWidget(m_userListView);
    leftLayout->addWidget(m_logoutButton);

    // --- 3. Создание и настройка правой панели (область чата) ---
    m_rightSideContainer = new QWidget();
    m_rightSideContainer->setObjectName("rightSideContainer");

    m_rightSideLayout = new QStackedLayout(); // QStackedLayout для переключения между заглушкой и чатом.

    // Создаем виджет-заглушку, который показывается, когда ни один чат не выбран.
    m_placeholderWidget = new QWidget();
    auto* placeholderLayout = new QVBoxLayout(m_placeholderWidget);
    auto* placeholderLabel = new QLabel("Выберите чат, чтобы начать общение");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setObjectName("placeholderLabel");
    placeholderLayout->addWidget(placeholderLabel);

    // Создаем виджет самого чата и настраиваем его модель и делегат.
    m_chatViewWidget = new ChatViewWidget();
    QListView* chatView = m_chatViewWidget->chatHistoryView();
    chatView->setModel(m_chatModel);
    ChatMessageDelegate* delegate = new ChatMessageDelegate(m_chatModel, this);
    chatView->setItemDelegate(delegate);

    // Добавляем заглушку и виджет чата в QStackedLayout.
    m_rightSideLayout->addWidget(m_placeholderWidget);
    m_rightSideLayout->addWidget(m_chatViewWidget);

    m_rightSideContainer->setLayout(m_rightSideLayout);

    // --- 4. Компоновка главного макета ---
    // Добавляем левую и правую панели в основной горизонтальный макет.
    // `0` - stretch factor для левой панели (не растягивать).
    // `1` - stretch factor для правой панели (будет занимать все доступное место).
    mainLayout->addWidget(m_chatListPanel, 0);
    mainLayout->addWidget(m_rightSideContainer, 1);
}

/**
 * @brief Устанавливает все соединения "сигнал-слот" между объектами приложения.
 *
 * @details Этот метод вызывается один раз в конструкторе и отвечает за создание
 *          всей логики взаимодействия. Он связывает:
 *          - События сетевого сокета с их обработчиками.
 *          - Сигналы от UI-виджетов (LoginWidget, ChatViewWidget) с основной логикой в MainWindow.
 *          - Сигналы от моделей данных с их обработчиками.
 *          - Работу таймеров с соответствующими слотами.
 */
void MainWindow::setupConnections()
{
    // --- 1. Соединения для сетевого сокета (QTcpSocket) ---
    // Эти три соединения управляют основным циклом сетевого взаимодействия.
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);     // При успешном подключении.
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);       // Когда от сервера приходят новые данные.
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected); // При разрыве соединения.

    // --- 2. Соединения для виджета входа/регистрации (LoginWidget) ---
    // MainWindow "слушает" действия пользователя на экране входа.
    connect(m_loginWidget, &LoginWidget::loginRequested, this, &MainWindow::onLoginRequested);
    connect(m_loginWidget, &LoginWidget::registerRequested, this, &MainWindow::onRegisterRequested);

    // --- 3. Соединения для виджета чата (ChatViewWidget) ---
    // Связываем все возможные действия пользователя внутри чата с методами в MainWindow.
    connect(m_chatViewWidget, &ChatViewWidget::sendMessageRequested, this, &MainWindow::onSendMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::headerClicked, this, &MainWindow::showProfileView);
    connect(m_chatViewWidget->messageTextEdit(), &QTextEdit::textChanged, this, &MainWindow::onTypingNotificationFired); // Для статуса "печатает".
    connect(m_chatViewWidget, &ChatViewWidget::replyToMessageRequested, this, &MainWindow::onReplyToMessage);
    connect(m_chatViewWidget, &ChatViewWidget::editMessageRequested, this, &MainWindow::onEditMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::deleteMessageRequested, this, &MainWindow::onDeleteMessageRequested);
    // Обработка отмены ответа через лямбда-функцию для простого действия.
    connect(m_chatViewWidget, &ChatViewWidget::replyCancelled, this, [this](){
        m_replyToMessageId = 0;
    });

    // --- 4. Соединения для скроллбара истории чата ---
    QScrollBar* scrollBar = m_chatViewWidget->chatHistoryView()->verticalScrollBar();
    // Когда пользователь отпускает скроллбар, проверяем, какие сообщения стали видимыми.
    //connect(scrollBar, &QScrollBar::sliderReleased, this, &MainWindow::processVisibleMessages);
    connect(scrollBar, &QScrollBar::valueChanged, this, &MainWindow::processVisibleMessages);
    // Отслеживаем прокрутку для подгрузки старой истории, когда достигается верх.
    connect(scrollBar, &QScrollBar::valueChanged, this, &MainWindow::onChatScroll);

    // --- 5. Соединения для списка контактов ---
    // Используем `selectionModel()` для отслеживания изменения ВЫБРАННОГО элемента.
    connect(m_userListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onUserSelectionChanged);

    // --- 6. Соединения для глобального поиска ---
    // Отложенный запуск поиска: при изменении текста запускается таймер.
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        if (text.isEmpty()) m_globalSearchTimer->stop();
        else m_globalSearchTimer->start();
    });
    // Когда таймер срабатывает, выполняется фактический поиск.
    connect(m_globalSearchTimer, &QTimer::timeout, this, &MainWindow::onGlobalSearchTriggered);

    // Обработка выбора пользователя из всплывающего списка результатов поиска.
    connect(m_searchResultsPopup, &SearchResultsPopup::userSelected, this, &MainWindow::onAddContactRequested);

    // --- 7. Прочие соединения ---
    // Кнопка выхода из аккаунта.
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);

    // Внутренний сигнал MainWindow для уведомления ChatViewWidget о новом сообщении.
    connect(this, &MainWindow::newMessageForCurrentChat, m_chatViewWidget, &ChatViewWidget::onNewMessageReceived);

    // Сигнал от модели сообщений, требующий отправить на сервер уведомление о прочтении.
    connect(m_chatModel, &ChatMessageModel::messageNeedsReadReceipt, this, &MainWindow::onSendMessageReadReceipt);
}

/**
 * @brief Обрабатывает все видимые в данный момент сообщения в чате.
 *
 * @details Этот слот вызывается после того, как пользователь завершил прокрутку
 *          (`sliderReleased`). Он итерируется по всем видимым элементам в `QListView`
 *          и для каждого входящего, еще не прочитанного сообщения, инициирует
 *          процесс отправки уведомления о прочтении. Это более надежный и
 *          производительный способ, чем делать это в делегате.
 */
void MainWindow::processVisibleMessages()
{
    qDebug() <<"processVisibleMessages";
    QListView* view = m_chatViewWidget->chatHistoryView();

    // Проверка, что чат активен и у него есть модель.
    if (!view->model() || m_currentChatPartner.username.isEmpty()) {
        return;
    }

    // Получаем индексы первого и последнего видимого элемента в области просмотра.
    QModelIndex firstVisible = view->indexAt(view->rect().topLeft());
    QModelIndex lastVisible = view->indexAt(view->rect().bottomLeft());

    // Если нет видимых элементов (например, чат пуст), выходим.
    if (!firstVisible.isValid()) {
        return;
    }

    // Проходимся в цикле от первого видимого до последнего.
    for (int row = firstVisible.row(); row <= lastVisible.row(); ++row) {
        QModelIndex index = m_chatModel->index(row, 0);
        if (!index.isValid()) continue; // Пропускаем, если индекс невалидный.

        // Получаем объект сообщения из модели.
        ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();

        // Главное условие: если сообщение ВХОДЯЩЕЕ и его статус "Доставлено"...
        if (!msg.isOutgoing && (msg.status == ChatMessage::Delivered|| msg.status == ChatMessage::Sent)) {
            // ...то мы эмитируем сигнал от модели.
            // Модель сама изменит статус на "Прочитано" и уведомит MainWindow,
            // что нужно отправить сетевой запрос.
            // Мы не вызываем markMessageAsRead напрямую, чтобы сохранить инкапсуляцию,
            // хотя это тоже был бы рабочий вариант. Эмитирование сигнала здесь более гибко.
            emit m_chatModel->messageNeedsReadReceipt(msg.id);
        }
    }
}

/**
 * @brief Слот, который активирует режим редактирования сообщения.
 *
 * @details Этот слот вызывается, когда пользователь выбирает "Редактировать"
 *          в контекстном меню сообщения в `ChatViewWidget`.
 *
 * @param messageId Уникальный ID сообщения, которое нужно отредактировать.
 * @param oldText Текущий текст этого сообщения, который будет помещен в поле ввода.
 */
void MainWindow::onEditMessageRequested(qint64 messageId, const QString& oldText)
{
    qDebug() << "MainWindow: Caught editMessageRequested signal for ID:" << messageId;

    // 1. Устанавливаем флаг `m_editingMessageId`, чтобы `onSendMessageRequested`
    //    знал, что нужно отправить команду "edit_message", а не "private_message".
    m_editingMessageId = messageId;

    // 2. Вызываем публичный метод `ChatViewWidget`, чтобы он переключил свой UI
    //    в режим редактирования (изменил кнопку, вставил текст и т.д.).
    m_chatViewWidget->setEditMode(true, oldText);
}

/**
 * @brief Слот, который отправляет на сервер запрос на удаление сообщения.
 *
 * @details Этот слот вызывается, когда пользователь выбирает "Удалить"
 *          в контекстном меню своего сообщения в `ChatViewWidget`.
 *
 * @param messageId Уникальный ID сообщения, которое нужно удалить.
 */
void MainWindow::onDeleteMessageRequested(qint64 messageId)
{
    // 1. Создаем JSON-объект запроса.
    QJsonObject request;
    request["type"] = "delete_message"; // Тип команды.
    request["id"] = messageId;          // ID удаляемого сообщения.

    // 2. Отправляем сформированный запрос на сервер.
    sendJson(request);
}

/**
 * @brief Обрабатывает запрос на добавление нового контакта.
 *
 * @details Этот слот вызывается, когда пользователь выбирает кого-то
 *          из всплывающего списка результатов поиска (`SearchResultsPopup`).
 *          Он показывает диалог подтверждения и, в случае согласия,
 *          отправляет запрос на добавление контакта на сервер.
 *
 * @param username Имя пользователя, которого нужно добавить в контакты.
 */
void MainWindow::onAddContactRequested(const QString& username)
{
    // 1. Скрываем всплывающее окно с результатами поиска, так как выбор сделан.
    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }

    // 2. Показываем стандартный диалог `QMessageBox` для подтверждения действия.
    //    Это хороший UX, предотвращающий случайные запросы.
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Добавить контакт",
                                  "Отправить запрос на добавление в контакты пользователю " + username + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    // 3. Если пользователь нажал "Да"...
    if (reply == QMessageBox::Yes) {
        qDebug() << "[CLIENT] Sending 'add_contact_request' for user:" << username;

        // ...формируем и отправляем соответствующий JSON-запрос на сервер.
        QJsonObject request;
        request["type"] = "add_contact_request";
        request["username"] = username;
        sendJson(request);
    }
}

/**
 * @brief Отправляет на сервер уведомление о наборе текста ("печатает...").
 *
 * @details Этот слот подключен к сигналу `textChanged` поля ввода сообщения.
 *          Чтобы не отправлять уведомление на каждое нажатие клавиши, используется
 *          таймер `m_typingSendTimer`, который ограничивает частоту отправок.
 */
void MainWindow::onTypingNotificationFired()
{
    // 1. Проверяем, что чат с кем-то открыт.
    if (m_currentChatPartner.username.isEmpty()) return;

    // 2. Не отправляем уведомление, если поле ввода снова стало пустым.
    if (m_chatViewWidget->messageTextEdit()->toPlainText().isEmpty()) return;

    // 3. Проверяем, активен ли таймер. Если да, значит, мы уже отправили
    //    уведомление недавно (в течение последних 2 секунд) и новое пока не нужно.
    if (m_typingSendTimer->isActive()) return;

    // 4. Если все проверки пройдены, формируем и отправляем запрос.
    QJsonObject typingRequest;
    typingRequest["type"] = "typing";
    typingRequest["toUser"] = m_currentChatPartner.username;
    sendJson(typingRequest);

    // 5. Запускаем таймер, чтобы предотвратить слишком частые отправки.
    m_typingSendTimer->start();
}

/**
 * @brief Активирует режим ответа на сообщение.
 *
 * @details Этот слот вызывается, когда пользователь выбирает "Ответить"
 *          в контекстном меню или делает двойной клик по сообщению.
 *
 * @param messageId ID сообщения, на которое нужно ответить.
 */
void MainWindow::onReplyToMessage(qint64 messageId)
{
    qDebug() << "[CLIENT] Setting reply context to message ID:" << messageId;
    ChatMessage msg;

    // 1. Пытаемся получить данные цитируемого сообщения из модели по его ID.
    if (m_chatModel->getMessageById(messageId, msg)) {
        // 2. Если сообщение найдено, сохраняем его ID. `m_replyToMessageId`
        //    будет использован при отправке следующего сообщения.
        m_replyToMessageId = messageId;

        // 3. Вызываем метод `ChatViewWidget`, чтобы он отобразил панель ответа
        //    с именем автора и текстом цитаты.
        m_chatViewWidget->showReplyUI(msg.fromUser, msg.payload);
    }
}

/**
 * @brief (ЗАГОТОВКА) Обрабатывает запрос на поиск текста в истории текущего чата.
 *
 * @details Этот слот предназначен для активации фильтрации в `m_chatFilterProxyModel`,
 *          чтобы в списке сообщений отображались только те, которые содержат
 *          поисковый запрос.
 *
 * @param text Текст для поиска.
 */
void MainWindow::onChatSearchTriggered(const QString &text)
{
}

/**
 * @brief Инициирует подключение к TCP-серверу.
 *
 * @details Этот метод использует жестко заданные адрес и порт сервера
 *          и вызывает асинхронный метод `connectToHost`. Результат подключения
 *          будет обработан в слотах `onConnected()` или `onErrorOccurred()`.
 */
void MainWindow::connectToServer() {
    const QString host = "127.0.0.1"; // Адрес сервера (localhost).
    const quint16 port = 1234;       // Порт сервера.

    // Асинхронно пытаемся подключиться к серверу.
    socket->connectToHost(host, port);

    qDebug() << "[Client] Attempting to connect to host on " << host << ":" << port;
}

/**
 * @brief Слот, вызываемый при успешном установлении TCP-соединения.
 *
 * @details Этот метод обновляет UI, чтобы показать пользователю, что соединение
 *          установлено, и активирует элементы управления на экране входа,
 *          позволяя пользователю начать аутентификацию.
 */
void MainWindow::onConnected() {
    qDebug() << "[CLIENT] Socket successfully connected!";

    // Показываем временное сообщение в статус-баре.
    if (ui->statusbar) {
        ui->statusbar->showMessage("Подключено", 2000); // Сообщение исчезнет через 2000 мс.
    }

    // Делаем поля ввода и кнопки на экране входа активными.
    m_loginWidget->setUiEnabled(true);
}

/**
 * @brief Слот, вызываемый при разрыве TCP-соединения.
 *
 * @details Этот метод обновляет UI, информируя пользователя о потере соединения,
 *          и блокирует элементы управления, которые требуют активного подключения,
 *          чтобы предотвратить отправку данных в "пустоту".
 */
void MainWindow::onDisconnected() {
    qDebug() << "[CLIENT] Socket disconnected!";

    // Показываем постоянное сообщение в статус-баре.
    if (ui->statusbar) {
        ui->statusbar->showMessage("Отключено");
    }

    // Блокируем UI на экране входа, так как без соединения он бесполезен.
    m_loginWidget->setUiEnabled(false);

    // TODO: Добавить логику обработки разрыва соединения, когда пользователь уже вошел в систему.
    // Например, показать уведомление "Потеряно соединение, попытка переподключения..."
    // и, возможно, сбросить состояние приложения (resetApplicationState).
    if (ui->rootStackedWidget->currentWidget() == m_mainChatWidget) {
    }
}

/**
 * @brief Сериализует JSON-объект и отправляет его на сервер по TCP.
 *
 * @details Этот метод реализует кастомный протокол отправки:
 *          1. JSON-объект преобразуется в компактную строку `QByteArray`.
 *          2. С помощью `QDataStream` формируется пакет, где сначала идет
 *             4-байтовое число (quint32) с размером последующих данных,
 *             а затем сами JSON-данные.
 *          Это позволяет серверу на другой стороне точно знать, сколько байт
 *          нужно прочитать, чтобы получить полный и неповрежденный JSON-пакет.
 *
 * @param json JSON-объект, который нужно отправить.
 */
void MainWindow::sendJson(const QJsonObject& json)
{
    qDebug() << "---------------------------------";
    qDebug() << "[CLIENT SENDING] Preparing to send JSON of type:" << json["type"].toString();
    qDebug() << "[CLIENT SENDING] Full JSON content:" << json;
    qDebug() << "---------------------------------";

    // 1. Преобразуем JSON-объект в байтовый массив.
    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    // 2. Создаем блок для отправки.
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2); // Важно для совместимости с сервером.

    // 3. Записываем в начало блока "пустое" место (0) для размера.
    out << (quint32)0;

    // 4. Записываем сами данные.
    out << jsonData;

    // 5. Возвращаемся в начало блока.
    out.device()->seek(0);

    // 6. Записываем в "пустое" место реальный размер всего блока минус размер самого поля для размера.
    out << (quint32)(block.size() - sizeof(quint32));

    // 7. Отправляем итоговый блок по сокету.
    socket->write(block);
}

/**
 * @brief Слот, вызываемый, когда в TCP-сокете появляются новые данные для чтения.
 *
 * @details Этот метод реализует логику чтения пакетов, отправленных по протоколу,
 *          описанному в `sendJson`. Он может обработать несколько JSON-пакетов,
 *          пришедших одновременно, или дождаться полной загрузки "разбитого" пакета.
 *          После успешного извлечения JSON-объекта он передает его на обработку
 *          в `processJsonRequest` (которая была позже заменена на логику с `m_responseHandlers`).
 */
void MainWindow::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2); // Версия должна совпадать с сервером.

    // Запускаем бесконечный цикл, который будет читать данные, пока они есть в буфере.
    while(true) {
        // Если мы не знаем размер следующего блока...
        if (m_nextBlockSize == 0) {
            // ...проверяем, достаточно ли данных в буфере, чтобы прочитать сам размер (4 байта).
            if (socket->bytesAvailable() < sizeof(quint32)) {
                break; // Данных мало, выходим и ждем следующего вызова onReadyRead.
            }
            // Читаем размер следующего блока.
            in >> m_nextBlockSize;
        }

        // Теперь мы знаем размер. Проверяем, достаточно ли данных в буфере,
        // чтобы прочитать весь блок целиком.
        if (socket->bytesAvailable() < m_nextBlockSize) {
            break; // Пакет еще не пришел целиком. Выходим и ждем.
        }

        // Данных достаточно, читаем сам JSON-пакет.
        QByteArray jsonData;
        in >> jsonData;

        // Сбрасываем размер блока, чтобы быть готовыми к чтению следующего.
        m_nextBlockSize = 0;

        // Парсим JSON.
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            qDebug() << "[CLIENT] Failed to parse JSON or it's not an object.";
            continue; // Пропускаем поврежденный пакет.
        }

        // --- Маршрутизация команды ---
        QJsonObject response = doc.object();
        QString type = response["type"].toString();
        qDebug() << "[CLIENT] Processing message of type" << type;

        // Ищем обработчик для данного типа команды в нашей карте.
        if (m_responseHandlers.contains(type)) {
            // Если найден, вызываем соответствующий метод-обработчик.
            ResponseHandler handler = m_responseHandlers[type];
            (this->*handler)(response);
        } else {
            qDebug() << "[CLIENT] No handler found for message type:" << type;
        }
    }
}

/**
 * @brief Инициализирует карту обработчиков команд от сервера.
 *
 * @details Этот метод реализует паттерн "Команда" (Command Pattern). Он создает
 *          сопоставление между строковым идентификатором команды (ключ в JSON "type")
 *          и указателем на метод-обработчик внутри класса `MainWindow`.
 *
 *          Это позволяет методу `onReadyRead()` динамически вызывать нужную логику,
 *          избегая громоздких конструкций `if-else` или `switch`, и делает
 *          добавление новых команд очень простым: достаточно добавить одну строку в этот метод
 *          и реализовать соответствующий метод-обработчик.
 */
void MainWindow::initResponseHandlers()
{
    // `m_responseHandlers` - это QMap<QString, ResponseHandler>,
    // где `ResponseHandler` - это псевдоним для указателя на метод класса MainWindow.

    // --- Аутентификация ---
    m_responseHandlers["login_success"] = &MainWindow::handleLoginSuccess;
    m_responseHandlers["login_failure"] = &MainWindow::handleLoginFailure;
    m_responseHandlers["register_success"] = &MainWindow::handleRegisterSuccess;
    m_responseHandlers["register_failure"] = &MainWindow::handleRegisterFailure;
    m_responseHandlers["logout_request_success"] = &MainWindow::handleLogoutSuccess;
    m_responseHandlers["logout_request_failure"] = &MainWindow::handleLogoutFailure;

    // --- Работа с контактами и пользователями ---
    m_responseHandlers["contact_list"] = &MainWindow::handleContactList;
    m_responseHandlers["user_list"] = &MainWindow::handleUserList; // Список онлайн-пользователей
    m_responseHandlers["search_results"] = &MainWindow::handleSearchResults;
    m_responseHandlers["add_contact_success"] = &MainWindow::handleAddContactSuccess;
    m_responseHandlers["add_contact_failure"] = &MainWindow::handleAddContactFailure;
    m_responseHandlers["incoming_contact_request"] = &MainWindow::handleIncomingContactRequest;
    m_responseHandlers["pending_requests_list"] = &MainWindow::handlePendingRequestsList;

    // --- Работа с сообщениями ---
    m_responseHandlers["history_data"] = &MainWindow::handleHistoryData;
    m_responseHandlers["old_history_data"] = &MainWindow::handleOldHistoryData;
    m_responseHandlers["private_message"] = &MainWindow::handlePrivateMessage;
    m_responseHandlers["edit_message"] = &MainWindow::handleEditMessage;
    m_responseHandlers["delete_message"] = &MainWindow::handleDeleteMessage;

    // --- Статусы сообщений и пользователя ---
    m_responseHandlers["message_delivered"] = &MainWindow::handleMessageDelivered;
    m_responseHandlers["message_read"] = &MainWindow::handleMessageRead;
    m_responseHandlers["typing"] = &MainWindow::handleTypingResponse;
    m_responseHandlers["unread_counts"] = &MainWindow::handleUnreadCounts; // Первоначальная загрузка непрочитанных.
}

/**
 * @brief Обрабатывает успешный ответ на запрос входа.
 *
 * @details Этот слот-обработчик вызывается, когда сервер подтверждает, что
 *          логин и пароль верны. Он выполняет все необходимые действия для
 *          перехода из состояния "не аутентифицирован" в состояние "в чате".
 *
 * @param response JSON-объект ответа от сервера (в данном случае не используется,
 *                 так как сам факт вызова этого метода уже означает успех).
 */
void MainWindow::handleLoginSuccess(const QJsonObject& response)
{
    Q_UNUSED(response); // Параметр пока не используется.

    // 1. Сохраняем имя текущего пользователя.
    m_currentUsername = m_loginWidget->username();
    qDebug() << "[CLIENT] Login successful for user:" << m_currentUsername;

    // 2. Очищаем поля на экране входа.
    m_loginWidget->clearFields();

    // 3. Переключаем `QStackedWidget`, чтобы показать основной интерфейс чата.
    ui->rootStackedWidget->setCurrentWidget(m_mainChatWidget);

    // 4. Устанавливаем имя пользователя в заголовок главного окна.
    this->setWindowTitle(m_currentUsername);
}

/**
 * @brief Обрабатывает ответ сервера с первоначальным списком непрочитанных сообщений.
 *
 * @details Этот метод вызывается один раз после успешного входа. Сервер присылает
 *          список всех чатов, в которых для текущего пользователя есть непрочитанные
 *          сообщения, и их количество.
 *
 * @param response JSON-объект ответа от сервера, содержащий массив "counts".
 */
void MainWindow::handleUnreadCounts(const QJsonObject& response)
{
    qDebug() << "[CLIENT] Received initial unread counts from server.";
    QJsonArray countsArray = response["counts"].toArray();

    // Очищаем локальный кэш счетчиков перед заполнением.
    m_unreadCounts.clear();

    // Проходимся по массиву, полученному от сервера.
    for (const QJsonValue &value : countsArray) {
        QJsonObject countObj = value.toObject();
        QString username = countObj["username"].toString(); // От кого непрочитанные.
        int count = countObj["count"].toInt();          // Сколько.

        if (count > 0) {
            // Заполняем наш кэш `m_unreadCounts`.
            m_unreadCounts[username] = count;
        }
    }

    // После того как кэш обновлен, мы принудительно обновляем
    // отображение списка контактов, чтобы `ContactListDelegate` смог
    // отрисовать "бейджи" с количеством непрочитанных.
    updateUserList();
}

/**
 * @brief Обрабатывает неудачный ответ на запрос входа.
 *
 * @details Этот слот-обработчик вызывается, когда сервер отклоняет попытку входа
 *          (например, из-за неверного пароля или несуществующего пользователя).
 *          Он просто показывает пользователю диалоговое окно с причиной ошибки.
 *
 * @param response JSON-объект ответа от сервера, содержащий поле "reason" с текстом ошибки.
 */
void MainWindow::handleLoginFailure(const QJsonObject& response)
{
    // Показываем стандартное окно с предупреждением.
    // Заголовок окна - "error login", текст - причина, полученная от сервера.
    QMessageBox::warning(this, "Ошибка входа", response["reason"].toString());
}

/**
 * @brief Обрабатывает успешный ответ на запрос регистрации.
 *
 * @details Этот слот-обработчик вызывается, когда сервер подтверждает, что
 *          новый пользователь был успешно создан. Он не выполняет никаких
 *          действий сам, а делегирует эту задачу `LoginWidget`.
 *
 * @param response JSON-объект ответа от сервера (в данном случае не используется).
 */
void MainWindow::handleRegisterSuccess(const QJsonObject& response)
{
    Q_UNUSED(response); // Параметр не используется.

    qDebug() << "[CLIENT] Registration successful.";

    // Вызываем публичный слот `LoginWidget`, который покажет пользователю
    // информационное сообщение и переключит интерфейс на вкладку входа.
    m_loginWidget->onRegistrationSuccess();
}

/**
 * @brief Обрабатывает неудачный ответ на запрос регистрации.
 *
 * @details Этот слот-обработчик вызывается, когда сервер отклоняет попытку регистрации
 *          (например, потому что имя пользователя уже занято).
 *          Он показывает пользователю диалоговое окно с причиной ошибки.
 *
 * @param response JSON-объект ответа от сервера, содержащий поле "reason" с текстом ошибки.
 */
void MainWindow::handleRegisterFailure(const QJsonObject& response)
{
    // Показываем стандартное окно с предупреждением.
    QMessageBox::warning(this, "Ошибка регистрации", response["reason"].toString());
}

/**
 * @brief Обрабатывает ответ сервера со списком контактов пользователя.
 *
 * @details Этот метод вызывается после успешного входа. Он получает от сервера
 *          список всех подтвержденных контактов текущего пользователя,
 *          заполняет локальный кэш `m_userCache` этими данными и затем
 *          запускает обновление UI списка контактов.
 *
 * @param response JSON-объект ответа от сервера, содержащий массив "users".
 */
void MainWindow::handleContactList(const QJsonObject& response)
{
    qDebug() << "[CLIENT] Received contact list from server.";
    QJsonArray usersFromServer = response["users"].toArray();

    // Очищаем кэш пользователей перед заполнением новыми данными.
    m_userCache.clear();

    // Проходимся по каждому пользователю в полученном JSON-массиве.
    for (const QJsonValue &value : usersFromServer) {
        QJsonObject userObj = value.toObject();
        User user;
        // Парсим данные и заполняем нашу локальную структуру User.
        user.username = userObj["username"].toString();
        user.displayName = userObj["displayname"].toString();
        user.lastSeen = userObj["last_seen"].toString();
        qDebug() << "  - Loaded contact:" << user.username;

        // Вставляем пользователя в кэш. QMap автоматически обработает
        // создание нового элемента или обновление существующего.
        // Проверка `if (user.username != m_currentUsername)` здесь, возможно,
        // избыточна, так как сервер обычно не присылает пользователя в его
        // собственном списке контактов, но она добавляет надежности.
        if (user.username != m_currentUsername) {
            m_userCache.insert(user.username, user);
        }
    }

    // После того как кэш `m_userCache` обновлен, вызываем `updateUserList()`,
    // который, в свою очередь, обновит `ContactListModel`, что приведет
    // к перерисовке списка контактов на экране.
    updateUserList();
}
/**
 * @brief Обрабатывает ответ сервера с первоначальным "пакетом" истории сообщений.
 *
 * @details Этот метод вызывается, когда пользователь открывает чат, для которого
 *          еще нет данных в локальном кэше. Он получает от сервера последний
 *          "срез" сообщений (например, последние 20), обрабатывает их,
 *          заполняет `ChatMessageModel` для отображения и сохраняет их в
 *          глобальный кэш `m_chatHistoryCache` для последующего быстрого доступа.
 *
 * @param response JSON-объект ответа от сервера, содержащий массив "history"
 *                 и поле "with_user", указывающее, для какого чата эта история.
 */
void MainWindow::handleHistoryData(const QJsonObject& response)
{
    // 1. Проверяем, что полученная история относится к чату, который открыт в данный момент.
    //    Это предотвращает редкую ситуацию, когда пользователь очень быстро переключает чаты,
    //    а ответ от сервера на старый запрос приходит с опозданием.
    QString historyForUser = response["with_user"].toString();
    if (historyForUser != m_currentChatPartner.username) return;

    // --- 2. Парсинг и обработка сообщений ---
    QJsonArray history = response["history"].toArray();
    qDebug() << "[CLIENT] Displaying initial" << history.count() << "messages for" << historyForUser;

    QList<ChatMessage> messages; // Временный список для хранения распарсенных сообщений.
    for (const QJsonValue &value : history) {
        QJsonObject msgObj = value.toObject();
        ChatMessage msg;

        // Заполняем структуру ChatMessage данными из JSON.
        msg.id = msgObj["id"].toDouble();
        msg.fromUser = msgObj["fromUser"].toString();
        msg.toUser = msgObj["toUser"].toString();
        msg.payload = msgObj["payload"].toString();
        msg.timestamp = msgObj["timestamp"].toString();
        msg.replyToId = msgObj["reply_to_id"].toDouble();
        msg.isEdited = msgObj["is_edited"].toInt();
        msg.isOutgoing = (msg.fromUser == m_currentUsername); // Флаг, который мы вычисляем на клиенте.

        // Определяем статус сообщения (для исходящих).
        if(msgObj["is_read"].toInt() == 1) {
            msg.status = ChatMessage::Read;
        } else if (msgObj["is_delivered"].toInt() == 1) {
            msg.status = ChatMessage::Delivered;
        } else {
            msg.status = ChatMessage::Sent;
        }

        messages.append(msg);
    }
    ChatCache& cache = m_chatHistoryCache[historyForUser];
    cache.messages = messages;
    if (!messages.isEmpty()) {
        cache.oldestMessageId = messages.first().id;
    } else {
        cache.allMessagesLoaded = true;
    }
    m_oldestMessageId = cache.oldestMessageId; // Обновляем состояние MainWindow

    // Просто и чисто: очищаем модель и загружаем в нее новые данные.
    m_chatModel->clearMessages();
    m_chatModel->addMessages(messages);

    // --- 4. Обновление UI и состояния ---
    QMetaObject::invokeMethod(m_chatViewWidget->chatHistoryView(), "scrollToBottom", Qt::QueuedConnection);

    QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);
    m_isLoadingHistory = false;
}

/**
 * @brief Обрабатывает входящие личные сообщения от сервера.
 *
 * @details Этот метод является одним из самых важных, так как он обрабатывает
 *          как "echo"-ответы на наши собственные отправленные сообщения, так и
 *          новые сообщения от других пользователей.
 *
 * @param response JSON-объект, представляющий сообщение.
 */
void MainWindow::handlePrivateMessage(const QJsonObject& response) {

    // --- Ветка 1: Обработка "echo"-ответа на наше собственное сообщение ---
    // Сервер, получив наше сообщение, сохраняет его в БД, присваивает ему ID
    // и возвращает нам его обратно с этим ID и `temp_id`, который мы ему прислали.
    QString tempId = response["temp_id"].toString();
    if (!tempId.isEmpty()) {
        qDebug() << "[CLIENT] Received ECHO for temp_id:" << tempId;

        // 1. Парсим подтвержденное сообщение.
        ChatMessage msg;
        msg.id = response["id"].toDouble(); // <-- Самое важное: получаем реальный ID.
        msg.tempId = tempId;
        msg.fromUser = response["fromUser"].toString();
        msg.toUser = response["toUser"].toString();
        msg.payload = response["payload"].toString();
        msg.timestamp = response["timestamp"].toString(); // Серверное время.
        msg.replyToId = response["reply_to_id"].toDouble();
        msg.isOutgoing = true;
        msg.status = ChatMessage::MessageStatus::Sent; // Статус меняется с "Sending" на "Sent".
        msg.isEdited = false;

        // 2. Обновляем сообщение в модели, видимой на экране.
        //    Модель найдет сообщение по `tempId` и заменит его на `msg`.
        m_chatModel->confirmMessage(tempId, msg);

        // 3. Обновляем сообщение в глобальном кэше истории.
        QString chatPartner = msg.toUser;
        if (m_chatHistoryCache.contains(chatPartner)) {
            QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
            // Ищем временное сообщение в кэше и заменяем его на подтвержденное.
            for (int i = 0; i < messagesInCache.size(); ++i) {
                if (messagesInCache[i].tempId == tempId) {
                    messagesInCache[i] = msg;
                    qDebug() << "[CACHE] Confirmed message with temp_id:" << tempId << "in cache for" << chatPartner;
                    break;
                }
            }
        }

        return; // Завершаем выполнение, так как это был echo-ответ.
    }

    // --- Ветка 2: Обработка нового входящего сообщения от другого пользователя ---
    // Если `temp_id` пустой, значит, это сообщение от кого-то другого.

    // 1. Парсим входящее сообщение.
    ChatMessage incomingMsg;
    incomingMsg.id = response["id"].toDouble();
    incomingMsg.fromUser = response["fromUser"].toString();
    incomingMsg.toUser = response["toUser"].toString();
    incomingMsg.payload = response["payload"].toString();
    incomingMsg.timestamp = response["timestamp"].toString();
    incomingMsg.replyToId = response["reply_to_id"].toDouble();
    incomingMsg.isOutgoing = false;
    incomingMsg.isEdited = false;

    // Определяем статус, присланный сервером.
    if (response["is_read"].toInt() == 1) {
        incomingMsg.status = ChatMessage::Read;
    } else if (response["is_delivered"].toInt() == 1) {
        incomingMsg.status = ChatMessage::Delivered;
    } else {
        incomingMsg.status = ChatMessage::Sent;
    }

    // 2. Обновляем UI списка контактов, чтобы показать превью этого нового сообщения.
    updateContactItem(incomingMsg.fromUser);

    // 3. Добавляем сообщение в кэш соответствующего чата.
    QString chatPartner = incomingMsg.fromUser;
    if (m_chatHistoryCache.contains(chatPartner)) {
        m_chatHistoryCache[chatPartner].messages.append(incomingMsg);
    }

    // 4. Отправляем на сервер подтверждение о ДОСТАВКЕ.
    //    Это говорит серверу, что клиент получил сообщение.
    QJsonObject deliveredCmd;
    deliveredCmd["type"] = "message_delivered";
    deliveredCmd["id"] = (double)incomingMsg.id;
    sendJson(deliveredCmd);

    // 5. Логика отображения в зависимости от того, какой чат сейчас открыт.
    if (incomingMsg.fromUser == m_currentChatPartner.username) {
        // --- Сообщение пришло для ТЕКУЩЕГО открытого чата ---
        bool wasScrolledToBottom = m_chatViewWidget->isScrolledToBottom();

        // Добавляем сообщение в `ChatMessageModel`, что приводит к его отображению.
        m_chatModel->addMessage(incomingMsg);

        if (wasScrolledToBottom) {
            // Если пользователь и так был внизу, плавно прокручиваем к новому сообщению.
            QMetaObject::invokeMethod(m_chatViewWidget, "scrollToBottom", Qt::QueuedConnection);
        } else {
            // Если пользователь читал историю, эмитируем сигнал для показа кнопки "вниз".
            emit newMessageForCurrentChat();
        }

        // С небольшой задержкой запускаем проверку видимых сообщений,
        // чтобы отправить статус "прочитано" для этого нового сообщения.
        QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);
    } else {
        // --- Сообщение пришло для ДРУГОГО (неактивного) чата ---
        m_unreadCounts[incomingMsg.fromUser]++; // Увеличиваем счетчик непрочитанных.
        m_contactModel->refreshContact(incomingMsg.fromUser); // Просим модель перерисовать этот контакт.
    }

    // 6. Показываем системное уведомление (мигание иконки), если окно неактивно
    //    или сообщение пришло для другого чата.
    if (incomingMsg.fromUser != m_currentChatPartner.username || isMinimized() || !isActiveWindow()) {
        QApplication::alert(this);
    }
}

/**
 * @brief Обрабатывает широковещательное сообщение от сервера со списком онлайн-пользователей.
 *
 * @details Этот метод вызывается каждый раз, когда кто-то из пользователей
 *          подключается к серверу или отключается от него. Он получает
 *          актуальный список `username`'ов тех, кто в сети, и обновляет
 *          флаг `isOnline` в локальном кэше `m_userCache`.
 *
 * @param response JSON-объект, содержащий массив строк "users" с именами онлайн-пользователей.
 */
void MainWindow::handleUserList(const QJsonObject& response)
{
    QJsonArray onlineUsernamesArray = response["users"].toArray();

    // 1. Для быстрой проверки (O(1)) используем QSet.
    QSet<QString> onlineUsers;
    for (const QJsonValue &value : onlineUsernamesArray) {
        onlineUsers.insert(value.toString());
    }

    // 2. Проходимся по всему нашему кэшу пользователей.
    for (auto it = m_userCache.begin(); it != m_userCache.end(); ++it) {
        // Проверяем, есть ли `username` текущего пользователя в сете `onlineUsers`.
        if (onlineUsers.contains(it.value().username)) {
            it.value().isOnline = true; // Если да, устанавливаем флаг.
        } else {
            it.value().isOnline = false; // Если нет, сбрасываем флаг.
        }
    }

    // 3. После обновления данных в кэше, вызываем updateUserList(),
    //    который передаст актуальный список в ContactListModel,
    //    что приведет к перерисовке списка контактов с новыми статусами.
    updateUserList();
}

/**
 * @brief Обрабатывает уведомление от сервера о том, что сообщение было доставлено.
 *
 * @details Этот метод вызывается, когда получатель нашего сообщения получает его
 *          (но еще не обязательно прочитал). Это означает, что наше исходящее
 *          сообщение должно сменить статус с `Sent` на `Delivered`.
 *
 * @param response JSON-объект, содержащий "id" доставленного сообщения.
 */
void MainWindow::handleMessageDelivered(const QJsonObject& response)
{
    qint64 messageId = response["id"].toDouble();
    qDebug() << "[CLIENT] Message" << messageId << "was delivered.";

    // Вызываем вспомогательный метод, который обновит статус сообщения
    // как в глобальном кэше `m_chatHistoryCache`, так и в активной
    // модели `m_chatModel`, если это сообщение из текущего чата.
    updateMessageStatusInCacheAndModel(messageId, ChatMessage::Delivered);
}

/**
 * @brief Обрабатывает уведомление от сервера о том, что сообщение было прочитано.
 *
 * @details Этот метод вызывается, когда получатель нашего сообщения его прочитал
 *          (т.е. оно появилось у него на экране). Наше исходящее сообщение
 *          должно сменить свой статус на `Read`.
 *
 * @param response JSON-объект, содержащий "id" прочитанного сообщения.
 */
void MainWindow::handleMessageRead(const QJsonObject& response)
{
    qint64 messageId = response["id"].toDouble();
    qDebug() << "[CLIENT] Message" << messageId << "was read.";

    // Аналогично `handleMessageDelivered`, вызываем общий метод для обновления статуса.
    updateMessageStatusInCacheAndModel(messageId, ChatMessage::Read);
}

/**
 * @brief Обрабатывает команду от сервера об изменении текста сообщения.
 *
 * @details Этот метод вызывается, когда кто-то (мы с другого устройства или наш собеседник)
 *          отредактировал сообщение в одном из чатов.
 *
 * @param response JSON-объект, содержащий "id" сообщения, "payload" (новый текст)
 *                 и "with_user" (имя собеседника, в чате с которым произошло изменение).
 */
void MainWindow::handleEditMessage(const QJsonObject& response)
{
    QString chatPartner = response["with_user"].toString();
    qint64 messageId = response["id"].toDouble();
    QString newPayload = response["payload"].toString();

    qDebug() << "[CLIENT] Received command to edit message" << messageId;

    // 1. Обновляем сообщение в глобальном кэше.
    if (m_chatHistoryCache.contains(chatPartner)) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        // Ищем сообщение по ID...
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                // ...и обновляем его данные.
                messagesInCache[i].payload = newPayload;
                messagesInCache[i].isEdited = true;
                qDebug() << "[CACHE] Message" << messageId << "edited in cache for" << chatPartner;
                break;
            }
        }
    }

    // 2. Если измененное сообщение находится в ТЕКУЩЕМ открытом чате...
    if (chatPartner == m_currentChatPartner.username) {
        // ...вызываем метод `editMessage` у `m_chatModel`, чтобы
        // немедленно отобразить изменения на экране.
        m_chatModel->editMessage(messageId, newPayload);
    }

    // 3. Принудительно обновляем элемент в списке контактов.
    //    Это необходимо, чтобы обновилось превью последнего сообщения,
    //    если было отредактировано именно оно.
    updateContactItem(chatPartner);
}

/**
 * @brief Обрабатывает команду от сервера об удалении сообщения.
 * @details Этот метод вызывается, когда кто-то (мы с другого устройства или наш собеседник)
 *          удалил сообщение. Он удаляет сообщение из кэша и, если необходимо,
 *          из видимой на экране модели чата.
 * @param response JSON-объект, содержащий "id" удаленного сообщения и "with_user"
 *                 (имя собеседника в этом чате).
 */
void MainWindow::handleDeleteMessage(const QJsonObject& response){
    qint64 messageId = response["id"].toDouble();
    QString chatUser = response["with_user"].toString();

    // Определяем, для какого из наших чатов предназначена эта команда.
    // Сервер сообщает, кто был собеседником автора сообщения.
    // Если автором были мы, то chatPartner - это собеседник.
    // Если автором был собеседник, то chatPartner - это он и есть.
    QString chatPartner = (chatUser == m_currentUsername) ? m_currentChatPartner.username : chatUser;

    qDebug() << "[CLIENT] Received command to delete message" << messageId << "in chat with user" << chatPartner;

    // 1. Удаляем сообщение из глобального кэша.
    if (m_chatHistoryCache.contains(chatPartner)) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        // Ищем сообщение по ID...
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                messagesInCache.removeAt(i); // ...и удаляем его.
                qDebug() << "[CACHE] Message" << messageId << "deleted from cache for" << chatPartner;
                break;
            }
        }
    }

    // 2. Если удаленное сообщение находилось в ТЕКУЩЕМ открытом чате...
    if (chatPartner == m_currentChatPartner.username) {
        // ...вызываем метод `removeMessage` у `m_chatModel`, чтобы оно исчезло с экрана.
        m_chatModel->removeMessage(messageId);
    }

    // 3. Принудительно обновляем элемент в списке контактов.
    //    Это необходимо, чтобы обновилось превью последнего сообщения,
    //    если было удалено именно оно.
    updateContactItem(chatPartner);
}

/**
 * @brief Централизованно обновляет статус сообщения и в кэше, и в активной модели.
 * @details Этот вспомогательный метод ищет сообщение по `messageId` во всех
 *          кэшированных чатах. Найдя, он обновляет его статус и, если это сообщение
 *          из текущего открытого чата, также обновляет его в `m_chatModel` для
 *          немедленной перерисовки.
 * @param messageId ID сообщения, статус которого нужно обновить.
 * @param newStatus Новый статус из `ChatMessage::MessageStatus`.
 */
void MainWindow::updateMessageStatusInCacheAndModel(qint64 messageId, ChatMessage::MessageStatus newStatus)
{
    // Проходимся по всему кэшу чатов.
    for (auto it = m_chatHistoryCache.begin(); it != m_chatHistoryCache.end(); ++it) {
        QList<ChatMessage>& messages = it.value().messages;
        // Проходимся по всем сообщениям в одном чате.
        for (int i = 0; i < messages.size(); ++i) {
            if (messages[i].id == messageId) {
                // Нашли нужное сообщение.
                if (messages[i].status == newStatus) return; // Статус уже такой, ничего не делаем.

                // 1. Обновляем статус в "источнике правды" - кэше.
                messages[i].status = newStatus;
                QString foundInChatWith = it.key();
                qDebug() << "[CACHE] Status updated for message" << messageId << "in chat with" << foundInChatWith;

                // 2. Если этот чат сейчас открыт, обновляем и активную модель.
                if (foundInChatWith == m_currentChatPartner.username) {
                    // Используем setData для эффективного обновления, а не `updateMessageStatus`.
                    // Это заставит view перерисовать только один элемент.
                    QModelIndex modelIndex = m_chatModel->index(i, 0);
                    m_chatModel->setData(modelIndex, QVariant::fromValue(messages[i]), Qt::UserRole);
                }

                return; // Сообщение найдено и обработано, выходим из всех циклов.
            }
        }
    }
}

/**
 * @brief Обрабатывает ответ сервера с результатами глобального поиска пользователей.
 * @param response JSON-объект, содержащий массив "users" с найденными пользователями.
 */
void MainWindow::handleSearchResults(const QJsonObject& response)
{
    QJsonArray users = response["users"].toArray();

    // Позиционируем всплывающее окно точно под полем поиска.
    QWidget *searchBar = m_searchLineEdit;
    m_searchResultsPopup->move(searchBar->mapToGlobal(QPoint(0, searchBar->height())));
    m_searchResultsPopup->setFixedWidth(searchBar->width());

    // Передаем данные в виджет и показываем его.
    m_searchResultsPopup->showResults(users);

    // С помощью QTimer::singleShot(0, ...) возвращаем фокус ввода обратно
    // в поле поиска асинхронно, после того как popup будет полностью показан.
    QTimer::singleShot(0, this, [this]() {
        m_searchLineEdit->setFocus();
    });
}

/**
 * @brief Обрабатывает ответ сервера об успешной отправке запроса на добавление в контакты.
 * @details
 * Этот слот-обработчик является конечной точкой для сценария "Пользователь -> Найти -> Выбрать -> Подтвердить добавление".
 * Он вызывается, когда сервер подтверждает, что запрос на добавление в друзья был успешно
 * создан и либо доставлен другому пользователю онлайн, либо сохранен для доставки, когда тот войдет в сеть.
 *
 * Функция выполняет единственную, но важную задачу: информирует пользователя об успехе.
 * Это обеспечивает положительную обратную связь (positive feedback), давая пользователю
 * понять, что его действие было успешно обработано системой.
 * @param response JSON-объект ответа от сервера. Ожидается, что он будет содержать
 *                 поле `reason` с текстовым сообщением для пользователя, например,
 *                 "Запрос на добавление в контакты пользователю 'JohnDoe' успешно отправлен."
 */
void MainWindow::handleAddContactSuccess(const QJsonObject& response)
{
    // Используем статический метод QMessageBox::information для отображения простого
    // модального диалогового окна с иконкой "Информация".
    QMessageBox::information(this, "Успех", response["reason"].toString());
}

/**
 * @brief Обрабатывает ответ сервера об ошибке при отправке запроса на добавление в контакты.
 * @details
 * Этот слот-обработчик вызывается, если сервер по какой-либо причине не смог обработать
 * запрос на добавление в контакты. Причины могут быть разными:
 * - Пользователь уже находится в вашем списке контактов.
 * - Вы уже отправили этому пользователю запрос, и он ожидает ответа.
 * - Пользователя с таким `username` не существует (хотя это маловероятно при выборе из поиска).
 * - Произошла ошибка на стороне базы данных сервера.
 *
 * Задача этого метода — предоставить пользователю четкую и понятную обратную связь
 * о том, почему его действие не было выполнено.
 * @param response JSON-объект ответа от сервера. Обязательно должен содержать
 *                 поле `reason` с текстом, объясняющим причину ошибки.
 */
void MainWindow::handleAddContactFailure(const QJsonObject& response)
{
    // Используем статический метод QMessageBox::warning для отображения модального
    // диалогового окна с иконкой "Предупреждение".
    QMessageBox::warning(this, "Ошибка", response["reason"].toString());
}

/**
 * @brief Обрабатывает push-уведомление от сервера о новом запросе на добавление в контакты.
 * @details
 * Этот метод вызывается в реальном времени, когда другой пользователь отправляет
 * *вам* запрос на добавление в друзья. Его задача — немедленно уведомить вас
 * и предоставить возможность принять или отклонить этот запрос.
 *
 * Он не выполняет логику сам, а делегирует показ диалогового окна вспомогательному
 * приватному методу `showContactRequestPrompt`.
 * @param response JSON-объект ответа от сервера, содержащий:
 *                 - `fromUsername`: Уникальное имя пользователя, отправившего запрос.
 *                 - `fromDisplayname`: Его отображаемое имя.
 */
void MainWindow::handleIncomingContactRequest(const QJsonObject& response)
{
    // 1. Извлекаем данные отправителя из JSON.
    QString fromUsername = response["fromUsername"].toString();
    QString fromDisplayName = response["fromDisplayname"].toString();

    // 2. Вызываем приватный метод, который отвечает за отображение UI-диалога.
    showContactRequestPrompt(fromUsername, fromDisplayName);
}

/**
 * @brief Обрабатывает список ожидающих запросов в контакты, полученный при входе.
 * @details
 * Этот метод вызывается один раз после успешной аутентификации. Он обрабатывает
 * сценарий, когда вам отправили запросы в друзья, пока вы были офлайн. Сервер
 * присылает список всех таких "висящих" запросов.
 *
 * Метод итерируется по списку и для каждого запроса показывает диалог подтверждения,
 * позволяя пользователю последовательно разобрать все накопившиеся уведомления.
 * @param response JSON-объект ответа от сервера, содержащий массив `requests`,
 *                 где каждый элемент — это объект с `fromUsername` и `fromDisplayname`.
 */
void MainWindow::handlePendingRequestsList(const QJsonObject& response)
{
    QJsonArray requests = response["requests"].toArray();

    // Проходимся в цикле по каждому элементу в JSON-массиве.
    for (const QJsonValue &value : requests) {
        QJsonObject reqObj = value.toObject();
        QString fromUsername = reqObj["fromUsername"].toString();
        QString fromDisplayName = reqObj["fromDisplayname"].toString();

        // Для каждого запроса в списке мы повторно используем тот же самый
        // метод, что и для онлайн-уведомлений.
        showContactRequestPrompt(fromUsername, fromDisplayName);
    }
}


/**
 * @brief Обрабатывает успешный ответ на запрос выхода из аккаунта.
 * @details
 * Этот метод вызывается, когда сервер подтверждает, что сессия пользователя
 * была успешно завершена. Главная задача этого метода — полностью сбросить
 * состояние клиентского приложения, вернув его к исходному виду (экрану входа).
 * @param response JSON-объект ответа от сервера (в данном случае не используется).
 */
void MainWindow::handleLogoutSuccess(const QJsonObject& response)
{
    // Макрос, чтобы компилятор не выдавал предупреждение о неиспользуемом параметре.
    Q_UNUSED(response);

    qDebug() << "[CLIENT] Logout successful. Resetting application state.";

    // Показываем пользователю короткое сообщение об успешном выходе.
    QMessageBox::information(this, "Выход", "Вы успешно вышли из аккаунта.");

    // Вызываем приватный метод, который выполняет всю "грязную работу"
    // по очистке кэшей, моделей, сбросу переменных и переключению UI.
    resetApplicationState();
}

/**
 * @brief Сбрасывает все состояние приложения к начальному (как при запуске).
 * @details Этот метод вызывается после успешного выхода из аккаунта. Он очищает
 *          все кэши, модели, сбрасывает текущего пользователя и переключает
 *          интерфейс обратно на экран входа.
 */
void MainWindow::resetApplicationState()
{
    // Сброс текущего состояния.
    m_currentUsername.clear();
    m_currentChatPartner = User();

    // Очистка всех кэшей и моделей.
    m_chatHistoryCache.clear();
    m_userCache.clear();
    m_chatModel->clearMessages();
    m_contactModel->clear();
    m_unreadCounts.clear();

    // Остановка и удаление всех активных таймеров.
    qDeleteAll(m_typingReceiveTimers);
    m_typingReceiveTimers.clear();

    // Скрытие вспомогательных виджетов.
    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }

    // Сброс флагов и состояний.
    m_oldestMessageId = 0;
    m_isLoadingHistory = false;

    // Возврат к экрану входа.
    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);
}

/**
 * @brief Обрабатывает неудачный ответ на запрос выхода из аккаунта.
 * @details Этот слот-обработчик вызывается в редких случаях, когда сервер
 *          по какой-то причине не смог корректно завершить сессию пользователя.
 *          Он просто информирует пользователя об ошибке.
 * @param response JSON-объект ответа от сервера, содержащий поле `reason` с текстом ошибки.
 */
void MainWindow::handleLogoutFailure(const QJsonObject& response)
{
    // Показываем стандартное окно с предупреждением.
    QMessageBox::warning(this, "Ошибка", response["reason"].toString());
}

/**
 * @brief Обрабатывает запрос на вход от `LoginWidget`.
 * @details Этот слот вызывается, когда пользователь вводит свои данные на экране
 *          входа и нажимает кнопку "Войти". Метод выполняет базовую проверку
 *          введенных данных на клиенте, после чего формирует и отправляет
 *          JSON-запрос на аутентификацию на сервер.
 * @param username Имя пользователя, введенное в `QLineEdit`.
 * @param password Пароль, введенный в `QLineEdit`.
 */
void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    // 1. Клиентская валидация: проверяем, что поля не пустые.
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя пользователя и пароль не могут быть пустыми.");
        return; // Прерываем выполнение, если проверка не пройдена.
    }

    qDebug() << "[CLIENT] MainWindow: Login requested for user:" << username;

    // 2. Формируем JSON-объект для отправки на сервер.
    QJsonObject loginRequest;
    loginRequest["type"] = "login"; // Обязательное поле типа команды.
    loginRequest["username"] = username;
    loginRequest["password"] = password;

    // 3. Отправляем запрос через централизованный метод `sendJson`.
    sendJson(loginRequest);
}

/**
 * @brief Обрабатывает запрос на регистрацию от `LoginWidget`.
 *
 * @details Этот слот вызывается, когда пользователь заполняет форму регистрации
 *          и нажимает кнопку "Зарегистрироваться". Метод выполняет базовую
 *          клиентскую валидацию, после чего формирует и отправляет JSON-запрос
 *          на создание нового аккаунта на сервер.
 *
 * @param username Желаемое имя пользователя (логин).
 * @param displayName Желаемое отображаемое имя.
 * @param password Желаемый пароль.
 */
void MainWindow::onRegisterRequested(const QString& username, const QString& displayName, const QString& password)
{
    // 1. Клиентская валидация: проверяем, что все поля заполнены.
    if (username.isEmpty() || password.isEmpty() || displayName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены.");
        return;
    }

    qDebug() << "[CLIENT] MainWindow: Register requested for user:" << username;

    // 2. Формируем JSON-объект для отправки на сервер.
    QJsonObject registerRequest;
    registerRequest["type"] = "register";
    registerRequest["username"] = username;
    registerRequest["password"] = password;
    registerRequest["display_name"] = displayName;

    // 3. Отправляем запрос.
    sendJson(registerRequest);
}

/**
 * @brief Обрабатывает запрос на отправку сообщения из `ChatViewWidget`.
 * @details Это центральный метод для отправки сообщений. Он имеет две основные ветки логики:
 *          - Если установлен флаг `m_editingMessageId`, он отправляет команду на редактирование.
 *          - В противном случае, он создает новое сообщение (с учетом возможного ответа),
 *            добавляет его в модель для мгновенного отображения ("оптимистичное обновление")
 *            и отправляет на сервер.
 * @param text Текст сообщения из поля ввода `QTextEdit`.
 */
void MainWindow::onSendMessageRequested(const QString& text)
{
    // 1. Базовые проверки: не отправляем пустые сообщения или если чат не выбран.
    if (text.isEmpty() || m_currentChatPartner.username.isEmpty()) {
        return;
    }

    // --- Ветка 1: Редактирование существующего сообщения ---
    if (m_editingMessageId > 0) {
        qDebug() << "[CLIENT] Sending 'edit_message' request for ID:" << m_editingMessageId;

        // Формируем запрос на редактирование.
        QJsonObject request;
        request["type"] = "edit_message";
        request["id"] = m_editingMessageId;
        request["payload"] = text;
        sendJson(request);

        // Сбрасываем состояние редактирования.
        m_editingMessageId = 0;
        m_chatViewWidget->setEditMode(false); // Возвращаем UI в обычный режим.

    } else {
        // --- Ветка 2: Отправка нового сообщения ---

        // 1. Создаем локальный объект сообщения.
        ChatMessage msg;
        msg.fromUser = m_currentUsername;
        msg.toUser = m_currentChatPartner.username;
        msg.payload = text;
        msg.status = ChatMessage::Sending; // Начальный статус - "Отправка".
        msg.isOutgoing = true;
        msg.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate); // Временная метка клиента.
        msg.tempId = QUuid::createUuid().toString(QUuid::WithoutBraces); // Уникальный временный ID.
        msg.replyToId = m_replyToMessageId; // Добавляем ID цитируемого сообщения, если есть.

        // 2. "Оптимистичное обновление": немедленно добавляем сообщение в модель,
        //    чтобы пользователь сразу увидел его на экране со статусом "Отправка".
        m_chatModel->addMessage(msg);
        m_chatViewWidget->chatHistoryView()->scrollToBottom();

        // 3. Также добавляем временное сообщение в глобальный кэш.
        QString chatPartner = m_currentChatPartner.username;
        if (m_chatHistoryCache.contains(chatPartner)) {
            m_chatHistoryCache[chatPartner].messages.append(msg);
        }

        // 4. Формируем и отправляем JSON-запрос на сервер.
        QJsonObject request;
        request["type"] = "private_message";
        request["fromUser"] = msg.fromUser;
        request["toUser"] = msg.toUser;
        request["payload"] = msg.payload;
        request["reply_to_id"] = msg.replyToId;
        request["temp_id"] = msg.tempId; // Отправляем `tempId`, чтобы сервер вернул его в echo-ответе.
        sendJson(request);

        // 5. Если это был ответ, сбрасываем состояние ответа.
        if (m_replyToMessageId > 0) {
            m_replyToMessageId = 0;
            m_chatViewWidget->hideReplyUI();
        }
    }

    // В любом случае (новое сообщение или отредактированное), обновляем
    // превью последнего сообщения в списке контактов.
    updateContactItem(m_currentChatPartner.username);
}

/**
 * @brief Обрабатывает клик по кнопке "Выйти".
 * @details Формирует и отправляет на сервер JSON-запрос на выход из аккаунта.
 */
void MainWindow::onLogoutButtonClicked()
{
    qDebug() << "[CLIENT] MainWindow: Logout button clicked.";

    QJsonObject logoutRequest;
    logoutRequest["type"] = "logout_request";
    logoutRequest["username"] = m_currentUsername; // Отправляем имя, чтобы сервер знал, чью сессию завершить.
    sendJson(logoutRequest);
}

/**
 * @brief Слот, обрабатывающий выбор нового контакта (чата) в списке.
 * @details Этот метод является центральной точкой для логики переключения чатов.
 *          Он выполняет следующие действия:
 *          1.  Сбрасывает состояния (ответ на сообщение, счетчик непрочитанных).
 *          2.  Устанавливает нового "собеседника" (`m_currentChatPartner`).
 *          3.  Обновляет UI (заголовок чата, переключает на виджет чата).
 *          4.  Проверяет наличие истории чата в локальном кэше.
 *          5.  Если история в кэше есть - загружает ее мгновенно.
 *          6.  Если истории нет - отправляет запрос на сервер.
 * @param current Индекс (`QModelIndex`) нового выбранного элемента в `ContactListModel`.
 */
void MainWindow::onUserSelectionChanged(const QModelIndex &current)
{
    qDebug() << "--- onUserSelectionChanged START ---";

    // --- 1. Очистка и подготовка состояния ---

    // (Заготовка) Если бы был активен поиск по чату, его нужно было бы сбросить.
    if (m_isChatSearchActive) {
    }
    // Если пользователь был в режиме ответа на сообщение, отменяем его.
    if (m_replyToMessageId > 0) {
        m_replyToMessageId = 0;
        m_chatViewWidget->hideReplyUI();
    }

    // Если `current` невалидный (например, было снято выделение),
    // показываем заглушку и сбрасываем текущего собеседника.
    if (!current.isValid()) {
        qDebug() << "Current item is null, resetting view.";
        m_rightSideLayout->setCurrentWidget(m_placeholderWidget);
        m_currentChatPartner = User();
        qDebug() << "--- onUserSelectionChanged END (reset) ---";
        return;
    }

    // --- 2. Получение данных о выбранном пользователе ---
    QString selectedUsername = current.data(ContactListModel::UsernameRole).toString();
    qDebug() << "Selected username from model:" << selectedUsername;

    // --- 3. Обновление счетчика непрочитанных ---
    // Если для этого чата были непрочитанные сообщения, сбрасываем счетчик.
    if (m_unreadCounts.value(selectedUsername, 0) > 0) {
        m_unreadCounts[selectedUsername] = 0;
        // Уведомляем модель, что данные для этого элемента изменились (чтобы "бейдж" исчез).
        m_contactModel->dataChanged(current, current, {ContactListModel::UnreadCountRole});
    }

    // --- 4. Установка нового активного собеседника ---
    if (selectedUsername.isEmpty() || !m_userCache.contains(selectedUsername)) {
        qWarning() << "CRITICAL: Selected user not found in cache or username is empty!";
        return;
    }
    m_currentChatPartner = m_userCache.value(selectedUsername);
    qDebug() << "Current chat partner set to:" << m_currentChatPartner.displayName;

    // Принудительно обновляем весь список контактов, чтобы делегат мог правильно
    // определить, какой чат является `m_currentChatPartner` (и, например, не показывать для него "печатает...").
    updateUserList();

    // --- 5. Подготовка к загрузке истории ---
    m_isLoadingHistory = true; // Устанавливаем флаг, чтобы избежать запросов во время загрузки.
    m_oldestMessageId = -1;    // Сбрасываем ID для пагинации.
    qDebug() << "Pagination state reset.";

    // --- 6. Обновление UI ---
    if (!m_chatViewWidget || !m_chatModel || !m_rightSideLayout) {
        qWarning() << "CRITICAL: A core widget is a nullptr!";
        return;
    }

    // Обновляем заголовок (имя, статус) в виджете чата.
    m_chatViewWidget->updateHeader(m_currentChatPartner);
    // Очищаем модель сообщений от предыдущего чата.
    m_chatModel->clearMessages();
    // Переключаем `QStackedLayout`, чтобы показать виджет чата вместо заглушки.
    m_rightSideLayout->setCurrentWidget(m_chatViewWidget);
    qDebug() << "Switched to ChatViewWidget.";

    // --- 7. Загрузка истории сообщений (из кэша или с сервера) ---
    if (m_chatHistoryCache.contains(selectedUsername)) {
        // [CACHE HIT] - История для этого чата уже есть в кэше.
        qDebug() << "[CACHE] Hit for user:" << selectedUsername << ". Loading from memory.";
        const ChatCache& cache = m_chatHistoryCache.value(selectedUsername);

        // Загружаем сообщения из кэша напрямую в модель.
        m_chatModel->addMessages(cache.messages);
        m_oldestMessageId = cache.oldestMessageId;
        m_isLoadingHistory = false; // Загрузка завершена.

        // Асинхронно прокручиваем в конец списка.
        QMetaObject::invokeMethod(m_chatViewWidget->chatHistoryView(), "scrollToBottom", Qt::QueuedConnection);

    } else {
        // [CACHE MISS] - Истории в кэше нет, нужно запросить у сервера.
        qDebug() << "[CACHE] Miss for user:" << selectedUsername << ". Requesting from server.";

        // Формируем и отправляем JSON-запрос на получение истории.
        QJsonObject request;
        request["type"] = "get_history";
        request["with_user"] = m_currentChatPartner.username;
        sendJson(request);
    }

    // --- 8. Финальные действия ---
    // С небольшой задержкой (чтобы UI успел обновиться) запускаем проверку видимых
    // сообщений, чтобы отправить на сервер статусы "прочитано" для тех,
    // что сразу попали в область видимости.
    QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);
    qDebug() << "--- onUserSelectionChanged END (success) ---";
}

/**
 * @brief (ЗАГОТОВКА) Показывает профиль текущего собеседника.
 * @details На данный момент является заглушкой, которая показывает QMessageBox
 *          с именем пользователя. В будущем здесь может быть реализовано
 *          открытие отдельного окна или боковой панели с детальной информацией.
 */
void MainWindow::showProfileView()
{
    qDebug() << "[CLIENT] Showing profile for" << m_currentChatPartner.username;
    QMessageBox::information(this, "Профиль", "Здесь будет показан профиль пользователя " + m_currentChatPartner.displayName);
}

/**
 * @brief Возвращает объект User из кэша по имени пользователя.
 * @param username Имя пользователя для поиска.
 * @return User Копия объекта User. Если пользователь не найден, вернется пустой объект User.
 */
User MainWindow::getUserFromCache(const QString& username) const
{
    return m_userCache.value(username);
}

/**
 * @brief Возвращает константную ссылку на кэш истории чатов.
 * @details Используется `ContactListModel`, чтобы получить доступ к последним сообщениям
 *          для отображения превью в списке контактов.
 * @return const QMap<QString, ChatCache>& Ссылка на кэш.
 */
const QMap<QString, ChatCache>& MainWindow::getChatCache() const
{
    return m_chatHistoryCache;
}

/**
 * @brief Возвращает константную ссылку на кэш счетчиков непрочитанных сообщений.
 * @details Используется `ContactListModel` для отображения "бейджей" с количеством
 *          непрочитанных сообщений.
 * @return const QMap<QString, int>& Ссылка на кэш.
 */
const QMap<QString, int>& MainWindow::getUnreadCounts() const
{
    return m_unreadCounts;
}

/**
 * @brief Возвращает данные о текущем собеседнике.
 * @details Используется `ContactListModel`, чтобы определить, какой чат является
 *          активным в данный момент (например, чтобы не показывать для него "печатает...").
 * @return User Копия объекта User текущего собеседника.
 */
User MainWindow::getCurrentChatPartner() const
{
    return m_currentChatPartner;
}

/**
 * @brief Показывает пользователю модальный диалог с запросом на добавление в контакты.
 * @details Этот метод создает QMessageBox с кнопками "Да" и "Нет", позволяя
 *          пользователю принять или отклонить запрос. После выбора пользователя
 *          формирует и отправляет на сервер JSON-ответ с принятым решением.
 * @param fromUsername Уникальное имя пользователя, отправившего запрос.
 * @param fromDisplayName Его отображаемое имя.
 */
void MainWindow::showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName)
{
    // Формируем текст вопроса с использованием `arg()` для безопасной подстановки.
    QString questionText = QString("Пользователь %1 (@%2) хочет добавить вас в список контактов. Принять запрос?")
                               .arg(fromDisplayName, fromUsername);

    // Показываем диалог и ждем, пока пользователь нажмет кнопку.
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Запрос на добавление в контакты", questionText,
                                  QMessageBox::Yes | QMessageBox::No);

    // Формируем JSON-объект для ответа серверу.
    QJsonObject contactResponse;
    contactResponse["type"] = "contact_request_response";
    contactResponse["fromUsername"] = fromUsername; // Указываем, на чей запрос мы отвечаем.

    // В зависимости от выбора пользователя, устанавливаем значение поля "response".
    if (reply == QMessageBox::Yes) {
        contactResponse["response"] = "accepted";
    } else {
        contactResponse["response"] = "declined";
    }

    // Отправляем ответ на сервер.
    sendJson(contactResponse);
}

/**
 * @brief Обновляет модель списка контактов на основе данных из `m_userCache`.
 * @details Этот метод является центральной точкой для обновления UI списка контактов.
 *          Он извлекает всех пользователей из кэша, сортирует их по отображаемому
 *          имени и передает отсортированный список `username`'ов в `ContactListModel`
 *          для "умного" и эффективного обновления представления.
 */
void MainWindow::updateUserList()
{
    // 1. Получаем список всех пользователей из кэша.
    QList<User> users = m_userCache.values();

    // 2. Сортируем пользователей по отображаемому имени (без учета регистра).
    std::sort(users.begin(), users.end(), [](const User& a, const User& b) {
        return a.displayName.toLower() < b.displayName.toLower();
    });

    // 3. Создаем `QStringList` только из имен пользователей (`username`) в уже отсортированном порядке.
    QStringList usernames;
    for (const User& user : users) {
        usernames.append(user.username);
    }

    // 4. Передаем этот отсортированный список в модель. Модель сама определит,
    //    какие элементы нужно добавить, удалить или переместить, и сделает это
    //    максимально эффективно, не сбрасывая выделение.
    m_contactModel->updateContacts(usernames);
}

/**
 * @brief Инициирует отправку запроса на глобальный поиск пользователей.
 * @details Этот слот вызывается по таймауту `m_globalSearchTimer`, то есть
 *          спустя короткое время после того, как пользователь прекратил ввод
 *          текста в `m_searchLineEdit`.
 */
void MainWindow::onGlobalSearchTriggered()
{
    QString query = m_searchLineEdit->text().trimmed();

    // Если поле поиска пустое, скрываем результаты и выходим.
    if (query.isEmpty()) {
        if (m_searchResultsPopup && m_searchResultsPopup->isVisible()) {
            m_searchResultsPopup->hide();
        }
        return;
    }

    qDebug() << "[CLIENT] Triggered global user search for:" << query;

    // Формируем и отправляем поисковый запрос на сервер.
    QJsonObject request;
    request["type"] = "search_users";
    request["term"] = query;
    sendJson(request);
}

/**
 * @brief Перехватывает события, предназначенные для других объектов в приложении.
 * @details Используется для реализации закрытия всплывающего окна `m_searchResultsPopup`
 *          при клике мышью в любом другом месте приложения.
 * @param watched Объект, получивший событие.
 * @param event Само событие.
 * @return `true` если событие было обработано, иначе `false`.
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Если произошло нажатие кнопки мыши, и при этом наше всплывающее окно видимо...
    if (event->type() == QEvent::MouseButtonPress && m_searchResultsPopup->isVisible()) {
        // ...и если курсор мыши в момент клика НЕ находится внутри геометрии всплывающего окна...
        if (!m_searchResultsPopup->geometry().contains(QCursor::pos())) {
            // ...то мы скрываем это окно.
            m_searchResultsPopup->hide();
        }
    }

    // Передаем событие на дальнейшую обработку в базовый класс.
    return QMainWindow::eventFilter(watched, event);
}

/**
 * @brief Обрабатывает прокрутку списка сообщений.
 * @details Основная задача этого слота — определить, когда пользователь
 *          достиг самого верха истории, и инициировать подгрузку более
 *          старых сообщений (пагинацию).
 * @param value Текущее значение позиции скроллбара (0 - в самом верху).
 */
void MainWindow::onChatScroll(int value) {
    // Не выполняем никаких действий, если активен поиск по чату или уже идет загрузка.
    if (m_isChatSearchActive) return;
    if (m_isLoadingHistory) return;

    // Главное условие:
    // 1. `value == 0` - скроллбар находится в самой верхней позиции.
    // 2. `!m_isLoadingHistory` - в данный момент не идет загрузка.
    // 3. `m_oldestMessageId != 0` - есть, что еще загружать (0 означает конец истории).
    if (value == 0 && !m_isLoadingHistory && m_oldestMessageId != 0) {
        qDebug() << "[CLIENT] Scrolled to top. Requesting older history before ID:" << m_oldestMessageId;

        m_isLoadingHistory = true; // Устанавливаем флаг, чтобы предотвратить повторные запросы.

        // Формируем и отправляем запрос на получение следующей "порции" истории.
        QJsonObject request;
        request["type"] = "get_history";
        request["with_user"] = m_currentChatPartner.username;
        request["before_id"] = m_oldestMessageId; // Указываем, что нам нужны сообщения СТАРШЕ этого ID.
        sendJson(request);
    }
}

/**
 * @brief Обрабатывает ответ сервера со старой порцией истории сообщений.
 * @details Этот метод вызывается в результате запроса, инициированного в `onChatScroll`,
 *          когда пользователь докрутил список до самого верха. Он получает
 *          предыдущую "порцию" сообщений, добавляет их в начало модели и кэша,
 *          а затем хитро восстанавливает позицию скроллбара так, чтобы
 *          пользователь остался на том же месте, где был, и не было "прыжка".
 * @param response JSON-объект ответа от сервера, содержащий массив "history"
 *                 и поле "with_user".
 */
void MainWindow::handleOldHistoryData(const QJsonObject& response)
{
    // 1. Проверяем, что история пришла для текущего открытого чата.
    QString historyForUser = response["with_user"].toString();
    if (historyForUser != m_currentChatPartner.username) {
        m_isLoadingHistory = false; // Сбрасываем флаг и выходим.
        return;
    }

    QJsonArray history = response["history"].toArray();
    // Если сервер вернул пустой массив, это значит, что мы достигли начала
    // всей истории переписки.
    if (history.isEmpty()) {
        m_oldestMessageId = 0; // 0 - это наш флаг, означающий "больше загружать нечего".
        m_isLoadingHistory = false;
        m_chatHistoryCache[historyForUser].allMessagesLoaded = true; // Отмечаем в кэше.
        return;
    }
    qDebug() << "[CLIENT] Prepending" << history.count() << "older messages for" << historyForUser;

    // --- 2. Получаем текущее состояние скроллбара ДО изменений ---
    QListView* chatView = m_chatViewWidget->chatHistoryView();
    QScrollBar* scrollBar = chatView->verticalScrollBar();
    // Запоминаем максимальное значение высоты контента.
    int oldScrollMax = scrollBar->maximum();

    // --- 3. Парсинг и обработка сообщений ---
    // (Этот блок идентичен `handleHistoryData`)
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
        messages.append(msg);
    }

    // --- 4. Обновление кэша и модели ---
    ChatCache& cache = m_chatHistoryCache[historyForUser];

    // Добавляем новые (старые) сообщения в НАЧАЛО списка в кэше.
    for (int i = messages.count() - 1; i >= 0; --i) {
        cache.messages.prepend(messages.at(i));
    }

    // Обновляем ID самого старого сообщения в кэше.
    if (!history.isEmpty()) {
        cache.oldestMessageId = history.first().toObject()["id"].toDouble();
    }

    // Добавляем сообщения в НАЧАЛО видимой модели.
    m_chatModel->prependMessages(messages);
    // Обновляем ID самого старого сообщения в `MainWindow`.
    m_oldestMessageId = cache.oldestMessageId;
    qDebug() << "New oldest message ID is:" << m_oldestMessageId;

    // --- 5. "Магия" восстановления позиции скроллбара ---

    // `QApplication::processEvents()` - это небольшой "хак". Мы просим Qt
    // немедленно обработать все отложенные события, включая изменения в модели
    // и перерисовку QListView. Это гарантирует, что скроллбар обновит свое
    // максимальное значение (`maximum`) ДО того, как мы попытаемся его установить.
    QApplication::processEvents();

    // Получаем новое максимальное значение скроллбара после добавления элементов.
    int newScrollMax = scrollBar->maximum();

    // Устанавливаем новую позицию. Разница `newScrollMax - oldScrollMax` - это
    // в точности высота добавленных сверху сообщений. Таким образом, мы "проталкиваем"
    // скроллбар вниз на высоту нового контента, и для пользователя видимая область
    // остается на том же самом сообщении, на котором он был до подгрузки.
    scrollBar->setValue(newScrollMax - oldScrollMax);

    QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);

    // Сбрасываем флаг, разрешая следующие подгрузки.
    m_isLoadingHistory = false;
}

/**
 * @brief Обновляет отображение одного конкретного контакта в списке.
 * @details Этот вспомогательный метод является оберткой над `ContactListModel::refreshContact`.
 *          Он вызывается всякий раз, когда изменяется какое-либо из данных контакта
 *          (статус онлайн, пришло новое сообщение, начал/закончил печатать),
 *          чтобы инициировать его перерисовку в `QListView`.
 * @param username Имя пользователя, элемент которого нужно обновить.
 */
void MainWindow::updateContactItem(const QString& username)
{
    // Просто вызываем соответствующий метод у нашей модели контактов.
    m_contactModel->refreshContact(username);
}

/**
 * @brief Обрабатывает уведомление от сервера о том, что пользователь начал печатать.
 * @details Этот метод вызывается, когда собеседник начинает вводить текст. Он обновляет
 *          состояние пользователя в кэше (`isTyping = true`) и запускает таймер,
 *          который по истечении времени сбросит этот статус. Также он немедленно
 *          обновляет UI, чтобы показать "печатает..." либо в заголовке текущего чата,
 *          либо в списке контактов.
 * @param response JSON-объект, содержащий `fromUser` - имя пользователя, который печатает.
 */
void MainWindow::handleTypingResponse(const QJsonObject& response)
{
    QString fromUser = response["fromUser"].toString();

    // Проверка, что такой пользователь вообще есть в нашем списке контактов.
    if (!m_userCache.contains(fromUser)) return;

    // 1. Обновляем "источник правды" - кэш пользователей.
    m_userCache[fromUser].isTyping = true;

    // 2. Уведомляем модель контактов, что данные этого пользователя изменились.
    //    Делегат, увидев флаг `isTyping`, отрисует "печатает..." в списке.
    m_contactModel->refreshContact(fromUser);

    // 3. Если уведомление пришло от текущего собеседника...
    if (fromUser == m_currentChatPartner.username) {
        // ...обновляем `m_currentChatPartner` и просим `ChatViewWidget` перерисовать заголовок.
        m_currentChatPartner = m_userCache[fromUser];
        m_chatViewWidget->updateHeader(m_currentChatPartner);
    }

    // --- 4. Управление таймером сброса статуса ---
    // Если для этого пользователя еще нет таймера, создаем его.
    if (!m_typingReceiveTimers.contains(fromUser)) {
        m_typingReceiveTimers[fromUser] = new QTimer(this);
        m_typingReceiveTimers[fromUser]->setInterval(2000); // Срабатывает через 2 секунды.
        m_typingReceiveTimers[fromUser]->setSingleShot(true); // Срабатывает только один раз.

        // Соединяем сигнал таймера `timeout` с лямбда-функцией, которая сбросит статус.
        connect(m_typingReceiveTimers[fromUser], &QTimer::timeout, this, [this, fromUser](){
            if (m_userCache.contains(fromUser)) {
                // Сбрасываем флаг в кэше.
                m_userCache[fromUser].isTyping = false;

                // Обновляем UI, как и при установке флага.
                if (fromUser == m_currentChatPartner.username) {
                    m_chatViewWidget->updateHeader(m_userCache.value(fromUser));
                }
                m_contactModel->refreshContact(fromUser);
            }
        });
    }

    // (Пере)запускаем таймер. Если в течение 2 секунд придет новое уведомление "печатает",
    // `start()` просто сбросит старый таймер и запустит его заново.
    m_typingReceiveTimers[fromUser]->start();
}

/**
 * @brief Отправляет на сервер подтверждение о прочтении сообщения.
 * @details Этот слот вызывается сигналом `messageNeedsReadReceipt` от `ChatMessageModel`.
 *          Сигнал, в свою очередь, испускается, когда `markMessageAsRead` успешно
 *          меняет статус сообщения с `Delivered` на `Read`.
 * @param messageId ID сообщения, которое было прочитано.
 */
void MainWindow::onSendMessageReadReceipt(qint64 messageId)
{
    qDebug() << "[CLIENT] Received receipt signal for message ID:" << messageId << ". Sending to server.";

    // Формируем JSON-запрос.
    QJsonObject readCmd;
    readCmd["type"] = "message_read";
    readCmd["id"] = (double)messageId;

    // Обновляем статус сообщения в кэше и модели (хотя модель это уже сделала сама).
    // Повторный вызов здесь служит для гарантии консистентности, но может быть избыточен.
    updateMessageStatusInCacheAndModel(messageId, ChatMessage::MessageStatus::Read);

    // Отправляем запрос на сервер.
    sendJson(readCmd);
}

/**
 * @brief Деструктор главного окна.
 */
MainWindow::~MainWindow()
{
    // Qt автоматически удалит все дочерние объекты (виджеты, модели, таймеры),
    // для которых `this` был указан в качестве родителя при создании.
    // Нам нужно вручную удалить только объект `ui`, который создается
    // `setupUi` и не имеет родителя.
    delete ui;
}

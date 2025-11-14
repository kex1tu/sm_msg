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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QKeySequence>
#include <QToolButton>
#include <QFile>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

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
#include "profileviewwidget.h"
#include "chatmessagedelegate.h"
#include "smoothlistview.h"
#include "dataservice.h"
#include "incomingrequestswidget.h"
#include "callhistorywidget.h"


DataService* MainWindow::m_dataService = nullptr;

MainWindow::MainWindow(DataService* dataService, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    qDebug()<<"[CLIENT] creating mainwindow";

    // Инициализация интерфейса
    ui->setupUi(this);

    // Быстрая клавиша для fullscreen (F11)
    QShortcut *fullScreenShortcut = new QShortcut(this);
    fullScreenShortcut->setKey(QKeySequence(Qt::Key_F11));
    connect(fullScreenShortcut, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    // DI: передаём DataService и создаём остальные сервисы
    m_dataService = dataService;
    m_networkService = new NetworkService(this);

    // Сервис и виджет для звонков (audio/video)
    m_callService = new CallService(m_networkService, m_dataService, this);
    m_callWidget = new CallWidget(m_callService, this);
    m_callWidget->hide();
    qDebug() << "[MainWindow] CallWidget created:" << (m_callWidget ? "OK" : "FAILED");

    // Модель сообщений чата и фильтр поиска по истории
    m_chatModel = new ChatMessageModel(this);
    m_chatFilterProxy = new ChatFilterProxyModel(this);
    m_chatFilterProxy->setSourceModel(m_chatModel);
    m_chatFilterProxy->setFilterRole(Qt::UserRole);
    m_chatFilterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // Сборка главного интерфейса, инициализация rootStackedWidget
    buildMainUI();
    ui->rootStackedWidget->addWidget(m_loginWidget);
    ui->rootStackedWidget->addWidget(m_mainChatWidget);
    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);

    // Popup для результатов поиска (быстрый поиск пользователей)
    m_searchResultsPopup = new SearchResultsPopup(this);

    // Глобальный EventFilter (может использоваться для хоткеев, специальных действий)
    qApp->installEventFilter(this);

    // Раздел "Меню", настройки страницы внутри приложения
    setupMenuPage();

    // Связь между сервисом данных и виджетом истории звонков
    qDebug() << "[MainWindow] setupConnections() completed";
    connect(m_dataService, &DataService::callHistoryReceived,
            m_callHistoryWidget, &CallHistoryWidget::setCallHistory);

    // Подключение всех пользовательских слотов/виджетов
    setupConnections();

    // Чтение конфигурации (IP сервера) из config.ini
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    qDebug() << "Current path:" << QDir::currentPath();
    qDebug() << "INI keys =" << settings.allKeys();
    QString serv_Ip = settings.value("network/servIp", "192.168.0.101").toString();

    // Подключение к серверу через NetworkService
    m_networkService->connectToServer(serv_Ip, 1234);
}


void MainWindow::setupMenuPage()
{
    // Создаём отдельный QWidget для меню (будет вкладываться в stack/layout панели)
    m_menuPage = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(m_menuPage);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Header меню: стрелка "назад" + заголовок "Menu"
    QHBoxLayout* headerLayout = new QHBoxLayout;
    QPushButton* backBtn = new QPushButton("← Back");
    backBtn->setFixedWidth(80);
    backBtn->setStyleSheet(
        "QPushButton { background-color: #f0f0f0; border: none; padding: 8px; }"
        "QPushButton:hover { background-color: #e0e0e0; }"
        );
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackFromMenu);
    QLabel* titleLabel = new QLabel("Menu");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    // Разделитель (горизонтальная линия)
    QFrame* separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #e0e0e0;");
    layout->addWidget(separator);

    // Layout для элементов меню, с отступами и отступом между кнопками
    QVBoxLayout* menuItemsLayout = new QVBoxLayout;
    menuItemsLayout->setSpacing(5);
    menuItemsLayout->setContentsMargins(10, 10, 10, 10);

    // Кнопка: Контакты (просто debug)
    QPushButton* contactsBtn = createMenuButton("Contacts", "");
    connect(contactsBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "[MENU] Contacts clicked";
    });
    menuItemsLayout->addWidget(contactsBtn);

    // Кнопка: Звонки
    QPushButton* callsBtn = createMenuButton("Calls", "New");
    connect(callsBtn, &QPushButton::clicked, this, &MainWindow::onCallsButtonClicked);
    menuItemsLayout->addWidget(callsBtn);

    // Кнопка: Incoming Requests — переключает правую панель на виджет с запросами
    QPushButton* incomingrequestsBtn = createMenuButton("incomingrequests", "");
    menuItemsLayout->addWidget(incomingrequestsBtn);
    connect(incomingrequestsBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "[DEBUG] Showing Incoming Requests";
        onUserSelectionChanged(QModelIndex());
        m_rightSideStackedLayout->setCurrentWidget(m_incomingRequestsWidget);
    });

    // Кнопка: Мой профиль (вызывает слот профиля)
    QPushButton* myProfileBtn = createMenuButton("my profile", "");
    menuItemsLayout->addWidget(myProfileBtn);
    connect(myProfileBtn, &QPushButton::clicked, this, onMyProfileClicked);

    layout->addLayout(menuItemsLayout);
    layout->addStretch();

    // Кнопка выхода (logout)
    m_logoutButton = new QPushButton("Выйти");
    m_logoutButton->setObjectName("logoutButton");
    layout->addWidget(m_logoutButton);

    // Финальные отступы
    layout->setContentsMargins(10, 0, 10, 10);

    // Главный layout монтируется в виджет меню
    m_menuPage->setLayout(layout);

    // Добавляем меню в левую панель приложения (например, QStackedLayout или QVBoxLayout)
    m_leftMainPanel->addWidget(m_menuPage);
}


void MainWindow::onMyProfileClicked() {

    // Получаем текущего пользователя из DataService (указатель на User)
    User* user = m_dataService->getCurrentUser();
    if (!user) {
        // Если почему-то пользователь не найден — логируем и ничего не переключаем
        qWarning() << "[PROFILE] User not found in cache:" << user->username;
        return;
    }

    // Передаём профиль пользователя и признак "свой профиль" (true)
    m_profileViewWidget->setUserProfile(*user,  true);

    // Переключаем правый layout на виджет профиля
    m_rightSideStackedLayout->setCurrentWidget(m_profileViewWidget);
}


QPushButton* MainWindow::createMenuButton(const QString& text, const QString& badge)
{
    QPushButton* btn = new QPushButton;
    btn->setFixedHeight(45);

    // Если нужен бейдж — добавляем его к тексту
    QString btnText = text;
    if (!badge.isEmpty()) {
        btnText += QString("  [%1]").arg(badge);
    }
    btn->setText(btnText);

    // CSS для визуального стиля кнопки
    btn->setStyleSheet(
        "QPushButton { "
        "  background-color: #f5f5f5; "
        "  border: 1px solid #e0e0e0; "
        "  border-radius: 8px; "
        "  padding: 10px 15px; "
        "  font-size: 13px; "
        "  text-align: left; "
        "} "
        "QPushButton:hover { "
        "  background-color: #e8e8e8; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #d0d0d0; "
        "}"
        );
    return btn;
}


void MainWindow::onMenuButtonClicked()
{
    qDebug() << "[UI] Menu button clicked";
    // В левой панели отображаем страницу меню
    m_leftMainPanel->setCurrentIndex(PAGE_MENU);
}


void MainWindow::onCallsButtonClicked()
{
    qDebug() << "[UI] Calls button clicked - showing Call History";

    if (m_callHistoryWidget) {
        // Получаем username текущего пользователя
        QString username = m_dataService->getCurrentUser()->username;
        qDebug() << "[DATA] Requesting call history for:" << username;

        // Формируем запрос к backend
        QJsonObject request;
        request["type"] = "get_call_history";
        request["username"] = username;
        m_networkService->sendJson(request);

        qDebug() << "[CALLS] Loading call history...";
        m_rightSideStackedLayout->setCurrentWidget(m_callHistoryWidget);
    }
}


void MainWindow::onBackFromMenu()
{
    qDebug() << "[UI] Back from Menu";
    m_leftMainPanel->setCurrentIndex(PAGE_CHATS);
    m_profileViewWidget->reset();
    m_rightSideStackedLayout->setCurrentWidget(m_placeholderWidget);
}



void MainWindow::buildMainUI()
{
    // Виджет авторизации
    m_loginWidget = new LoginWidget(this);
    if (!m_loginWidget) {
        qWarning() << "[MainWindow] Failed to create m_loginWidget";
        return;
    }

    // Главный виджет чата (левая + правая панели)
    m_mainChatWidget = new QWidget();
    if (!m_mainChatWidget) {
        qDebug() << "[MainWindow] Failed to create m_mainChatWidget";
        return;
    }

    // Горизонтальный лэйаут для деления окна (левая/правая части)
    auto* mainLayout = new QHBoxLayout(m_mainChatWidget);
    if (!mainLayout) {
        qDebug() << "[MainWindow] Failed to create mainLayout";
        return;
    }
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(1);

    // Список чатов (панель слева)
    m_chatListPanel = new QWidget();
    m_chatListPanel->setObjectName("chatListPanel");
    auto* leftLayout = new QVBoxLayout(m_chatListPanel);
    leftLayout->setContentsMargins(5, 5, 5, 5);

    m_searchLineEdit = new QLineEdit();
    m_searchLineEdit->setPlaceholderText("Поиск контактов...");
    m_userListView = new QListView(this);
    m_contactModel = new ContactListModel(m_dataService, this);
    m_userListView->setModel(m_contactModel);
    m_userListView->setItemDelegate(new ContactListDelegate(this));
    m_userListView->setMinimumWidth(250);
    m_userListView->setMaximumWidth(500);
    m_userListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_leftMainPanel = new QStackedWidget(this);
    m_leftMainPanel->addWidget(m_chatListPanel);

    // Верхнее меню (кнопка меню + строка поиска)
    m_menuButton = new QToolButton(m_chatListPanel);
    m_menuButton->setIcon(QIcon(":/icons/icons/menu.png"));
    m_menuButton->setToolTip("Меню");
    m_menuButton->setFixedSize(32,32);

    m_mainMenu = new QMenu(this);
    m_actionIncomingRequests = m_mainMenu->addAction("Входящие запросы");

    QHBoxLayout* topMenuLayout = new QHBoxLayout();
    topMenuLayout->setContentsMargins(0,0,0,0);
    topMenuLayout->addWidget(m_menuButton);
    topMenuLayout->addStretch();
    leftLayout->insertLayout(0, topMenuLayout);

    topMenuLayout->addWidget(m_searchLineEdit);
    leftLayout->addWidget(m_userListView);

    // Правая часть: контейнер и компоновка
    m_rightSideContainer = new QWidget();
    m_rightSideContainer->setObjectName("rightSideContainer");
    m_rightSideLayout = new QVBoxLayout(m_rightSideContainer);
    m_rightSideLayout->setContentsMargins(0, 0, 0, 0);

    // Заглушка — по умолчанию, когда ничего не выбрано
    m_placeholderWidget = new QWidget(m_rightSideContainer);
    auto* placeholderLayout = new QVBoxLayout(m_placeholderWidget);
    auto* placeholderLabel = new QLabel("Выберите чат, чтобы начать общение");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setObjectName("placeholderLabel");
    placeholderLayout->addWidget(placeholderLabel);

    // Основной чатовый виджет
    m_chatViewWidget = new ChatViewWidget(m_rightSideContainer);
    QListView* chatView = m_chatViewWidget->chatHistoryView();
    chatView->setModel(m_chatFilterProxy);
    m_chatDelegate = new ChatMessageDelegate(m_chatModel, chatView);
    chatView->setItemDelegate(m_chatDelegate);

    // Стековый layout для правой панели (чат, профиль, заявка, история звонков, заглушка)
    m_rightSideStackedLayout = new QStackedLayout;
    m_profileViewWidget = new ProfileViewWidget(m_networkService, m_rightSideContainer);
    m_rightSideContainer->setLayout(m_rightSideLayout);

    m_rightSideStackedLayout->addWidget(m_chatViewWidget);
    m_rightSideStackedLayout->addWidget(m_profileViewWidget);

    m_incomingRequestsWidget = new IncomingRequestsWidget(m_rightSideContainer);
    m_rightSideStackedLayout->addWidget(m_incomingRequestsWidget);
    m_rightSideStackedLayout->addWidget(m_placeholderWidget);
    m_callHistoryWidget = new CallHistoryWidget;
    m_rightSideStackedLayout->addWidget(m_callHistoryWidget);

    // Сигнал: refresh истории звонков
    connect(m_callHistoryWidget, &CallHistoryWidget::refreshRequested,
            this, [this](){
                QString username = m_dataService->getCurrentUser()->username;
                qDebug() << "[DATA] Requesting call history for:" << username;
                QJsonObject request;
                request["type"] = "get_call_history";
                request["username"] = username;
                m_networkService->sendJson(request);
            });

    connect(m_dataService, &DataService::callHistoryReceived,
            m_callHistoryWidget, &CallHistoryWidget::setCallHistory);

    // Финальный монтаж layout'а: сначала rightSideStackedLayout, затем остальное
    m_rightSideLayout->addLayout(m_rightSideStackedLayout);
    m_rightSideStackedLayout->setCurrentWidget(m_placeholderWidget);

    mainLayout->addWidget(m_leftMainPanel, 0);
    mainLayout->addWidget(m_rightSideContainer, 1);
}


void MainWindow::onContactsUpdated(const QStringList& sortedUsernames) {
    m_contactModel->updateContacts(sortedUsernames);
}


void MainWindow::onOnlineStatusUpdated() {
    if (m_dataService->getUserCache() == nullptr) {
        return;
    }
    m_contactModel->updateContacts(m_dataService->getUserCache()->keys());
}


void MainWindow::onOlderHistoryChunkPrepended(const QString& chatPartner, const QList<ChatMessage>& messages) {
    if (chatPartner != m_dataService->getCurrentChatPartner()->username || messages.isEmpty()) {
        // Если чата нет, грузить не нужно, сбрасываем флаги ожидания/загрузки
        *(m_dataService->getIsLoadingHistory()) = false;
        m_expectingRangeChange = false;
        return;
    }
    // Добавляем сообщения в начало модели чата
    m_chatModel->prependMessages(messages);
}


void MainWindow::onScrollBarRangeChanged()
{
    // Если мы не ожидали range change (аналог лени), ничего не выполняем
    if (!m_expectingRangeChange) {
        return;
    }

    m_expectingRangeChange = false;
    qDebug() << "[SCROLL] Range changed as expected.";

    SmoothListView* view = qobject_cast<SmoothListView*>(m_chatViewWidget->chatHistoryView());
    QScrollBar* scrollBar = view->verticalScrollBar();

    int newMaximum = scrollBar->maximum();
    int heightAdded = newMaximum - m_oldScrollMax;

    if (heightAdded > 0) {
        view->stopScrollAnimation();
        m_programmaticScrollInProgress = true;
        scrollBar->setValue(heightAdded);
        qDebug() << "[SCROLL] Manually setting scroll value to:" << heightAdded;

        // Через 50 мс сбрасываем признак "программная прокрутка"
        QTimer::singleShot(50, this, [this]() {
            m_programmaticScrollInProgress = false;
        });
    }

    *(m_dataService->getIsLoadingHistory()) = false;
    qDebug() << "[SCROLL] Loading finished, flag reset.";
}


void MainWindow::onHistoryLoaded(const QString& chatPartner, const QList<ChatMessage>& messages) {
    if (chatPartner != m_dataService->getCurrentChatPartner()->username) {
        return;
    }
    m_chatModel->clearMessages();
    m_chatModel->addMessages(messages);

    // Отправляем scrollToBottom — через invokeMethod (async UI)
    QMetaObject::invokeMethod(m_chatViewWidget->chatHistoryView(), "scrollToBottom", Qt::QueuedConnection);

    // После скролла — обработать видимые сообщения (для ленточки, медиа и др.)
    QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);
}



void MainWindow::onLoginSuccess(const QJsonObject& response)
{
    // Создаём объект пользователя из JSON
    User me;
    me.username      = response["username"].toString();
    me.displayName   = response["display_name"].toString();
    me.statusMessage = response["status_message"].toString();
    me.avatarUrl     = response["avatar_url"].toString();
    me.isOnline = true;
    me.isTyping = false;

    // Проверяем наличие виджета логина
    if (m_loginWidget.isNull()) {
        qWarning() << "[MainWindow] LoginWidget was deleted!";
        return;
    }

    // Проверяем сервис данных
    if(m_dataService == nullptr)
        qDebug() << "m_data service is nullptr";

    // Сохраняем пользователя в DataService
    *m_dataService->getCurrentUser() = me;
    qDebug() << "[DataService] Login successful for user:" << m_dataService->getCurrentUser()->username;

    // Очищаем поля ввода (логин/пароль)
    m_loginWidget->clearFields();

    // Переключаем UI на основную страницу чатов
    if (m_mainChatWidget) {
        ui->rootStackedWidget->setCurrentWidget(m_mainChatWidget);
    }

    // Обновляем заголовок окна
    this->setWindowTitle(me.username);

    // Проверяем кеш пользователей (userCache) и загружаем недавние сообщения из базы для каждого чата
    auto userCache = m_dataService->getUserCache();
    if (userCache) {
        QStringList contacts = userCache->keys();
        qDebug() << "[MainWindow] Contacts to load history for:" << contacts;

        for (const QString& contact : contacts) {
            QList<ChatMessage> cachedMessages = m_dataService->getDatabaseService()
            ->loadRecentMessages(m_dataService->getCurrentUser()->username, contact, 50);
            qDebug() << "[MainWindow] Loaded" << cachedMessages.size() << "messages for chat with" << contact;
        }
    } else {
        qDebug() << "[FATAL] userCache is nullptr!";
    }
}


void MainWindow::onLoginFailure(const QString& reason){
    QMessageBox::warning(this, "Ошибка входа", "Не удалось войти: " + reason);
}


void MainWindow::onRegisterSuccess(){
    QMessageBox::information(this, "Регистрация успешна", "Ваш аккаунт был успешно создан. Теперь вы можете войти.");
    m_loginWidget->onRegistrationSuccess();
}


void MainWindow::onRegisterFailure(const QString& reason){
    QMessageBox::warning(this, "Ошибка регистрации", reason);
}


void MainWindow::onNewMessageReceived(const ChatMessage& incomingMsg)
{
    qDebug() << "onNewMessageRecived";

    // Всегда отправляем на сервер событие "доставлено"
    QJsonObject deliveredCmd;
    deliveredCmd["type"] = "message_delivered";
    deliveredCmd["id"] = (double)incomingMsg.id;
    m_networkService->sendJson(deliveredCmd);

    // Если сообщение для текущего открытого чата
    if (incomingMsg.fromUser == m_dataService->getCurrentChatPartner()->username && m_dataService->getCurrentChatPartner() != nullptr) {
        bool wasScrolledToBottom = m_chatViewWidget->isScrolledToBottom();
        if (!m_chatModel) {
            qWarning() << "[MainWindow] ChatModel deleted!";
            return;
        }
        m_chatModel->addMessage(incomingMsg);

        if (wasScrolledToBottom) {
            // Скроллим до конца чата, если был внизу
            QMetaObject::invokeMethod(m_chatViewWidget, "scrollToBottom", Qt::QueuedConnection);
        } else {
            // Иначе эмитим сигнал для UI "есть непрочитанные"
            emit newMessageForCurrentChat();
        }

        // После вставки — пересчёт видимых сообщений (лента, медиа и др.)
        QTimer::singleShot(100, this, &MainWindow::processVisibleMessages);
    } else {
        // Если сообщение не для текущего чата — обновляем счётчик и контакт-лист
        if(m_dataService->getUnreadCounts() == nullptr){
            qDebug() <<"m_dataService->getUnreadCounts() == nullptr";
            (*(m_dataService->getUnreadCounts()))[incomingMsg.fromUser] = 0;
        }
        (*(m_dataService->getUnreadCounts()))[incomingMsg.fromUser]++;
        m_contactModel->refreshContact(incomingMsg.fromUser);
    }

    // Если чат не активен или окно свернуто — делаем визуальное оповещение (мерцание панели/иконки)
    if (incomingMsg.fromUser != m_dataService->getCurrentChatPartner()->username || isMinimized() || !isActiveWindow()) {
        QApplication::alert(this);
    }
}


void MainWindow::onMessageStatusChanged(qint64 messageId, ChatMessage::MessageStatus newStatus) {
    m_chatModel->updateMessageStatus(messageId, newStatus);
}


void MainWindow::onUnreadCountChanged() {
    updateUserList();
}


void MainWindow::onMessageEdited(const QString& chatPartner, qint64 messageId, const QString& newPayload) {
    if (chatPartner == m_dataService->getCurrentChatPartner()->username) {
        m_chatModel->editMessage(messageId, newPayload);
    }
    m_contactModel->refreshContact(chatPartner);
}


void MainWindow::onMessageDeleted(const QString& chatPartner, qint64 messageId) {
    if (chatPartner == m_dataService->getCurrentChatPartner()->username) {
        m_chatModel->removeMessage(messageId);
    }
    m_contactModel->refreshContact(chatPartner);
}


void MainWindow::onSearchResultsReceived(const QJsonArray& users){
    QWidget *searchBar = m_searchLineEdit;
    m_searchResultsPopup->move(searchBar->mapToGlobal(QPoint(0, searchBar->height())));
    m_searchResultsPopup->setFixedWidth(searchBar->width());
    m_searchResultsPopup->showResults(users);
    QTimer::singleShot(0, this, [this]() {
        m_searchLineEdit->setFocus();
    });
}


void MainWindow::onAddContactSuccess(const QString& username) {
    QMessageBox::information(this, "Запрос отправлен", "Запрос на добавление пользователя " + username + " был успешно отправлен.");
}

void MainWindow::onAddContactFailure(const QString& reason) {
    QMessageBox::warning(this, "Ошибка добавления контакта", reason);
}


void MainWindow::onPendingContactRequestsUpdated(const QJsonArray & requests){
    for (const QJsonValue &value : requests) {
        QJsonObject reqObj = value.toObject();
        QString fromUsername = reqObj["fromUsername"].toString();
        QString fromDisplayName = reqObj["fromDisplayname"].toString();
        showContactRequestPrompt(fromUsername, fromDisplayName);
    }
}


void MainWindow::onLogoutSuccess(){
    qDebug() << "[CLIENT] Logout successful. Resetting application state.";
    QMessageBox::information(this, "Выход", "Вы успешно вышли из аккаунта.");
    resetApplicationState();
}


void MainWindow::onLogoutFailure(const QString& reason){
    QMessageBox::warning(this, "Ошибка", reason);
}


void MainWindow::onTypingStatusChanged(const QString& username, bool isTyping){
    Q_UNUSED(isTyping);
    m_contactModel->refreshContact(username);
    if (m_dataService->getCurrentChatPartner()->username == username) {
        m_chatViewWidget->updateHeader(*(m_dataService->getUserFromCache(username)));
    }
}


void MainWindow::onConfirmMessageSent(QString tempId, const ChatMessage& msg){
    m_chatModel->confirmMessage(tempId, msg);
}


void MainWindow::setupConnections()
{
    qDebug() << "[MainWindow] setupConnections() START";

    // Проверка ключевых сервисов (без них дальше нет смысла работать)
    if (!m_callService) {
        qWarning() << "[MainWindow] ERROR: m_callService is nullptr!";
        return;
    }
    qDebug() << "[MainWindow] m_callService exists: OK";

    if (!m_callWidget) {
        qWarning() << "[MainWindow] ERROR: m_callWidget is nullptr!";
        return;
    }
    qDebug() << "[MainWindow] m_callWidget exists: OK";

    // --- Подключение сервисных и пользовательских событий (DataService/NetworkService/UI) ---
    connect(m_networkService, &NetworkService::connected, this, &MainWindow::onConnected);
    connect(m_networkService, &NetworkService::disconnected, this, &MainWindow::onDisconnected);
    connect(m_networkService, &NetworkService::jsonReceived, this, &MainWindow::onJsonReceived);

    connect(m_dataService, &DataService::contactsUpdated, this, &MainWindow::onContactsUpdated);
    connect(m_dataService, &DataService::onlineStatusUpdated, this, &MainWindow::onOnlineStatusUpdated);
    connect(m_dataService, &DataService::olderHistoryChunkPrepended, this, &MainWindow::onOlderHistoryChunkPrepended);
    connect(m_dataService, &DataService::historyLoaded, this, &MainWindow::onHistoryLoaded);
    connect(m_dataService, &DataService::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(m_dataService, &DataService::loginFailure, this, &MainWindow::onLoginFailure);
    connect(m_dataService, &DataService::registerSuccess, this, &MainWindow::onRegisterSuccess);
    connect(m_dataService, &DataService::registerFailure, this, &MainWindow::onRegisterFailure);

    connect(m_dataService, &DataService::newMessageReceived, this, &MainWindow::onNewMessageReceived);
    connect(m_dataService, &DataService::messageStatusChanged, this, &MainWindow::onMessageStatusChanged);
    connect(m_dataService, &DataService::unreadCountChanged, this, &MainWindow::onUnreadCountChanged);
    connect(m_dataService, &DataService::messageEdited, this, &MainWindow::onMessageEdited);
    connect(m_dataService, &DataService::messageDeleted, this, &MainWindow::onMessageDeleted);
    connect(m_dataService, &DataService::searchResultsReceived, this, &MainWindow::onSearchResultsReceived);
    connect(m_dataService, &DataService::addContactSuccess, this, &MainWindow::onAddContactSuccess);
    connect(m_dataService, &DataService::addContactFailure, this, &MainWindow::onAddContactFailure);

    // Входящие запросы — напрямую во view виджет заявок
    connect(m_dataService, &DataService::contactRequestReceived, m_incomingRequestsWidget, &IncomingRequestsWidget::addRequest);
    connect(m_incomingRequestsWidget, &IncomingRequestsWidget::requestRejected, this, &MainWindow::onRequestRejected);
    connect(m_incomingRequestsWidget, &IncomingRequestsWidget::requestAccepted, this, &MainWindow::onRequestAccepted);

    connect(m_dataService, &DataService::logoutSuccess, this, &MainWindow::onLogoutSuccess);
    connect(m_dataService, &DataService::logoutFailure, this, &MainWindow::onLogoutFailure);
    connect(m_dataService, &DataService::typingStatusChanged, this, &MainWindow::onTypingStatusChanged);
    connect(m_dataService, &DataService::confirmMessageSent, this, &MainWindow::onConfirmMessageSent);

    // Не запускать, если логин-виджет ещё не создан
    if(m_loginWidget.isNull()){
        qDebug()<<"m_loginWidget is NULLPTR INSIDE CONNECTIONS";
        return;
    }
    connect(m_loginWidget, &LoginWidget::loginRequested, this, &MainWindow::onLoginRequested);
    connect(m_loginWidget, &LoginWidget::registerRequested, this, &MainWindow::onRegisterRequested);

    // --- Чатовые сигналы/слоты ---
    connect(m_chatViewWidget, &ChatViewWidget::sendMessageRequested, this, &MainWindow::onSendMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::headerClicked, this, &MainWindow::showProfileView);
    connect(m_profileViewWidget, &ProfileViewWidget::backButtonClicked, this, &MainWindow::hideProfileView);
    connect(m_chatViewWidget->messageTextEdit(), &QTextEdit::textChanged, this, &MainWindow::onTypingNotificationFired);
    connect(m_chatViewWidget, &ChatViewWidget::replyToMessageRequested, this, &MainWindow::onReplyToMessage);
    connect(m_chatViewWidget, &ChatViewWidget::editMessageRequested, this, &MainWindow::onEditMessageRequested);
    connect(m_chatViewWidget, &ChatViewWidget::deleteMessageRequested, this, &MainWindow::onDeleteMessageRequested);

    // Сброс reply-состояния
    connect(m_chatViewWidget, &ChatViewWidget::replyCancelled, this, [this](){
        *(m_dataService->getReplyToMessageId()) = 0;
    });
    connect(m_chatViewWidget, &ChatViewWidget::scrollToUnreadRequested, this, &MainWindow::onScrollToUnread);
    connect(m_chatViewWidget, &ChatViewWidget::scrollToBottomRequested, this, &MainWindow::onScrollToBottom);

    QScrollBar* scrollBar = m_chatViewWidget->chatHistoryView()->verticalScrollBar();
    connect(scrollBar, &QScrollBar::valueChanged, this, &MainWindow::processVisibleMessages);
    connect(scrollBar, &QScrollBar::valueChanged, this, &MainWindow::onChatScroll);
    connect(scrollBar, &QScrollBar::rangeChanged, this, &MainWindow::onScrollBarRangeChanged);

    connect(m_userListView, &QListView::clicked, this, &MainWindow::onUserSelectionChanged);

    // Поиск: запускает дебоунс-таймер при каждом новом вводе
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        if (text.isEmpty()){
            m_dataService->getGlobalSearchTimer()->stop();
        } else {
            m_dataService->getGlobalSearchTimer()->start();
        }
    });
    connect(m_dataService->getGlobalSearchTimer(), &QTimer::timeout, this, &MainWindow::onGlobalSearchTriggered);
    connect(m_searchResultsPopup, &SearchResultsPopup::userSelected, this, &MainWindow::onAddContactRequested);

    // Кнопка logout
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);

    // Вспомогательные сигналы/слоты
    connect(this, &MainWindow::newMessageForCurrentChat, m_chatViewWidget, &ChatViewWidget::onNewMessageReceived);
    connect(m_chatModel, &ChatMessageModel::messageNeedsReadReceipt, this, &MainWindow::onSendMessageReadReceipt);

    // Символьный поиск по истории чата
    connect(m_chatViewWidget, &ChatViewWidget::searchTextEntered, this, [this](const QString& text){
        m_chatFilterProxy->setFilterRegularExpression(QRegularExpression::escape(text));
    });

    // --- Меню и заявки ---
    connect(m_actionIncomingRequests, &QAction::triggered, [this]{
        onUserSelectionChanged(QModelIndex());
        m_rightSideStackedLayout->setCurrentWidget(m_incomingRequestsWidget);
    });
    connect(m_dataService, &DataService::pendingContactRequestsUpdated, m_incomingRequestsWidget, &IncomingRequestsWidget::updateRequests);
    connect(m_menuButton, &QToolButton::clicked, this,  &MainWindow::onMenuButtonClicked);

    // --- Звонки (audio/video) ---
    qDebug() << "[MW] Setting up call connections...";
    if (!m_callService || !m_callWidget) {
        qWarning() << "[MW] ❌ CallService or CallWidget not initialized";
        return;
    }

    // Запрос на вызов (из чата)
    connect(m_chatViewWidget, &ChatViewWidget::callRequested, this,
            [this]() {
                const QString& username = m_dataService->getCurrentChatPartner()->username;
                qDebug() << "[MW] >>> callRequested from ChatViewWidget, user:" << username;
                if (!m_callService) {
                    qWarning() << "[MW] ❌ CallService not initialized";
                    return;
                }
                qDebug() << "[MW] Calling m_callService->initiateCall(" << username << ")";
                m_callService->initiateCall(username);
                qDebug() << "[MW] ✅ Call initiated";
            });

    // Входящий вызов, принятый вызов, показ callWidget, обновление состояния, время, ошибки
    connect(m_dataService, &DataService::incomingCall, m_callService, &CallService::onCallRequestReceived);
    connect(m_dataService, &DataService::callAccepted, m_callService,
            [this](const QString& from, const QString& ip, quint16 port) {
                qDebug() << "[MW] Forwarding callAccepted to CallService";
                m_callService->onCallAcceptedReceived(ip, port);
            });

    connect(m_callService, &CallService::incomingCallShow, this,
            [this](const QString& fromUser) {
                qDebug() << "[MW] ========== INCOMING CALL SIGNAL RECEIVED ==========";
                qDebug() << "[MW] From:" << fromUser;
                if (!m_callWidget) {
                    qWarning() << "[MW] ❌ CallWidget is nullptr!";
                    return;
                }
                qDebug() << "[MW] Setting caller name to:" << fromUser;
                m_callWidget->setCallerName(fromUser);
                qDebug() << "[MW] Setting state to 'Входящий звонок'";
                m_callWidget->setCallState("Входящий звонок");
                m_callWidget->show();
                m_callWidget->raise();
                m_callWidget->activateWindow();
                qDebug() << "[MW] ✅ Incoming call UI SHOWN";
                qDebug() << "[MW] ================================================";
            });

    connect(m_callService, &CallService::outgoingCallShow, this,
            [this]() {
                qDebug() << "[MW] >>> OUTGOING CALL - waiting for answer";
                qDebug() << "[MW] Setting caller name to:" << m_dataService->getCurrentChatPartner()->username;
                m_callWidget->setCallerName(m_dataService->getCurrentChatPartner()->username);
                m_callWidget->setCallState("Ожидание ответа");
                m_callWidget->show();
                m_callWidget->raise();
                m_callWidget->activateWindow();
                qDebug() << "[MW] ✅ Outgoing call UI shown (waiting for remote user)";
            });

    connect(m_callService, &CallService::callConnected, this,
            [this]() {
                qDebug() << "[MW] >>> CALL CONNECTED";
                m_callWidget->setCallState("Разговор активен");
                m_callWidget->show();
                qDebug() << "[MW] ✅ Call connected";
            });

    connect(m_callService, &CallService::callDurationUpdated, this,
            [this](int seconds) {
                int mins = seconds / 60;
                int secs = seconds % 60;
                QString timeStr = QString::asprintf("%02d:%02d", mins, secs);
                m_callWidget->onDurationChanged(timeStr);
            });

    connect(m_callService, &CallService::callEnded, this,
            [this]() {
                qDebug() << "[MW] >>> CALL ENDED";
                m_callWidget->hide();
                qDebug() << "[MW] ✅ Call UI hidden";
            });

    connect(m_dataService, &DataService::callEnded, m_callService, &CallService::onCallEndedReceived);
    connect(m_callService, &CallService::callError, this,
            [this](const QString& error) {
                qWarning() << "[MW] Call error:" << error;
            });

    if (!m_callWidget) {
        qWarning() << "[MainWindow] CallWidget not available, skipping connections";
        return;
    }

    connect(m_callWidget, &CallWidget::acceptClicked, m_callService, &CallService::acceptCall);
    connect(m_callWidget, &CallWidget::rejectClicked, m_callService, &CallService::rejectCall);
    connect(m_callWidget, &CallWidget::endCallClicked, m_callService, &CallService::endCall);

    // Прочее: получение истории, обновление профиля
    connect(m_dataService, &DataService::requestServerHistory, this, &MainWindow::onRequestServerHistory);
    connect(m_dataService, &DataService::profileUpdateResult, this, &MainWindow::onProfileUpdateResult);

    qDebug() << "[MW] ✅ Call connections setup complete";
}


void MainWindow::onProfileUpdateResult(const QJsonObject& response) {
    if (response.value("success").toBool()) {
        // Получаем указатель на текущего пользователя
        User* currentUser = m_dataService->getCurrentUser();
        // Обновляем его displayName и статус из ответа
        currentUser->displayName = response.value("display_name").toString();
        currentUser->statusMessage = response.value("status_message").toString();

        // Передаём актуальные данные в виджет профиля (перерисовка/деактивация режима редактирования)
        m_profileViewWidget->setUserProfile(*currentUser, true);

        // Показываем пользователю, что профиль обновлён
        QMessageBox::information(this, "Профиль", "Профиль успешно обновлён!");
    } else {
        // Если неудачно — достаём причину
        QString reason = response.value("reason").toString();
        // Показываем диалог об ошибке с описанием
        QMessageBox::warning(this, "Ошибка", QString("Не удалось обновить профиль: %1").arg(reason));
    }
}


void MainWindow::onCallRequested() {
    qDebug() << "[MW] Call button clicked - initiating call";
    QString selectedUser = m_dataService->getCurrentChatPartner()->username;
    if (selectedUser.isEmpty()) {
        qWarning() << "[MW] ❌ No user selected";
        return;
    }
    qDebug() << "[MW] Calling user:" << selectedUser;
    if (m_callService) {
        m_callService->initiateCall(selectedUser);
    } else {
        qWarning() << "[MW] ❌ CallService is nullptr";
    }
}


void MainWindow::onScrollToUnread() {
    qDebug() << "[MainWindow] Scroll to unread requested.";
    // Индекс в оригинальной модели
    QModelIndex firstUnreadIndex = m_chatModel->findFirstUnreadMessage();
    // Отображаемый индекс (через прокси, фильтрация)
    QModelIndex proxyIndex = m_chatFilterProxy->mapFromSource(firstUnreadIndex);
    // Скроллим к сообщению в истории
    m_chatViewWidget->scrollToMessage(proxyIndex);
}


void MainWindow::onScrollToBottom() {
    qDebug() << "[MainWindow] Scroll to bottom requested.";
    m_chatViewWidget->scrollToBottom();
}


void MainWindow::onConnected() {
    ui->statusbar->showMessage("Подключено", 2000); // В статусбар — уведомление о подключении на 2000 мс
    qDebug() << "[DEBUG] onConnected, m_loginWidget =" << m_loginWidget;
    if (!m_loginWidget) {
        qWarning() << "m_loginWidget is nullptr in onConnected!";
        return;
    }
    m_loginWidget->setUiEnabled(true); // Разблокируем поля для пользователя
}

void MainWindow::onDisconnected() {
    ui->statusbar->showMessage("Отключено");
    m_loginWidget->setUiEnabled(false); // Блокируем форму логина
}


void MainWindow::onJsonReceived(const QJsonObject& response) {
    m_dataService->processResponse(response); // Централизованный парсер всей логики ответов от backend
}


void MainWindow::processVisibleMessages()
{
    // Получаем view истории сообщений текущего чата
    QListView* view = m_chatViewWidget->chatHistoryView();

    // Без модели или если не выбран собеседник — ничего не делаем
    if (!view->model() || m_dataService->getCurrentChatPartner()->username.isEmpty()) {
        return;
    }

    // Границы viewport'а (видимой области в списке)
    QRect viewportRect = view->viewport()->rect();
    QAbstractItemModel* model = view->model();
    int rowCount = model->rowCount();

    // Перебираем все строки виджета истории
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0);
        QRect rect = view->visualRect(index);   // Где реально отрисован элемент
        if (!rect.isValid() || rect.isEmpty()) continue;
        // Оставляем только реально видимые сообщения (пересечение с viewport)
        if (rect.bottom() >= viewportRect.top() && rect.top() <= viewportRect.bottom()) {
            // Достаем сообщение из модельного поля UserRole
            ChatMessage msg = index.data(Qt::UserRole).value<ChatMessage>();
            // Проверяем: входящее сообщение, статус либо Delivered, либо Sent
            if (!msg.isOutgoing && (msg.status == ChatMessage::Delivered || msg.status == ChatMessage::Sent)) {
                // Эмитим сигнал для модели, что этому сообщению нужен read-receipt (прочитано)
                emit m_chatModel->messageNeedsReadReceipt(msg.id);
            }
        }
    }
}


void MainWindow::onEditMessageRequested(qint64 messageId, const QString& oldText)
{
    qDebug() << "[MainWindow]: Caught editMessageRequested signal for ID:" << messageId;

    // Скрываем интерфейс "ответить", чтобы не пересекался с edit
    m_chatViewWidget->hideReplyUI();

    // Через указатель в DataService передаем ID редактируемого сообщения
    *(m_dataService->getEditinigMessageId()) = messageId;

    // Включаем "режим редактирования" в чате (UI): старый текст и спец. оформление
    m_chatViewWidget->setEditMode(true, oldText);
}


void MainWindow::onDeleteMessageRequested(qint64 messageId)
{
    QJsonObject request;
    request["type"] = "delete_message";  // Тип запроса на сервер
    request["id"] = messageId;           // Какое сообщение удалять

    m_networkService->sendJson(request); // Отправляем серверу
}


void MainWindow::onAddContactRequested(const QString& username)
{
    // Прячем PopUp с поиском, чтобы не мешал диалогу
    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }

    // Диалоговое окно с подтверждением действия
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Добавить контакт",
                                  "Отправить запрос на добавление в контакты пользователю " + username + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    // Если выбрано "Да" — отправляем запрос на сервер
    if (reply == QMessageBox::Yes) {
        qDebug() << "[CLIENT] Sending 'add_contact_request' for user:" << username;

        QJsonObject request;
        request["type"] = "add_contact_request";
        request["username"] = username;
        m_networkService->sendJson(request);
    }
}


void MainWindow::onTypingNotificationFired()
{
    // Без собеседника — нет смысла посылать typing
    if (m_dataService->getCurrentChatPartner()->username.isEmpty()) return;

    // Пустое поле ввода — не считается за typing
    if (m_chatViewWidget->messageTextEdit()->toPlainText().isEmpty()) return;

    // Не чаще, чем разрешает antispam-таймер (anti-flood)
    if ( m_dataService->getTypingSendTimer()->isActive()) return;

    // Формируем JSON для backend
    QJsonObject typingRequest;
    typingRequest["type"] = "typing";
    typingRequest["toUser"] = m_dataService->getCurrentChatPartner()->username;
    m_networkService->sendJson(typingRequest);

    // Стартуем таймер блокировки, чтобы следующее уведомление не ушло раньше времени
    m_dataService->getTypingSendTimer()->start();
}


void MainWindow::onReplyToMessage(qint64 messageId)
{
    qDebug() << "[CLIENT] Setting reply context to message ID:" << messageId;
    ChatMessage msg;

    // Ищем сообщение по ID, если находим — продолжаем
    if (m_chatModel->getMessageById(messageId, msg)) {
        *(m_dataService->getReplyToMessageId()) = messageId;

        // На UI — показать reply bar снизу (от какого пользователя и preview текста)
        m_chatViewWidget->showReplyUI(msg.fromUser, msg.payload);
    }
}


void MainWindow::onChatSearchTriggered(const QString &text)
{
    Q_UNUSED(text); // Не используется
}


void MainWindow::resetApplicationState()
{
    // Сбросим все данные, связанные с звонками (CallService)
    if (m_callService) {
        m_callService->resetCallData();
    }

    // Сброс данных и кешей внутри DataService (пользователь, контакты, сообщения и пр.)
    m_dataService->clearAllData();

    // Полная очистка чата и контактов на UI, если модели уже созданы
    if (m_chatModel) {
        m_chatModel->clearMessages();
    }
    m_contactModel->clear();  // убрать все контакты

    // Очистка строк поиска, поля ввода сообщения, reply-бар и редактор
    m_searchLineEdit->clear();
    m_chatViewWidget->messageTextEdit()->clear();
    m_chatViewWidget->hideReplyUI();
    m_chatViewWidget->setEditMode(false);

    // Прячем popup поиска (если он открыт)
    if (m_searchResultsPopup) {
        m_searchResultsPopup->hide();
    }

    // Возвращаем UI в состояние "нет открытого чата": справа заглушка, слева список чатов
    m_rightSideStackedLayout->setCurrentWidget(m_placeholderWidget);
    m_leftMainPanel->setCurrentWidget(m_chatListPanel);

    // Показываем экран авторизации (rootStackedWidget)
    ui->rootStackedWidget->setCurrentWidget(m_loginWidget);

    qDebug() << "[MainWindow] Application UI and models have been reset.";
}



void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    // Проверка на пустые поля: нельзя отправлять пустые значения
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя пользователя и пароль не могут быть пустыми.");
        return;
    }

    qDebug() << "[CLIENT] MainWindow: Login requested for user:" << username;

    // Формируем JSON объект для запроса на сервер
    QJsonObject loginRequest;
    loginRequest["type"] = "login";    // указываем тип запроса
    loginRequest["username"] = username;
    loginRequest["password"] = password;

    // Отправляем JSON через сервис сетевого взаимодействия
    m_networkService->sendJson(loginRequest);
}


void MainWindow::onRegisterRequested(const QString& username, const QString& displayName, const QString& password)
{
    // Проверка: все поля обязательны
    if (username.isEmpty() || password.isEmpty() || displayName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены.");
        return;
    }

    qDebug() << "[CLIENT] MainWindow: Register requested for user:" << username;

    // Формируем JSON-запрос на регистрацию
    QJsonObject registerRequest;
    registerRequest["type"] = "register";
    registerRequest["username"] = username;
    registerRequest["password"] = password;
    registerRequest["display_name"] = displayName;

    // Отправляем на сервер
    m_networkService->sendJson(registerRequest);
}


void MainWindow::onSendMessageRequested(const QString& text)
{
    // Без собеседника или пустой текст — не отправлять
    if (text.isEmpty() || m_dataService->getCurrentChatPartner()->username.isEmpty()) {
        return;
    }

    // Если сейчас редактируется сообщение (edit mode)
    if (*(m_dataService->getEditinigMessageId()) > 0) {
        qDebug() << "[CLIENT] Sending 'edit_message' request for ID:" << *(m_dataService->getEditinigMessageId());

        QJsonObject request;
        request["type"] = "edit_message";                            // backend-код для редактирования
        request["id"] = *(m_dataService->getEditinigMessageId());    // какое сообщение
        request["payload"] = text;                                   // на что меняем
        m_networkService->sendJson(request);

        // Завершаем edit mode: сбрасываем ID и отключаем UI-режим edit
        *(m_dataService->getEditinigMessageId()) = 0;
        m_chatViewWidget->setEditMode(false);

    } else {
        // --- Обычный режим отправки нового сообщения ---
        ChatMessage msg;
        msg.fromUser = (m_dataService->getCurrentUser()->username);
        msg.toUser = m_dataService->getCurrentChatPartner()->username;
        msg.payload = text;
        msg.status = ChatMessage::Sending;          // локальный статус: только что отправляем
        msg.isOutgoing = true;
        msg.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        msg.tempId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        msg.replyToId = *(m_dataService->getReplyToMessageId());

        // Сохраняем в SQLite, если возможно (асинхронно или sync)
        if (m_dataService->getDatabaseService() && m_dataService->getDatabaseService()->isConnected()) {
            m_dataService->getDatabaseService()->saveMessage(msg, m_dataService->getCurrentUser()->username);
        }

        // Вставляем в модель для мгновенного отображения в чате
        m_chatModel->addMessage(msg);
        // Скроллим вниз (новое сообщение — всегда снизу)
        QMetaObject::invokeMethod(m_chatViewWidget, "scrollToBottom", Qt::QueuedConnection);

        // Добавляем в кэш сообщений, если он сформирован
        QString chatPartner = m_dataService->getCurrentChatPartner()->username;
        if (m_dataService->getChatCacheForUser(chatPartner) != nullptr) {
            m_dataService->getChatCacheForUser(chatPartner)->messages.append(msg);
        }

        // JSON-запрос к серверу для отправки личного сообщения
        QJsonObject request;
        request["type"] = "private_message";
        request["fromUser"] = msg.fromUser;
        request["toUser"] = msg.toUser;
        request["payload"] = msg.payload;
        request["reply_to_id"] = msg.replyToId;
        request["temp_id"] = msg.tempId;
        m_networkService->sendJson(request);

        // Если это был ответ (reply), сбрасываем состояние и скрываем reply UI
        if (*(m_dataService->getReplyToMessageId()) > 0) {
            *(m_dataService->getReplyToMessageId()) = 0;
            m_chatViewWidget->hideReplyUI();
        }
    }
    // Обновляем контакты (UI) — возможно, чат сдвинется наверх
    m_contactModel->refreshContact(m_dataService->getCurrentChatPartner()->username);
}



void MainWindow::onLogoutButtonClicked()
{
    qDebug() << "[CLIENT] MainWindow: Logout button clicked.";

    // Формируем logout-запрос в виде JSON для backend
    QJsonObject logoutRequest;
    logoutRequest["type"] = "logout_request";
    logoutRequest["username"] = m_dataService->getCurrentUser()->username;  // чей аккаунт логаутить

    // Отправляем на сервер для сброса сессии/токена
    m_networkService->sendJson(logoutRequest);
}


void MainWindow::onUserSelectionChanged(const QModelIndex &current)
{
    // Нет выбора (клик вне контакта, сброс — либо первый старт)
    if (!current.isValid()) {
        qDebug() << "Current item is null, resetting view.";
        m_rightSideStackedLayout->setCurrentWidget(m_placeholderWidget);

        // Сброс текущего собеседника и всех reply/edit/cache состояний
        (*(m_dataService->getCurrentChatPartner())) = User();
        if (m_chatDelegate) {
            m_chatDelegate->clearSizeHintCache();
            m_chatDelegate->clearCaches();
        }
        m_chatModel->clearMessages();
        m_userListView->clearSelection();
        qDebug() << "--- onUserSelectionChanged END (reset) ---";
        return;
    }

    qDebug() << "--- onUserSelectionChanged START ---";

    // Сбросить reply UI если он активен (остался с прошлого чата)
    if (*(m_dataService->getReplyToMessageId()) > 0) {
        *(m_dataService->getReplyToMessageId()) = 0;
        m_chatViewWidget->hideReplyUI();
    }

    // Username выбранного контакта из модели
    QString selectedUsername = current.data(ContactListModel::UsernameRole).toString();
    qDebug() << "Selected username from model:" << selectedUsername;

    // Если были непрочитанные для этого пользователя — сбрасываем на 0 и обновляем контакт
    if (m_dataService->getUnreadCounts() != nullptr){
        if (m_dataService->getUnreadCounts()->value(selectedUsername, 0) > 0) {
            (*(m_dataService->getUnreadCounts()))[selectedUsername] = 0;
            m_contactModel->refreshContact(current);
        }
    }

    // Проверка, есть ли этот пользователь в кеше DataService
    if (m_dataService->getUserCache() != nullptr){
        if (selectedUsername.isEmpty() || !(*m_dataService->getUserCache()).contains(selectedUsername)) {
            qWarning() << "CRITICAL: Selected user not found in cache or username is empty!";
            return;
        }
    }

    // Сохраняем пользователя, с кем чат (для дальнейших операций)
    (*(m_dataService->getCurrentChatPartner())) = (*m_dataService->getUserCache()).value(selectedUsername);
    qDebug() << "Current chat partner set to:" << m_dataService->getCurrentChatPartner()->displayName;
    m_contactModel->refreshContact(selectedUsername);

    // Сбросим флаги загрузки, историю пагинации (oldestMessageId)
    *(m_dataService->getIsLoadingHistory()) = true;
    *(m_dataService->getOldestMessageId()) =
        (m_dataService->getChatCacheForUser(selectedUsername) != nullptr) ?
            m_dataService->getChatCacheForUser(selectedUsername)->oldestMessageId : -1;
    qDebug() << "Pagination state reset.";

    // Защита на случай отсутствия виджетов/моделей
    if (!m_chatViewWidget || !m_chatModel || !m_rightSideLayout) {
        qWarning() << "CRITICAL: A core widget is a nullptr!";
        return;
    }

    // Обновляем делегат, очищаем кеши, очищаем историю сообщений (UI)
    if (m_chatDelegate) {
        m_chatDelegate->clearSizeHintCache();
        m_chatDelegate->clearCaches();
    }
    m_chatModel->clearMessages();
    if (!m_chatViewWidget) {
        qWarning() << "[MainWindow] ChatViewWidget was deleted!";
        return;
    }
    // Обновляем header (аватар/статус/имя) в чате под нового собеседника
    m_chatViewWidget->updateHeader((*(m_dataService->getCurrentChatPartner())));

    // Показываем виджет переписки с пользователем
    m_rightSideStackedLayout->setCurrentWidget(m_chatViewWidget);

    qDebug() << "Switched to ChatViewWidget.";
    // Синхронизация истории чата (запрос к базе/серверу)
    m_dataService->syncChatHistory(selectedUsername);

    // Асинхронно обновляем "прочитано" для видимых сообщений (на всякий случай)
    QTimer::singleShot(50, this, &MainWindow::processVisibleMessages);
    qDebug() << "--- onUserSelectionChanged END (success) ---";
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // F5 — тестовая функция "музыкальная гамма"
    if (event->key() == Qt::Key_F5) {
        m_callService->playMusicalScale();
    }
    // F6 — тест диапазона частот
    else if (event->key() == Qt::Key_F6) {
        m_callService->testFrequencyRange();
    }

    // Прочие клавиши — стандартное поведение окна (не ломаем хоткеи/текстовые события)
    QMainWindow::keyPressEvent(event);
}


void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();      // выход из fullscreen
    } else {
        showFullScreen();  // переход в fullscreen
    }
}


void MainWindow::showProfileView()
{
    qDebug() << "[CLIENT] Showing profile for" << m_dataService->getCurrentChatPartner()->username;

    // Без выбранного пользователя — ничего не показываем
    if (m_dataService->getCurrentChatPartner()->username.isEmpty()) {
        return;
    }

    // Передаём данные пользователя в виджет профиля
    m_profileViewWidget->setUserProfile((*(m_dataService->getCurrentChatPartner())));

    // Показываем профиль вместо чата в правом стеке
    m_rightSideStackedLayout->setCurrentWidget(m_profileViewWidget);
}


void MainWindow::hideProfileView()
{
    qDebug() << "[CLIENT] Hiding profile, returning to chat view.";

    // Если есть активный собеседник — возвращаемся в чат
    if(m_dataService->getCurrentChatPartner() != nullptr){
        m_rightSideStackedLayout->setCurrentWidget(m_chatViewWidget);
    } else{
        // Если ни с кем не ведём переписку — показываем заглушку
        m_rightSideStackedLayout->setCurrentWidget(m_placeholderWidget);
    }
}


void MainWindow::showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName)
{
    // Текст вопроса для диалога
    QString questionText = QString("Пользователь %1 (@%2) хочет добавить вас в список контактов. Принять запрос?")
                               .arg(fromDisplayName, fromUsername);

    // Отображаем диалог с Yes/No
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Запрос на добавление в контакты", questionText,
                                  QMessageBox::Yes | QMessageBox::No);

    // Готовим JSON-ответ серверу
    QJsonObject contactResponse;
    contactResponse["type"] = "contact_request_response";
    contactResponse["fromUsername"] = fromUsername;  // кому адресуем ответ

    // "accepted" если пользователь согласился, "declined" — если отказал
    if (reply == QMessageBox::Yes) {
        contactResponse["response"] = "accepted";
    } else {
        contactResponse["response"] = "declined";
    }

    // Отправляем результат на сервер
    m_networkService->sendJson(contactResponse);
}


void MainWindow::onRequestAccepted(const QJsonObject& request) {
    QJsonObject contactResponse;
    contactResponse["type"] = "contact_request_response";    // Тип ответа на запрос
    contactResponse["fromUsername"] = request["fromUsername"]; // От кого пришла заявка

    contactResponse["response"] = "accepted"; // Решение пользователя
    m_networkService->sendJson(contactResponse); // Ответ на сервер
}


void MainWindow::onRequestRejected(const QJsonObject& request) {
    QJsonObject contactResponse;
    contactResponse["type"] = "contact_request_response";
    contactResponse["fromUsername"] = request["fromUsername"];

    contactResponse["response"] = "declined";
    m_networkService->sendJson(contactResponse);
}


void MainWindow::updateUserList()
{
    if(m_dataService->getUserCache() == nullptr){
        return;
    }
    // Берём всех пользователей
    QList<User> users = (*m_dataService->getUserCache()).values();

    // Сортируем по displayName (без учёта регистра)
    std::sort(users.begin(), users.end(), [](const User& a, const User& b) {
        return a.displayName.toLower() < b.displayName.toLower();
    });

    // Извлекаем usernames по отсортированному списку
    QStringList usernames;
    for (const User& user : users) {
        usernames.append(user.username);
    }

    // Обновляем модель для отображения в UserListView
    m_contactModel->updateContacts(usernames);
}


void MainWindow::onGlobalSearchTriggered()
{
    QString query = m_searchLineEdit->text().trimmed();

    // Если запрос пустой — скрыть popup и выйти
    if (query.isEmpty()) {
        if (m_searchResultsPopup && m_searchResultsPopup->isVisible()) {
            m_searchResultsPopup->hide();
        }
        return;
    }

    qDebug() << "[CLIENT] Triggered global user search for:" << query;

    // Формируем JSON-запрос поиска пользователей
    QJsonObject request;
    request["type"] = "search_users";
    request["term"] = query;
    m_networkService->sendJson(request);
}


bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Скрываем pop-up поиска, если клик был вне его области
    if (event->type() == QEvent::MouseButtonPress && m_searchResultsPopup->isVisible()) {
        if (!m_searchResultsPopup->geometry().contains(QCursor::pos())) {
            m_searchResultsPopup->hide();
        }
    }
    // По закрытию окна — завершаем звонок
    if (event->type() == QEvent::Close) {
        m_callService->endCall();
    }

    // Для остальных событий — стандартная обработка MainWindow
    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::onChatScroll(int value)
{
    // Пропустить, если скролл программный или идёт загрузка истории
    if (m_programmaticScrollInProgress || *(m_dataService->getIsLoadingHistory())) {
        return;
    }

    // Если дошёл до самого верха и есть ещё сообщения для подгрузки
    if (value == 0 && *(m_dataService->getOldestMessageId()) != 0) {
        QScrollBar* scrollBar = m_chatViewWidget->chatHistoryView()->verticalScrollBar();

        m_expectingRangeChange = true;       // Будем ждать изменения диапазона (rangeChanged)
        m_oldScrollMax = scrollBar->maximum(); // Запомним oldMax для корректной прокрутки

        qDebug() << "[SCROLL] Scrolled to top. Expecting range change. Old max:" << m_oldScrollMax;

        *(m_dataService->getIsLoadingHistory()) = true; // Включаем флаг загрузки

        // Собираем запрос догрузки истории для сервера
        QJsonObject request;
        request["type"] = "get_history";
        request["with_user"] = m_dataService->getCurrentChatPartner()->username;
        request["before_id"] = *(m_dataService->getOldestMessageId());
        m_networkService->sendJson(request);
    }
}


void MainWindow::onSendMessageReadReceipt(qint64 messageId)
{
    qDebug() << "[CLIENT] Received receipt signal for message ID:" << messageId << ". Sending to server.";

    // JSON для read-receipt
    QJsonObject readCmd;
    readCmd["type"] = "message_read";
    readCmd["id"] = (double)messageId;

    // Локально отмечаем сообщение как прочитанное
    onMessageStatusChanged(messageId, ChatMessage::MessageStatus::Read);

    // Отправляем на сервер
    m_networkService->sendJson(readCmd);
}


void MainWindow::onRequestServerHistory(const QString& chatPartner, int afterId) {
    qDebug() << "[MainWindow] Requesting server history for" << chatPartner << "after id" << afterId;

    QJsonObject request;
    request["type"] = "get_history";
    request["with_user"] = chatPartner;
    request["after_server_id"] = afterId;

    m_networkService->sendJson(request);
}


MainWindow::~MainWindow()
{
    qDebug() << "[DEBUG] MainWindow destructor called";
    delete ui;
}


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "chatfilterproxymodel.h" // (Сейчас не используется, но оставлено как заготовка)
#include "structures.h"

// Прямые объявления (Forward Declarations) для классов Qt.
// Позволяют использовать указатели на них без включения полных .h файлов,
// что ускоряет компиляцию и разрывает циклические зависимости.
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTcpSocket;
class QJsonObject;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QTimer;
class QPoint;
class QListView;
class ContactListModel;
class ContactListDelegate;
QT_END_NAMESPACE

// Прямые объявления кастомных классов приложения.
class LoginWidget;
class ChatViewWidget;
class ProfileViewWidget; // (Заготовка)
class ChatMessageModel;
class SearchResultsPopup;

/**
 * @class MainWindow
 * @brief Главное окно и центральный контроллер ("мозг") всего клиентского приложения.
 *
 * Этот класс выполняет роль "дирижера", координируя работу всех остальных компонентов:
 * - Управляет сетевым соединением и обрабатывает протокол обмена данными с сервером.
 * - Хранит всё состояние приложения (кэши, текущего пользователя, текущий чат).
 * - Собирает основной пользовательский интерфейс из составных виджетов.
 * - Связывает сигналы от UI-элементов со слотами бизнес-логики.
 * @author kex1tu
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна.
     * @param parent Родительский виджет, по умолчанию `nullptr`.
     */
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //=========================================================================
    /// @name Публичные геттеры
    /// @brief Предоставляют моделям и делегатам контролируемый доступ (только для чтения)
    ///        к централизованным кэшам данных, сохраняя MainWindow как "единственный источник правды".
    /// @{
    //=========================================================================
    User getUserFromCache(const QString& username) const;
    const QMap<QString, ChatCache>& getChatCache() const;
    const QMap<QString, int>& getUnreadCounts() const;
    User getCurrentChatPartner() const;
    /// @}

protected:
    /**
     * @brief Перехватывает события для других объектов.
     * @details Используется для закрытия всплывающего окна `m_searchResultsPopup`
     *          при клике мышью в любом другом месте приложения.
     * @param watched Объект, за которым наблюдаем.
     * @param event Произошедшее событие.
     * @return bool `true` если событие было обработано, иначе `false`.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    //=========================================================================
    /// @name Слоты для обработки сетевых событий
    /// @{
    //=========================================================================
    void onConnected();      ///< Вызывается при успешном подключении к серверу.
    void onDisconnected();   ///< Вызывается при разрыве соединения с сервером.
    void onReadyRead();      ///< Главный слот для чтения и парсинга всех входящих данных от сервера.
    /// @}

    //=========================================================================
    /// @name Слоты для обработки действий пользователя в UI
    /// @{
    //=========================================================================
    void onLoginRequested(const QString& username, const QString& password);
    void onRegisterRequested(const QString& username, const QString& displayName, const QString& password);
    void onSendMessageRequested(const QString& text);
    void onUserSelectionChanged(const QModelIndex &current);
    void onLogoutButtonClicked();
    void onAddContactRequested(const QString& username);
    void onEditMessageRequested(qint64 messageId, const QString& oldText);
    void onDeleteMessageRequested(qint64 messageId);
    void onChatSearchTriggered(const QString &text); // (Заготовка)
    void showProfileView();                           // (Заготовка)
    void onReplyToMessage(qint64 messageId);
    void onSendMessageReadReceipt(qint64 messageId);
    void onGlobalSearchTriggered();
    void onChatScroll(int value);
    void onTypingNotificationFired();
    /// @}

    //=========================================================================
    /// @name Внутренние слоты и обработчики
    /// @{
    //=========================================================================
    void handleUnreadCounts(const QJsonObject& response);
    void updateMessageStatusInCacheAndModel(qint64 messageId, ChatMessage::MessageStatus newStatus);
    void processVisibleMessages(); ///< Проверяет видимые сообщения и помечает их как прочитанные.
    /// @}


signals:
    /**
     * @brief Сигнал, который отправляется в ChatViewWidget.
     * @details Эмитируется, когда приходит новое сообщение для текущего чата,
     *          но пользователь прокрутил историю вверх и не видит его.
     *          Запускает показ кнопки "прокрутить вниз" и счетчика.
     */
    void newMessageForCurrentChat();

private:
    //=========================================================================
    /// @name Методы инициализации и управления приложением
    /// @{
    //=========================================================================
    void buildMainUI();        ///< Собирает и компонует основной интерфейс из виджетов.
    void setupConnections();   ///< Устанавливает все `connect` между объектами.
    void initResponseHandlers(); ///< Заполняет карту `m_responseHandlers` для обработки команд сервера.
    void connectToServer();      ///< Инициирует подключение к серверу.
    void resetApplicationState();///< Сбрасывает все кэши и состояние при выходе из аккаунта.
    /// @}

    //=========================================================================
    /// @name Механизм обработки команд сервера (Command Pattern)
    /// @{
    //=========================================================================
    /**
     * @brief Псевдоним для указателя на метод-обработчик команды.
     * @details Позволяет хранить указатели на методы в `QMap` для динамического вызова.
     */
    using ResponseHandler = void (MainWindow::*)(const QJsonObject&);

    /**
     * @brief Карта, сопоставляющая строковую команду от сервера с методом-обработчиком.
     */
    QMap<QString, ResponseHandler> m_responseHandlers;

    // --- Набор методов-обработчиков для каждой команды от сервера ---
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
    /// @}

    //=========================================================================
    /// @name Вспомогательные приватные методы
    /// @{
    //=========================================================================
    void sendJson(const QJsonObject& json);
    void updateUserList();
    void showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName);
    void updateContactItem(const QString& username);
    QString formatLastSeen(const User &user);
    void updateChatHeader(); // (Не используется, можно удалить)
    /// @}

private:
    //=========================================================================
    /// @name Члены класса: состояние, кэши и указатели на UI
    /// @{
    //=========================================================================

    //--- UI ---
    Ui::MainWindow *ui;         ///< Указатель на UI-форму из Qt Designer.

    //--- Сеть ---
    QTcpSocket *socket;         ///< Главный сокет для связи с сервером.
    quint32 m_nextBlockSize;    ///< Для корректного чтения TCP-пакетов переменной длины.

    //--- Кэши данных (Single Source of Truth) ---
    QMap<QString, ChatCache> m_chatHistoryCache; ///< Кэш истории сообщений для каждого чата.
    QMap<QString, User> m_userCache;             ///< Кэш данных о пользователях (displayName, lastSeen и т.д.).
    QMap<QString, int> m_unreadCounts;           ///< Кэш счетчиков непрочитанных сообщений.

    //--- Текущее состояние приложения ---
    QString m_currentUsername;        ///< Имя пользователя текущей сессии.
    User m_currentChatPartner;        ///< Данные собеседника в активном чате.
    bool m_isLoadingHistory = false;  ///< Флаг, предотвращающий повторные запросы истории при прокрутке.
    qint64 m_oldestMessageId = 0;     ///< ID самого старого сообщения в чате (для пагинации).
    qint64 m_editingMessageId = 0;    ///< ID сообщения, которое редактируется в данный момент.
    qint64 m_replyToMessageId = 0;    ///< ID сообщения, на которое создается ответ.

    //--- Указатели на основные виджеты ---
    LoginWidget* m_loginWidget;
    QWidget* m_mainChatWidget;

    //--- Компоненты левой панели ---
    QWidget* m_chatListPanel;
    QLineEdit* m_searchLineEdit;
    QListView* m_userListView;         // Заменил QListWidget
    ContactListModel* m_contactModel;  // Новая модель для списка контактов
    QPushButton* m_logoutButton;

    //--- Компоненты правой панели ---
    QWidget* m_rightSideContainer;
    QStackedLayout* m_rightSideLayout;
    QWidget* m_placeholderWidget;
    ChatViewWidget* m_chatViewWidget;
    ProfileViewWidget* m_profileViewWidget; // (Заготовка)

    //--- Модели данных ---
    ChatMessageModel* m_chatModel; // Модель для сообщений в *текущем* чате.

    //--- Вспомогательные виджеты и таймеры ---
    SearchResultsPopup* m_searchResultsPopup;
    QTimer* m_globalSearchTimer;
    QTimer* m_typingSendTimer;
    QMap<QString, QTimer*> m_typingReceiveTimers;

    bool m_isChatSearchActive = false; // (Заготовка)
    /// @}
};

#endif // MAINWINDOW_H

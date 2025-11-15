#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>       
#include <QTimer>
#include "chatfilterproxymodel.h"
#include "structures.h"
#include "networkservice.h"
#include "core/callservice.h"
#include "ui/callwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QStackedWidget;
class QTcpSocket;
class QJsonObject;
class QLineEdit;
class QPushButton;
class QStackedLayout;
class QTimer;
class QPoint;
class QListView;
class QToolButton;
class QMenu;
class QAction;
class QVBoxLayout;
class QWidget;
class ContactListModel;
class ContactListDelegate;
class ChatMessageDelegate;
class LoginWidget;
class ChatViewWidget;
class ProfileViewWidget;
class ChatMessageModel;
class ChatFilterProxyModel;
class SearchResultsPopup;
class DataService;
class IncomingRequestsWidget;
class CallHistoryWidget;

QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
     
    static DataService* m_dataService;
    /**
 * @brief Конструктор главного окна приложения: инициализация сервисов, виджетов, моделей, подключение сигналов/слотов, установка параметров, запуск подключения к серверу.
 * @param dataService Сервис данных (передаётся из main)
 * @param parent Родительский виджет (обычно nullptr)
 */
    MainWindow(DataService* dataService, QWidget *parent = nullptr);
    /**
 * @brief Деструктор главного окна (освобождает память, удаляет UI).
 */
    ~MainWindow();
    ChatViewWidget& getChatViewWidget();
    ChatMessageModel& getChatMessageModel();

public slots:

    /**
 * @brief Переключает окно между полноэкранным и оконным режимом.
 *
 * - Если уже fullscreen — возвращает к обычному размеру.
 * - Если не fullscreen — разворачивает окно на весь экран.
 */
    void toggleFullScreen();

protected:
    /**
 * @brief Глобальный обработчик событий для MainWindow (фильтр событий).
 *
 * В обработчике:
 * - Если щёлкнули мышью вне области pop-up поиска, он автоматически скрывается.
 * - При событии закрытия главного окна завершаем звонок (если идет).
 * - Все остальные события пропускает дальше.
 *
 * @param watched Объект, на котором произошло событие
 * @param event Событие Qt (QEvent)
 * @return true если событие полностью обработано здесь, иначе — обрабатывается стандартными средствами.
 */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
 * @brief Обработка нажатия клавиш в главном окне (функциональные F5/F6).
 *
 * - F5: проиграть звуковую гамму через CallService (например, для проверки звука).
 * - F6: тестирование диапазона частот (например, для калибровки микрофона/динамика).
 * - Все остальные клавиши перенаправляются в базовый обработчик QMainWindow.
 *
 * @param event QKeyEvent событие клавиатуры.
 */
    void keyPressEvent(QKeyEvent *event);
private slots:
    /**
 * @brief Слот: новый JSON-ответ от сервера — направляет в DataService для разбора.
 *
 * - Любой ответ с backend передаётся сервису данных, который уже разбирает его по типу и вызывает нужные слоты/изменения данных в приложении.
 * @param response Серверный ответ (QJsonObject)
 */
    void onJsonReceived(const QJsonObject& response);
    /**
 * @brief Слот при изменении (обновлении) списка контактов.
 * @param sortedUsernames Отсортированный список username'ов для обновлённой модели контактов.
 */
    void onContactsUpdated(const QStringList& sortedUsernames);
    /**
 * @brief Слот при изменении статусов online пользователей: обновляет модель на основании кэша данных.
 */
    void onOnlineStatusUpdated();
    /**
 * @brief Слот: добавляет порцию старых сообщений в начало истории (post-scroll up).
 * @param chatPartner Имя чата, для которого догрузили историю
 * @param messages Список старых сообщений (для prepend)
 */
    void onOlderHistoryChunkPrepended(const QString& chatPartner, const QList<ChatMessage>& messages);
    /**
 * @brief Слот: полностью загружена новая история чата, подгружается в модель, скролл — в самый низ.
 * @param chatPartner Юзернейм собеседника
 * @param messages Полная история сообщений (replace all)
 */
    void onHistoryLoaded(const QString& chatPartner, const QList<ChatMessage>& messages);

    /**
 * @brief Слот по успешному входу пользователя. Сохраняет профиль, очищает поля, переключает UI на чаты, инициирует подгрузку истории сообщений.
 * @param response Серверный JSON-ответ с данными пользователя
 */
    void onLoginSuccess(const QJsonObject& response);
    /**
 * @brief Слот по неудачному входу — показывает предупреждение пользователю.
 * @param reason Причина ошибки (текст)
 */
    void onLoginFailure(const QString& reason);
    /**
 * @brief Слот по успешной регистрации — показывает информационное сообщение.
 */
    void onRegisterSuccess();
    /**
 * @brief Слот по ошибке регистрации — показывает сообщение пользователю.
 * @param reason Причина ошибки (текст)
 */
    void onRegisterFailure(const QString& reason);
    /**
 * @brief Слот: успешный логаут пользователя — показывает сообщение и сбрасывает состояние приложения.
 */
    void onLogoutSuccess();
    /**
 * @brief Слот: ошибка выхода из аккаунта.
 * @param reason Причина ошибки
 */
    void onLogoutFailure(const QString& reason);

    /**
 * @brief Слот: обработка получения нового сообщения (реального времени или из истории).
 * Обновляет unread, модель чата, уведомляет пользователя о новых сообщениях.
 * @param incomingMsg Структура ChatMessage (новое сообщение)
 */
    void onNewMessageReceived(const ChatMessage& incomingMsg);
    /**
 * @brief Слот: изменение статуса сообщения (доставлено, прочитано и др.).
 * @param messageId ID сообщения
 * @param newStatus Новый статус сообщения (enum)
 */
    void onMessageStatusChanged(qint64 messageId, ChatMessage::MessageStatus newStatus);
    /**
 * @brief Слот: изменение количества непрочитанных сообщений, обновляет список контактов.
 */
    void onUnreadCountChanged();
    /**
 * @brief Слот: сообщение было отредактировано — обновление текста и контакта.
 * @param chatPartner Имя собеседника
 * @param messageId ID редакитуемого сообщения
 * @param newPayload Новый текст сообщения
 */
    void onMessageEdited(const QString& chatPartner, qint64 messageId, const QString& newPayload);
    /**
 * @brief Слот: сообщение было удалено — обновление модели и контакта.
 * @param chatPartner Имя собеседника
 * @param messageId ID удаляемого сообщения
 */
    void onMessageDeleted(const QString& chatPartner, qint64 messageId);
    /**
 * @brief Слот: подтверждение отправки сообщения, обновляет статус сообщения.
 * @param tempId Временный ID сообщения
 * @param msg Структура отправленного сообщения
 */
    void onConfirmMessageSent(QString tempId, const ChatMessage& msg);

    /**
 * @brief Слот: приходят результаты поиска пользователей для SearchResultsPopup.
 * @param users Массив QJsonArray с найденными пользователями
 */
    void onSearchResultsReceived(const QJsonArray& users);
    /**
 * @brief Слот: удачное добавление контакта — показывает messagebox пользователю.
 * @param username Имя добавленного пользователя
 */
    void onAddContactSuccess(const QString& username);
    /**
 * @brief Слот: ошибка при добавлении контакта.
 * @param reason Причина ошибки
 */
    void onAddContactFailure(const QString& reason);
    /**
 * @brief Слот: обновление списка ожидающих запросов в друзья (от других пользователей).
 * @param requests QJsonArray запросов
 */
    void onPendingContactRequestsUpdated(const QJsonArray& requests);
    /**
 * @brief Слот: пользователь пытается выполнить вход (авторизация).
 *
 * Детали:
 * - Проверяет, чтобы поля логина и пароля не были пустыми (иначе — предупреждение через MessageBox)
 * - Формирует JSON-запрос типа "login" с введёнными username и password.
 * - Отправляет на сервер через NetworkService (типичный сценарий REST-like).
 *
 * @param username Имя пользователя для входа
 * @param password Пароль
 */
    void onLoginRequested(const QString& username, const QString& password);
    /**
 * @brief Слот: пользователь отправил заявку на регистрацию нового аккаунта.
 *
 * Ход работы:
 * - Проверяет, что все поля заполнены (username, displayName, password). Если нет — предупреждение.
 * - Формирует JSON-запрос типа "register", передаёт его через NetworkService.
 *
 * @param username Имя пользователя (логин)
 * @param displayName Отображаемое имя
 * @param password Пароль
 */
    void onRegisterRequested(const QString& username, const QString& displayName, const QString& password);
    /**
 * @brief Слот: обработка запроса на отправку сообщения из чата (или редактирования существующего).
 *
 * Логика:
 * - Если текст пустой или не выбран собеседник — не делает ничего.
 * - Если в режиме редактирования (есть ID) — формируется и отправляется edit_message.
 * - Иначе формируется новое сообщение (ChatMessage), добавляется в модель и БД, отправляется на сервер.
 * - После отправки сбрасывается reply-состояние, обновляется контакт.
 *
 * @param text Текст нового или редактируемого сообщения
 */
    void onSendMessageRequested(const QString& text);
    /**
 * @brief Слот: обработка выбора пользователя в списке контактов.
 *
 * Алгоритм:
 * - Если аргумент некорректен (ничего не выбрано), сбрасывает правую панель, текущего собеседника, reply UI, кеши и выделения. Полное обнуление состояния чата.
 * - Если выбран контакт, но был активен reply — сбрасывает reply-состояние.
 * - Сбрасывает счетчик непрочитанных для выбранного пользователя, обновляет контакт.
 * - Проверяет на наличие пользователя в кеше данных. Если нет — warning и выход.
 * - Устанавливает нового собеседника, сбрасывает состояние пагинации и подгружает историю чата.
 * - Полностью очищает модель сообщений, кеши делегата, обновляет header.
 * - Включает chatView, триггерит синхронизацию истории, выставляет scroll/перерисовку видимых сообщений.
 *
 * @param current Индекс модели контактов для выбранного пользователя.
 */
    void onUserSelectionChanged(const QModelIndex &current);
    /**
 * @brief Слот: обработка нажатия кнопки "Выйти" (logout).
 *
 * - Логирует событие.
 * - Формирует JSON-запрос типа "logout_request" с текущим пользователем.
 * - Отправляет на сервер через NetworkService.
 */
    void onLogoutButtonClicked();
    /**
 * @brief Слот: ручной запрос на добавление контакта (например, из SearchResultsPopup).
 *
 * - Прячет PopUp поиска.
 * - Выводит диалог (MB) с вопросом-подтверждением.
 * - Если пользователь согласен, формирует и отправляет add_contact_request на сервер.
 *
 * @param username Имя пользователя, кому отправляем запрос
 */
    void onAddContactRequested(const QString& username);
    /**
 * @brief Слот: пользователь запросил редактирование сообщения.
 *
 * - Вызывается, когда пользователь кликает "редактировать" в истории чата.
 * - Скрывает UI-реплая (reply).
 * - Заполняет dataService ID редактируемого сообщения.
 * - Переводит ChatViewWidget в режим редактирования, выставляет старый текст.
 *
 * @param messageId ID сообщения для редактирования
 * @param oldText Предыдущее содержимое сообщения
 */
    void onEditMessageRequested(qint64 messageId, const QString& oldText);
    /**
 * @brief Слот: пользователь запросил удаление сообщения.
 *
 * - Формирует JSON-запрос на удаление.
 * - Отправляет запрос на сервер через NetworkService.
 *
 * @param messageId ID сообщения для удаления
 */
    void onDeleteMessageRequested(qint64 messageId);
    /**
 * @brief Слот-заглушка для поиска — если поиск не реализован, просто игнорируется.
 * @param text Введённый текст поиска
 */
    void onChatSearchTriggered(const QString &text);
    /**
 * @brief Слот: пользователь выбрал сообщение для ответа (reply).
 *
 * Действия:
 * - Логирует ID выбранного сообщения.
 * - Находит нужный ChatMessage в модели по ID.
 * - Сохраняет ID реплая в DataService.
 * - Показывает reply UI в чате (от кого и preview текста).
 *
 * @param messageId ID сообщения, на которое будет отправлен ответ
 */
    void onReplyToMessage(qint64 messageId);
    /**
 * @brief Слот: отправка read-receipt для сообщения (получено, просмотрено).
 *
 * - Формирует команду "message_read" для сервера.
 * - Локально меняет статус сообщения на Read.
 *
 * @param messageId ID сообщения, для которого отправляем receipt.
 */
    void onSendMessageReadReceipt(qint64 messageId);
    /**
 * @brief Слот: срабатывание глобального поиска пользователей по введённому запросу.
 *
 * — Получает trimmed-текст из строки поиска.
 * — Если поиск пустой, скрывает pop-up результатов.
 * — Формирует запрос "search_users" с поисковым термином и отправляет на сервер.
 */
    void onGlobalSearchTriggered();
    /**
 * @brief Слот: обработка прокрутки истории чата (ScrollBar).
 *
 * - Не реагирует при программной прокрутке или когда идёт догрузка истории.
 * - Если скролл-бар в самом верху и есть ещё предыдущая история (oldestMessageId != 0):
 *   - Запоминает текущее максимальное значение scrollBar (для коррекции после догрузки).
 *   - Ставит флаг ожидания rangeChange (изменения диапазона скролла).
 *   - Генерирует запрос "get_history" для сервера, чтобы подгрузить предыдущие сообщения.
 *
 * @param value Текущее значение скролла (0 — самый верх).
 */
    void onChatScroll(int value);
    /**
 * @brief Слот: обработка события "пользователь начал/продолжает печатать".
 *
 * Алгоритм и сдержки:
 * - Не отправляет "typing", если нет активного собеседника или поле ввода пусто.
 * - Не спамит событиями: если таймер уже запущен — не посылает повторно.
 * - Формирует JSON и отправляет на сервер ("type" = "typing", "toUser" = текущий собеседник).
 * - После отсылки запускает таймер, чтобы отправить следующее уведомление не чаще заданного периода.
 */
    void onTypingNotificationFired();
    /**
 * @brief Слот: изменился статус "печатает" для контакта.
 * @param username Контакт
 * @param isTyping True — пользователь печатает
 */
    void onTypingStatusChanged(const QString& username, bool isTyping);
    /**
 * @brief Слот: подключение к серверу выполнено, интерфейс активируется.
 *
 * - Показывает уведомление "Подключено" в статусбаре (на 2 секунды).
 * - В debug-лог выводит состояние указателя на LoginWidget.
 * - Если LoginWidget не создан — предупреждает и выходит. Иначе активирует поля ввода на экране логина.
 */
    void onConnected();
    /**
 * @brief Слот: соединение с сервером разорвано — блокируем поля логина.
 *
 * - Показывает "Отключено" в статусбаре (навсегда до переподключения).
 * - Блокирует поля логина для предотвращения новых попыток до восстановления соединения.
 */
    void onDisconnected();
    /**
 * @brief Скролл к первому непрочитанному сообщению в текущем чате.
 *
 * Детали работы:
 * - Ищет индекс первого непрочитанного сообщения через модель чата.
 * - Преобразует индекс модели с помощью прокси-фильтра (если поиск активен).
 * - Просит виджет истории чата проскроллить к найденному сообщению.
 */
    void onScrollToUnread();
    /**
 * @brief Скролл к самому низу — показа последнего сообщения в чате.
 *
 * — Просто вызывает scrollToBottom у виджета истории сообщений.
 */
    void onScrollToBottom();
    /**
 * @brief Слот: реакция на изменение диапазона scrollBar при подгрузке истории (для корректного смещения скролла).
 */
    void onScrollBarRangeChanged();
    /**
 * @brief Слот: обработка нажатия "Принять" на входящую заявку в друзья.
 *
 * - Формирует JSON-ответ с "accepted" для данного пользователя.
 * - Отправляет ответ backend-у через NetworkService.
 * @param request QJsonObject с данными пришедшей заявки (fromUsername и др.)
 */
    void onRequestAccepted(const QJsonObject& request);
    /**
 * @brief Слот: обработка нажатия "Отклонить" на входящую заявку в друзья.
 *
 * - Формирует JSON-ответ с "declined" для данного пользователя.
 * - Отправляет ответ backend-у через NetworkService.
 * @param request QJsonObject с данными пришедшей заявки (fromUsername и др.)
 */
    void onRequestRejected(const QJsonObject& request);

    /**
 * @brief Инициация звонка по текущему собеседнику из панели чата.
 *
 * Логика:
 * - Получает имя активного собеседника (username).
 * - Если неизвестен — пишет warning.
 * - Иначе вызывает метод initiateCall сервисa звонков.
 *
 * @note Если сервис звонков не инициализирован — пишет в лог.
 */
    void onCallRequested();
    /**
 * @brief Слот: обработка клика по кнопке "Меню" в основном окне.
 */
    void onMenuButtonClicked();
    /**
 * @brief Слот: обработка кнопки "Calls"/Звонки.
 * Запрашивает историю звонков у сервиса, переключает на виджет истории звонков.
 */
    void onCallsButtonClicked();
    /**
 * @brief Слот: переход назад из меню (например, к чату).
 * Сбрасывает состояние профиля и выводит плейсхолдер справа.
 */
    void onBackFromMenu();
    /**
 * @brief Проверяет видимые сообщения в истории чата и отправляет read-уведомления для входящих сообщений.
 *
 * Алгоритм:
 * - Находит все сообщения, которые реально отображаются в viewport'е истории чата.
 * - Для каждого такого сообщения, если оно входящее и статус (Sent/Delivered), эмитит сигнал о необходимости read-уведомления.
 *
 * Используется при скроллинге, изменении истории, для реализации корректной read-логики ("прочитано" только для реально показанных на экране).
 */
    void processVisibleMessages();
    /**
 * @brief Открывает окно профиля для текущего выбранного собеседника.
 *
 * - Логирует имя собеседника
 * - Если никнейм пустой — ничего не делает
 * - Заполняет виджет профиля данными пользователя и показывает его в правой панели
 */
    void showProfileView();
    /**
 * @brief Скрывает виджет профиля и возвращает пользователя к чату (или placeholder, если чат не выбран).
 *
 * — Логирует действие.
 * — Если выбран собеседник — возвращает обратно в чат.
 * — Иначе показывает заглушку (нет переписки).
 */
    void hideProfileView();
    /**
 * @brief Слот при клике на кнопку "мой профиль" в меню.
 * Показывает профиль текущего пользователя в правой панели.
 */
    void onMyProfileClicked();
/**
 * @brief Обработка результата обновления профиля пользователя.
 *
 * Пошагово:
 * - Проверяет, был ли успешен запрос к серверу ("success").
 * - Если все ок — обновляет локальный объект User, синхронизирует с виджетом профиля, показывает уведомление.
 * - Если ошибка — показывает причину через MessageBox.
 *
 * @param response JSON-ответ от сервера:
 *   - "success" (bool): результат операции.
 *   - "display_name", "status_message": новые данные пользователя, если успешно.
 *   - "reason" (string): причина отказа (если неудачно).
 */
    void onProfileUpdateResult(const QJsonObject& response);
    /**
 * @brief Слот: ручной запрос истории сообщений с сервера (пагинация).
 *
 * - Генерирует JSON-запрос "get_history", отправляет на сервер (будет загружать сообщения с id > afterId).
 *
 * @param chatPartner Собеседник/контакт для истории
 * @param afterId ID сообщения, с которого ведём догрузку
 */
    void onRequestServerHistory(const QString& chatPartner, int afterId);

signals:
     
    void newMessageForCurrentChat();

private:
    /**
 * @brief Строит основную структуру UI окна: авторизация, список чатов, меню, правая панель (чат/профиль/звонки).
 *
 * — Инициализирует все виджеты основной страницы, связывает модели, подключает сигналы/слоты.
 * — Делает layout-сборку: левая панель (чаты) + правая панель (стек: чат, профиль, вызовы, заглушка).
 */
    void buildMainUI();
    /**
 * @brief Настраивает все сигналы/слоты между сервисами, виджетами и моделями главного окна.
 *
 * — Подключает основные UI события, логику обмена сообщениями, меню, чаты, звонки, обработку ошибок.
 * — Проверяет корректность инициализации сервисов и виджетов перед подключением.
 */
    void setupConnections();

    void initResponseHandlers();
    /**
 * @brief Полный сброс состояния приложения при выходе пользователя или восстановлении по ошибке.
 *
 * Этапы:
 * - Сброс данных звонков (CallService), очистка внутренних данных DataService (пользователи, сообщения, кеши).
 * - Очистка всех сообщений в модели чата и контактов.
 * - Очистка и возврат в начальное состояние UI (инпуты поиска, поля для сообщения, reply/edit bar скрываются).
 * - Прячем popup поиска, возвращаем панели и стэки UI к состоянию "вход/чат по умолчанию", отображаем лейаут авторизации.
 * - Логируем финальный статус в debug.
 */
    void resetApplicationState();
    /**
 * @brief Обновляет список контактов в UI: сортирует по displayName (без регистра), передаёт их в модель.
 *
 * - Читаем пользовательский кеш из DataService, если он пустой — выход.
 * - Все User приводим к списку, сортируем по displayName.
 * - Вытаскиваем usernames как QStringList (по порядку сортировки displayname).
 * - Передаём итоговый список в модель контактов.
 */
    void updateUserList();
    /**
 * @brief Диалог-подтверждение входящего запроса на добавление в контакты.
 *
 * - Формирует вопрос (QMessageBox): кто хочет добавить, какое имя и username.
 * - В зависимости от ответа — формирует JSON "contact_request_response" с результатом (accepted/declined).
 * - Отправляет ответ на сервер через NetworkService.
 *
 * @param fromUsername Имя пользователя (username), инициатора запроса
 * @param fromDisplayName Отображаемое имя инициатора
 */
    void showContactRequestPrompt(const QString& fromUsername, const QString& fromDisplayName);


    /**
 * @brief Строит и настраивает страницу меню (левая панель/меню приложения).
 *
 * — Добавляет кнопки: Back, Contacts, Calls, Incoming Requests, My Profile, Logout.
 * — Включает стилизацию, разделители, layout'ы, подключает слоты к каждой кнопке.
 */
    void setupMenuPage();
    /**
 * @brief Создаёт и настраивает кнопку меню с возможностью бейджа (отметки "new"/количества).
 * @param text Текст кнопки
 * @param badge Дополнительный бейдж (например "New" или число)
 * @return Готовый QPushButton с оформлением
 */
    QPushButton* createMenuButton(const QString& text, const QString& badge);


private:
    /**
     * @brief Автоматически сгенерированный UI класс из ui-файла (Qt Designer).
     */
    Ui::MainWindow *ui;

    /**
     * @brief Сетевой сервис для связи с сервером (отправка/приём запросов).
     */
    QPointer<NetworkService> m_networkService;

    /**
     * @brief Сервис звонков (аудио/видео), управление вызовами.
     */
    QPointer<CallService> m_callService;

    /**
     * @brief Виджет экрана логина (авторизация пользователя).
     */
    QPointer<LoginWidget> m_loginWidget;

    /**
     * @brief Главный виджет чата (левая и правая панели).
     */
    QPointer<QWidget> m_mainChatWidget;

    /**
     * @brief Левая панель с контактами и чатом.
     */
    QPointer<QWidget> m_chatListPanel;

    /**
     * @brief Правая часть (стек виджетов — чат, профиль, история и др.).
     */
    QPointer<QWidget> m_rightSideContainer;

    /**
     * @brief Заглушка (отображается, когда нет активного чата).
     */
    QPointer<QWidget> m_placeholderWidget;

    /**
     * @brief Строка поиска по контактам.
     */
    QPointer<QLineEdit> m_searchLineEdit;

    /**
     * @brief Список пользователей (контакт-лист).
     */
    QPointer<QListView> m_userListView;

    /**
     * @brief Кнопка выхода из аккаунта.
     */
    QPointer<QPushButton> m_logoutButton;

    /**
     * @brief Кнопка открытия меню (гамбургер).
     */
    QPointer<QToolButton> m_menuButton;

    /**
     * @brief Главное меню приложения.
     */
    QPointer<QMenu> m_mainMenu;

    /**
     * @brief Action для входящих запросов (меню).
     */
    QPointer<QAction> m_actionIncomingRequests;

    /**
     * @brief Layout для правой панели (вертикальный).
     */
    QPointer<QVBoxLayout> m_rightSideLayout;

    /**
     * @brief Стековый layout для правой панели (переключение между чат/профиль/история).
     */
    QPointer<QStackedLayout> m_rightSideStackedLayout;

    /**
     * @brief Виджет просмотра чата (переписка).
     */
    QPointer<ChatViewWidget> m_chatViewWidget;

    /**
     * @brief Виджет профиля пользователя/собеседника.
     */
    QPointer<ProfileViewWidget> m_profileViewWidget;

    /**
     * @brief Виджет входящих запросов.
     */
    QPointer<IncomingRequestsWidget> m_incomingRequestsWidget;

    /**
     * @brief Pop-up с результатами поиска по пользователям.
     */
    QPointer<SearchResultsPopup> m_searchResultsPopup;

    /**
     * @brief Виджет звонков (UI для входящего/исходящего звонка).
     */
    QPointer<CallWidget> m_callWidget;

    /**
     * @brief Модель сообщений текущего чата.
     */
    QPointer<ChatMessageModel> m_chatModel;

    /**
     * @brief Прокси для фильтрации сообщений (поиск и т.п.).
     */
    QPointer<ChatFilterProxyModel> m_chatFilterProxy;

    /**
     * @brief Модель контактов (список пользователей).
     */
    QPointer<ContactListModel> m_contactModel;

    /**
     * @brief Делегат для отображения сообщений чата (кастомные отрисовки).
     */
    QPointer<ChatMessageDelegate> m_chatDelegate;

    /**
     * @brief ID сообщения для программного скролла (scroll anchor).
     */
    qint64 m_scrollAnchorId = 0;

    /**
     * @brief Сохраняем максимальный диапазон скролла (для коррекции после загрузки истории).
     */
    int m_oldScrollMax = 0;

    /**
     * @brief Флаг, что сейчас scrollBar меняется программно, а не руками пользователя.
     */
    bool m_programmaticScrollInProgress = false;

    /**
     * @brief Флаг ожидания смены диапазона scrollBar (при догрузке истории).
     */
    bool m_expectingRangeChange = false;

    /**
     * @brief Стек виджетов для левой панели (чаты/меню).
     */
    QStackedWidget* m_leftMainPanel;

    /**
     * @brief Индексы страниц для в QStackedWidget: чаты и меню.
     */
    enum PageIndex {
        PAGE_CHATS = 0,
        PAGE_MENU = 1
    };

    /**
     * @brief Виджет страницы чатов (QStackedWidget).
     */
    QWidget* m_chatsPage;

    /**
     * @brief Виджет страницы меню (QStackedWidget).
     */
    QWidget* m_menuPage;

    /**
     * @brief Виджет истории звонков (отдельный стек).
     */
    CallHistoryWidget* m_callHistoryWidget;

};

#endif  

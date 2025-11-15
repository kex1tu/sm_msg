#ifndef DATASERVICE_H
#define DATASERVICE_H

#include <QObject>
#include <QMap>
#include <QJsonObject>
#include "structures.h"
#include <QTimer>
#include <databaseservice.h>

class DataService : public QObject
{
    Q_OBJECT
public:

    /**
 * @brief Конструктор класса DataService — инициализация сервисов, таймеров и обработчиков событий.
 *
 * - Инициализирует объект DatabaseService, выполняет подключение к базе данных.
 * - Логгирует предупреждение при сбое инициализации базы для дальнейшей диагностики.
 * - Создаёт и настраивает таймеры глобального поиска и отправки статуса "печатает" (singleShot, интервалы).
 * - Запускает инициализацию маршрутизаторов обработчиков событий (initResponseHandlers).
 *
 * @param parent Родительский QObject (например, для цепочки удаления/сигнальных маршрутов).
 */
    explicit DataService(QObject *parent = nullptr);

    // --- User State ---
    /**
 * @brief Возвращает указатель на текущего пользователя.
 * @return User* текущего пользователя
 */
    User* getCurrentUser();
    /**
 * @brief Возвращает указатель на текущего собеседника.
 * @return Указатель на User текущего чата
 */
    User* getCurrentChatPartner();
    /**
 * @brief Указатель на флаг загрузки истории.
 * @return bool* флаг активности загрузки истории
 */
    bool* getIsLoadingHistory();
    /**
 * @brief Указатель на идентификатор самого старого сообщения текущей переписки.
 * @return qint64* идентификатора
 */
    qint64* getOldestMessageId();

    /**
 * @brief Указатель на идентификатор редактируемого сообщения.
 * @return qint64* идентификатора
 */
    qint64* getEditinigMessageId();
    /**
 * @brief Указатель на идентификатор сообщения, на которое идёт ответ.
 * @return qint64* идентификатора
 */
    qint64* getReplyToMessageId();
    bool* getIsChatSearchActive();
    QVector<QString>* getUploadingFilePath();


    /**
 * @brief Таймер глобального поиска — возвращает указатель для управления таймингом поиска.
 * @return QTimer для глобального поиска
 */
    QTimer* getGlobalSearchTimer();
    /**
 * @brief Таймер отправки статуса «печатает» — управление циклом отправки.
 * @return QTimer отправки статуса «печатает»
 */
    QTimer* getTypingSendTimer();
    /**
 * @brief Возвращает указатель на мапу таймеров приёма статуса «печатает» для каждого юзера.
 * @return QMap таймеров, либо nullptr, если пусто
 */
    QMap<QString, QTimer*>* getTypingRecieveTimers();

    /**
 * @brief Получает указатель на пользователя из кеша по имени.
 *
 * - Проверяет наличие пользователя в мапе кеша.
 * - Возвращает указатель на пользователя, если найден, иначе nullptr (безопасная работа с памятью).
 *
 * @param username Имя пользователя
 * @return Указатель на найденного User, либо nullptr если не найден
 */
    User* getUserFromCache(const QString& username);
    /**
 * @brief Возвращает указатель на кеш истории чатов, если кеш не пустой.
 *
 * - Проверяет, не пуст ли кеш истории чатов.
 * - Если есть данные — возвращает указатель на QMap.
 * - Если нет данных — nullptr.
 *
 * @return Указатель на QMap истории чатов, либо nullptr
 */
    QMap<QString, ChatCache>* getChatCache();
    /**
 * @brief Возвращает указатель на мапу с числом непрочитанных, если есть данные.
 *
 * - Проверяет заполненность мапы.
 * - Если есть данные — возврат адреса структуры.
 * - Если нет — возврат основной структуры (безопасно, но пусто).
 *
 * @return Указатель на QMap с числом непрочитанных сообщений
 */
    QMap<QString, int>* getUnreadCounts();
    /**
 * @brief Возвращает кеш конкретного чата по имени пользователя.
 *
 * - Если в кеше есть этот чат — возращает указатель.
 * - Нет — nullptr.
 *
 * @param username Имя собеседника
 * @return Указатель на ChatCache, либо nullptr
 */
    ChatCache* getChatCacheForUser(const QString& username);
    /**
 * @brief Указатель на полную мапу кеша пользователей.
 * @return QMap закешированных User
 */
    QMap<QString, User> * getUserCache();
    /**
 * @brief Получает указатель на используемый сервис базы данных (DatabaseService).
 *
 * - Используется для доступа к основному сервису работы с SQLite или другой реализованной БД.
 * - Все операции по хранению/чтению сообщений, аудиту и миграциям идут через этот объект.
 * - Возвращает текущий объект DatabaseService для низкоуровневых операций.
 *
 * @return Указатель на DatabaseService
 */
    DatabaseService* getDatabaseService();

public slots:
    /**
 * @brief Обрабатывает входящий JSON-ответ, направляя его на соответствующий обработчик.
 *
 * - Извлекает тип события из JSON (поле "type").
 * - Поиск подходящего обработчика (slot) по типу в маршрутизаторе m_responseHandlers.
 * - Если найден — вызывает функцию-обработчик с передачей того же JSON.
 * - Если не найден обработчик — логгирует подробное предупреждение с указанием типа.
 *
 * @param response JSON-объект, полученный по сети
 */
    void processResponse(const QJsonObject& response);
    /**
 * @brief Обрабатывает успешную авторизацию — пробрасывает сигнал с всей полезной нагрузкой для UI и логики.
 *
 * @param response JSON-объект с деталями успешного входа
 */
    void handleLoginSuccess(const QJsonObject& response);
    /**
 * @brief Обрабатывает неудачную попытку входа — вызывает сигнал loginFailure с причиной ошибки.
 *
 * @param response JSON c полем "reason" (описание ошибки)
 */
    void handleLoginFailure(const QJsonObject& response);
    /**
 * @brief Обрабатывает успех регистрации — эмитит сигнал для UI/логики, тело ответа не используется.
 *
 * @param response JSON результата регистрации (не извлекается)
 */
    void handleRegisterSuccess(const QJsonObject& response);
    /**
 * @brief Обрабатывает неудачную регистрацию — сигнализирует ошибку с конкретной причиной.
 *
 * @param response JSON с полем "reason" (описание ошибки)
 */
    void handleRegisterFailure(const QJsonObject& response);
    /**
 * @brief Обрабатывает список контактов, пришедший с сервера.
 *
 * - Логгирует факт получения списка для дальнейшей диагностики.
 * - Извлекает массив пользователей из JSON, очищает локальный кеш для точной синхронизации.
 * - Проходит по каждому пользователю в массиве, извлекает и сохраняет свойства.
 * - Логгирует имена загруженных контактов по одному для аудита работы парсера.
 * - После загрузки — сортирует список контактов по displayName (без учёта регистра) для корректного интерфейса.
 * - Собирает список usernames и эмитит сигнал contactsUpdated, чтобы UI мог обновить "Контакты".
 *
 * @param response JSON-объект с массивом пользователей ("users")
 */
    void handleContactList(const QJsonObject& response);
    /**
 * @brief Обрабатывает новую (основную) историю чата, дополняет кеш и сигнализирует UI.
 *
 * - Извлекает массив истории сообщений и имя собеседника.
 * - Логгирует факт получения и длину истории (кол-во сообщений).
 * - Для каждого сообщения формирует ChatMessage, аккуратно выставляет статусы (read, delivered, sent) и прочие поля.
 * - Перед апдейтом кеша вызывает insertMessagesWithUpsertFiltered для корректного upsert в локальное хранилище.
 * - Добавляет в кеш только новые сообщения (по id).
 * - Обновляет глобальные флаги, если история для текущего чата и сигнализирует через historyLoaded.
 * - Если это не текущий чат, сообщает в лог о тихом обновлении кеша.
 *
 * @param response JSON-объект с историей ("history") и именем пользователя ("with_user")
 */
    void handleHistoryData(const QJsonObject& response);
    /**
 * @brief Обрабатывает получение старых сообщений истории чата и обновляет локальный кеш.
 *
 * - Извлекает имя пользователя для которого история (historyForUser), массив сообщений.
 * - Логгирует количество пришедших сообщений для истории удобства профилирования.
 * - Если история пуста, проставляет флаг allMessagesLoaded (все сообщения загружены), обновляет oldestMessageId,
 *   и эмитит сигнал с пустым списком, если чата текущий.
 * - Если есть сообщения:
 *   - Преобразует каждый элемент JSON в объект ChatMessage, выставляет статус флагов, сортирует направления.
 *   - Перед загрузкой – вызывает insertMessagesWithUpsertFiltered для фильтрации и upsert в локальное хранилище.
 *   - Препендит каждое сообщение в кеш чата, чтобы сохранить правильную хронологию.
 *   - Обновляет oldestMessageId для кеша и глобально, если чат текущий.
 *   - Если чат текущий — эмитит сигнал с новым куском истории, иначе просто пишет в лог о фоновом кешировании.
 *
 * @param response JSON-объект с массивом сообщений, ключом chat-партнера
 */
    void handleOldHistoryData(const QJsonObject& response);
    /**
 * @brief Обрабатывает входящее событие нового сообщения или echo отправленного (по temp_id).
 *
 * - Если в ответе есть temp_id (echo своего отправленного) — находит по temp_id сообщение:
 *   - Подтверждает серверный id через БД, логгирует изменение, обновляет кеш чата.
 *   - Эмитит confirmMessageSent, чтобы UI мог отразить подтверждение.
 *   - Обновляет локальный кеш отправителя (например, меняет статус с "отправлено" на "подтверждено сервером").
 *   - После echo дальнейшая обработка не требуется (return).
 * - Если temp_id нет — это новое входящее сообщение:
 *   - Формирует структуру ChatMessage с правильными статусами (read, delivered, sent).
 *   - Добавляет его в кеш чата и обновляет oldestMessageId (если нужно).
 *   - Сохраняет в БД, если возможно.
 *   - Логгирует все действия (сохранение, обработка).
 *   - Эмитит newMessageReceived, чтобы UI и остальные части системы обновились.
 *
 * @param response JSON-объект с данными сообщения
 */
    void handlePrivateMessage(const QJsonObject& response);
    /**
 * @brief Обновляет статус "онлайн" для всех пользователей из кеша на основании списка с сервера.
 *
 * - Извлекает массив usernames, создаёт QSet для быстрой проверки онлайна.
 * - Проходит по всему локальному кешу пользователей, отмечает online статус.
 * - По завершении эмитит сигнал onlineStatusUpdated для UI, логики и статистики.
 *
 * @param response JSON-объект с ключом "users" — массив пользователей в онлайне
 */
    void handleUserList(const QJsonObject& response);

    /**
 * @brief Обрабатывает событие изменения статуса сообщения на "доставлено" (delivered).
 *
 * - Извлекает идентификатор сообщения.
 * - Логгирует получение delivered-статуса для анализа сетевых событий.
 * - Обновляет статус в базе данных, если онлайн и возможно соединение.
 * - В локальном кеше чатов ищет соответствующее сообщение и обновляет статус, если требуется.
 * - Генерирует сигнал для UI/логики только если статус действительно изменился (не обновляет лишний раз).
 *
 * @param response JSON с ключом "id" — идентификатор сообщения для delivered
 */
    void handleMessageDelivered(const QJsonObject& response);
    /**
 * @brief Обрабатывает событие о прочтении сообщения (read).
 *
 * - Извлекает идентификатор сообщения.
 * - Логгирует успешное получение статуса о прочтении.
 * - Производит обновление статуса read в базе при наличии соединения.
 * - В локальном кеше обновляет статус сообщения и сигнализирует текущему диалогу, если статус реально изменился.
 *
 * @param response JSON с ключом "id" — идентификатор прочитанного сообщения
 */
    void handleMessageRead(const QJsonObject& response);
    /**
 * @brief Обрабатывает изменение содержимого сообщения (edit).
 *
 * - Извлекает chatPartner, идентификатор сообщения и новый payload.
 * - Логгирует полученную команду на редактирование (для аудита действий).
 * - Обновляет содержимое сообщения в базе через метод editMessage при наличии соединения.
 * - Находит сообщение в локальном кеше чата и обновляет payload и флаг isEdited.
 * - Логгирует изменения на обеих стадиях.
 * - Эмитит сигнал messageEdited с данными для UI и логики.
 *
 * @param response JSON с chatPartner, id и новым payload
 */
    void handleEditMessage(const QJsonObject& response);
    /**
 * @brief Обрабатывает удаление сообщения (delete) из чата.
 *
 * - Извлекает идентификатор сообщения и вычисляет реального chatPartner.
 * - Обновляет базу методом deleteMessage, если возможно соединение, и логгирует это.
 * - Логгирует получение команды на удаление сообщения.
 * - В кеше сообщений чата находит и удаляет сообщение по id.
 * - Эмитит сигнал messageDeleted, чтобы UI обновил отображение.
 *
 * @param response JSON с id удаляемого сообщения и with_user (для корректного выбора chatPartner)
 */
    void handleDeleteMessage(const QJsonObject& response);
    /**
 * @brief Обрабатывает результаты поиска пользователей по критерию.
 *
 * @param response JSON-объект с массивом найденных пользователей ("users")
 */
    void handleSearchResults(const QJsonObject& response);
    /**
 * @brief Обрабатывает успешное добавление контакта.
 *
 * @param response JSON с полем "username" — имя добавленного контакта
 */
    void handleAddContactSuccess(const QJsonObject& response);
    /**
 * @brief Обрабатывает ошибку добавления контакта (вызывает сигнал с причиной).
 *
 * @param response JSON с полем "reason"
 */
    void handleAddContactFailure(const QJsonObject& response);
    /**
 * @brief Обрабатывает входящий контакт-запрос (например, приглашение в друзья).
 *
 * @param response JSON-объект запроса
 */
    void handleIncomingContactRequest(const QJsonObject& response);
    /**
 * @brief Обрабатывает список всех входящих и ожидающих запросов на добавление контакта.
 *
 * @param response JSON с массивом запроса ("requests")
 */
    void handlePendingRequestsList(const QJsonObject& response);
    /**
 * @brief Обрабатывает успешный logout (выход из аккаунта), пробрасывает сигнал для UI и логики.
 *
 * @param response JSON, тело не используется
 */
    void handleLogoutSuccess(const QJsonObject& response);
    /**
 * @brief Обрабатывает ошибку выхода из аккаунта (logout), пробрасывает причину для UI и логики.
 *
 * @param response JSON с полем "reason"
 */
    void handleLogoutFailure(const QJsonObject& response);
    /**
 * @brief Обрабатывает событие "печатает" (typing) для пользователя, обновляя состояние и таймеры.
 *
 * - Извлекает имя пользователя-отправителя.
 * - Пропускает, если пользователя нет в кеше (может быть race-условие).
 * - Устанавливает флаг "печатает" и эмитит изменение для реакций UI.
 * - Для каждого пользователя создаёт timer, который сбрасывает флаг по истечении 2,5 секунд (если typing не повторился).
 * - Таймеры хранятся в мапе по имени для независимого контроля.
 *
 * @param response JSON с ключом "fromUser"
 */
    void handleTypingResponse(const QJsonObject& response);
    /**
 * @brief Обрабатывает получение списка непрочитанных сообщений по каждому чату.
 *
 * - Логгирует получение обновлённого списка непрочитанных сообщений с сервера.
 * - Очищает локальное хранилище мапы непрочитанных.
 * - Проходит по каждому элементу массива counts, вытаскивает username и count.
 * - Добавляет в локальное хранилище только те чаты, где есть хотя бы одно непрочитанное сообщение.
 * - В конце эмитит сигнал unreadCountChanged для реакции UI.
 *
 * @param response JSON, содержащий массив counts: [{ username, count }]
 */
    void handleUnreadCounts(const QJsonObject& response);

    // --- Call Handling ---
    /**
 * @brief Обрабатывает подтверждение факта успешной отправки запроса звонка.
 *
 * - Извлекает из JSON ответившего сервера идентификаторы пользователя и звонка.
 * - Логгирует событие о запуске исходящего звонка.
 * - Генерирует сигнал callRequestSent для всего приложения (UI и сетевой подсистемы).
 *
 * @param response JSON-ответ от сервера с информацией о звонке
 */
    void handleCallRequestSent(const QJsonObject& response);
    /**
 * @brief Обрабатывает поступление входящего звонка и передаёт параметры соответствующим модулям.
 *
 * - Извлекает параметры входящего вызова: кто звонит, идентификатор звонка, IP и порт звонящего.
 * - Логгирует происшествие для последующего анализа и отладки.
 * - Посылает сигнал incomingCall для вызова UI/систем соединения.
 *
 * @param response JSON-объект, описывающий входящий вызов.
 */
    void handleIncomingCall(const QJsonObject& response);
    /**
 * @brief Обрабатывает событие принятия звонка.
 *
 * - Извлекает параметры сессии (кто принял, ip и порт абонента для медиасоединения).
 * - Логгирует факт, параметры и диагностику акцепта.
 * - Генерирует сигнал callAccepted для других компонентов приложения.
 *
 * @param response JSON с данными о принятии вызова
 */
    void handleCallAccepted(const QJsonObject& response);
    /**
 * @brief Обрабатывает событие отклонения звонка.
 *
 * - Извлекает кто отклонил и причину отклонения.
 * - Логгирует причину и параметры для UI и анализа сценариев отклонения.
 * - Посылает сигнал callRejected.
 *
 * @param response JSON с деталями отклонения
 */
    void handleCallRejected(const QJsonObject& response);
    /**
 * @brief Обрабатывает завершение звонка (от любого участника).
 *
 * - Извлекает параметры завершившего и id сессии (если есть).
 * - Логгирует событие завершения для дальнейшей диагностики и истории.
 * - Посылает сигнал callEnded.
 *
 * @param response JSON c деталями завершения вызова
 */
    void handleCallEnd(const QJsonObject& response);
    /**
 * @brief Обрабатывает историю звонков: логгирует и отправляет массив данных подписчикам.
 *
 * - Получает массив звонков (JSON-массив) из response.
 * - Логгирует размер массива и каждую запись звонка с основными параметрами (исходящий/входящий, caller/callee, статус, длительность).
 * - Передаёт всю историю через сигнал callHistoryReceived для дальнейшей работы с UI.
 *
 * @param response JSON-объект с массивом истории вызовов
 */
    void handleCallHistory(const QJsonObject& response);
    /**
 * @brief Обрабатывает статистику по звонкам и передаёт её подписчикам.
 *
 * - Получает в response агрегированные данные по звонкам (исходящие, входящие, завершённые, пропущенные, общая длительность).
 * - Логгирует получение статистики и ключевые параметры для отладки и профилирования.
 * - После обработки данных посылает сигнал callStatsReceived, чтобы UI или логика могли их визуализировать.
 *
 * @param response JSON-объект с параметрами статистики по звонкам
 */
    void handleCallStats(const QJsonObject& response);
    /**
 * @brief Обрабатывает результат обновления профиля пользователя.
 *
 * - Получает JSON-ответ после попытки изменить/сохранить профиль пользователя на сервере.
 * - Генерирует сигнал profileUpdateResult для передачи данных ответа во все подписчики UI и других компонент.
 *
 * @param response JSON-объект, содержащий результат операции (успех/ошибка, новые значения и пр.)
 */
    void handleUpdateProfileResult(const QJsonObject& response);
    /**
 * @brief Запрашивает историю звонков у сервера.
 *
 * - Метод-заглушка. Реализация должна включать логику отправки соответствующего запроса на сервер для получения истории звонков.
 * - Может использоваться для явного обновления истории звонков по запросу пользователя.
 */
    void requestCallHistory();

    /**
 * @brief Синхронизирует историю чата — грузит локальные сообщения и запрашивает обновления с сервера.
 *
 * - Логгирует запуск синхронизации для выбранного собеседника.
 * - Выполняет SQL-запрос для загрузки всех сообщений с данным chatPartner (и текущим пользователем).
 * - Сохраняет результат в кеш истории для пользователя.
 * - Обновляет глобальный m_oldestMessageId на случай непустой истории.
 * - Эмитит сигнал, чтобы UI мог начально отобразить локальную историю чата.
 * - Находит lastId (самый свежий) для дальнейшего запроса обновлений с сервера.
 * - Логгирует все этапы: загрузку локальных сообщений и запрос на сервер.
 * - Эмитит сигнал requestServerHistory, чтобы послать запрос на обновление истории начиная с lastId.
 *
 * @param chatPartner Имя собеседника
 */
    void syncChatHistory(const QString& chatPartner);
    /**
 * @brief Возвращает ссылку на кеш актуального чата пользователя.
 *
 * - Позволяет обращаться к кешу по username и модифицировать его напрямую, не копируя структуру.
 *
 * @param username Имя пользователя-собеседника
 * @return Ссылка на структуру ChatCache для быстрого обращения
 */
    ChatCache& getChatCacheRef(const QString& username);
    /**
 * @brief Полностью очищает все данные, структуру состояния и таймеры DataService.
 *
 * - Очищает все структуры кеша истории, пользователей, непрочитанных.
 * - Ставит все ключевые переменные и флаги состояний в изначальное состояние.
 * - Удаляет все таймеры статуса "печатает" для корректной работы при смене пользователя.
 * - Логгирует факт полной очистки для отладки и профилировки.
 */
    void clearAllData();

signals:
    /**
     * @brief Сигнал о завершении загрузки и сортировке контактов.
     * @param sortedUsernames Отсортированные usernames (для интерфейса/листа контактов)
     */
    void contactsUpdated(const QStringList& sortedUsernames);

    /**
     * @brief Сигнал об обновлении статуса «онлайн» для всех пользователей (перерисовка UI).
     */
    void onlineStatusUpdated();

    /**
     * @brief Сигнал о подгрузке очередного чанка старых сообщений (например, при скролле наверх истории).
     * @param chatPartner Имя собеседника
     * @param messages Список новых (старых) сообщений
     */
    void olderHistoryChunkPrepended(const QString& chatPartner, const QList<ChatMessage>& messages);

    /**
     * @brief Сигнал о загрузке основной истории чата из БД или с сервера (для быстрого рендера UI).
     * @param chatPartner Имя собеседника
     * @param messages Список сообщений
     */
    void historyLoaded(const QString& chatPartner, const QList<ChatMessage>& messages);

    /**
     * @brief Сигнал об успешной авторизации пользователя — передаёт полезную нагрузку для UI, логики.
     * @param response JSON успешного входа
     */
    void loginSuccess(const QJsonObject& response);

    /**
     * @brief Сигнал о провале авторизации пользователя — причина для отображения и логирования.
     * @param reason Описание причины ошибки входа
     */
    void loginFailure(const QString& reason);

    /**
     * @brief Сигнал об успешной регистрации пользователя.
     */
    void registerSuccess();

    /**
     * @brief Сигнал о провале регистрации — причина для отображения и анализа логики.
     * @param reason Описание причины ошибки регистрации
     */
    void registerFailure(const QString& reason);

    /**
     * @brief Сигнал о поступлении нового сообщения в чат — сразу отдаёт полный объект.
     * @param message Структура поступившего сообщения
     */
    void newMessageReceived(const ChatMessage& message);

    /**
     * @brief Сигнал об изменении статуса конкретного сообщения (отправлено, доставлено, прочитано).
     * @param messageId Идентификатор сообщения
     * @param newStatus Новый статус сообщения
     */
    void messageStatusChanged(qint64 messageId, ChatMessage::MessageStatus newStatus);

    /**
     * @brief Сигнал о смене количества непрочитанных сообщений в чатах/собеседниках.
     */
    void unreadCountChanged();

    /**
     * @brief Сигнал о редактировании сообщения — для обновления UI.
     * @param chatPartner Имя собеседника
     * @param messageId Идентификатор редактируемого сообщения
     * @param newPayload Новое содержимое после редактирования
     */
    void messageEdited(const QString& chatPartner, qint64 messageId, const QString& newPayload);

    /**
     * @brief Сигнал об удалении сообщения — входит id для удаления в модели UI.
     * @param chatPartner Имя собеседника
     * @param messageId Идентификатор удаляемого сообщения
     */
    void messageDeleted(const QString& chatPartner, qint64 messageId);

    /**
     * @brief Сигнал с результатами поиска пользователей — для рендера в форме.
     * @param users Массив найденных пользователей (JSON)
     */
    void searchResultsReceived(const QJsonArray& users);

    /**
     * @brief Сигнал об успешном добавлении контакта (username для выделения/перехода).
     * @param username Имя добавленного контакта
     */
    void addContactSuccess(const QString& username);

    /**
     * @brief Сигнал о провале добавления контакта — причина для UI.
     * @param reason Текстовая причина ошибки
     */
    void addContactFailure(const QString& reason);

    /**
     * @brief Сигнал о поступлении входящего запроса на добавление в контакты.
     * @param request JSON-объект запроса
     */
    void contactRequestReceived(const QJsonObject& request);

    /**
     * @brief Сигнал об обновлении списка всех ожидающих запросов на добавление.
     * @param requests JSON-массив текущих запросов
     */
    void pendingContactRequestsUpdated(const QJsonArray& requests);

    /**
     * @brief Сигнал успешного выхода (logout) пользователя.
     */
    void logoutSuccess();

    /**
     * @brief Сигнал ошибки при logout — передаёт причину для UI.
     * @param reason Текст ошибки
     */
    void logoutFailure(const QString& reason);

    /**
     * @brief Сигнал смены статуса "печатает" для пользователя (например, показать в UI).
     * @param username Имя пользователя
     * @param isTyping Активен ли сейчас статус "печатает"
     */
    void typingStatusChanged(const QString& username, bool isTyping);

    /**
     * @brief Сигнал подтверждения отправки сообщения — echo по tempId, для обновления статуса UI.
     * @param tempId Временный id сообщения
     * @param msg Структура сообщения
     */
    void confirmMessageSent(QString tempId, const ChatMessage& msg);

    /**
     * @brief Сигналы для VoIP/Call: отправка запроса звонка, входящий звонок, принятие/отклонение/завершение.
     */
    void callRequestSent(const QString& toUser, const QString& callId);
    void incomingCall(const QString& fromUser, const QString& callId, const QString& callerIp, quint16 callerPort);
    void callAccepted(const QString& fromUser, const QString& calleeIp, quint16 calleePort);
    void callRejected(const QString& fromUser, const QString& reason);
    void callEnded();

    /**
     * @brief Сигналы истории звонков, статистической информации, обновления профиля.
     */
    void callHistoryReceived(const QJsonArray& calls);
    void callStatsReceived(const QJsonObject& stats);

    /**
     * @brief Сигнал о результате обновления профиля.
     * @param response JSON с результатом
     */
    void profileUpdateResult(const QJsonObject& response);

    /**
     * @brief Сигнал для запроса серверной истории сообщений (после lastId).
     * @param chatPartner Собеседник
     * @param afterId id после которого нужно получить сообщения
     */
    void requestServerHistory(const QString& chatPartner, int afterId);


private:
    /**
 * @brief Инициализирует маршрутизаторы серверных/клиентских событий в DataService.
 *
 * - Заполняет карту m_responseHandlers соответствием строковых типов событий и слотов-обработчиков.
 * - Позволяет централизованно прокладывать все сетевые и клиентские события на нужные обработчики.
 * - После вызова функции любой тип, пришедший как "type" в processResponse(), попадёт на соответствующий метод.
 */
    void initResponseHandlers();
    /**
 * @brief Получает максимальный server_id для заданного чата, используемый для синхронизации и запроса новых сообщений.
 *
 * - Формирует SQL-запрос на выбор максимального server_id из таблицы messages с заданным chat_id.
 * - Если запрос выполняется успешно и есть результат — возвращает максимальный server_id.
 * - В случае ошибок SQL или если результат пустой — возвращает 0 (корректная инициализация, fallback).
 *
 * @param chatId Идентификатор чата (chat_id в базе данных)
 * @return Максимальный server_id для этого чата, либо 0 если нет сообщений
 */
    int getLastServerIdForChat(int chatId);
    /**
 * @brief Возвращает множество server_id, уже присутствующих в локальном кеше для проверки дублирования сообщений.
 *
 * - Формирует SQL-запрос, который вытаскивает все существующие server_id по chatPartner.
 * - Используется при фильтрации и upsert новых сообщений для исключения дублей.
 * - Возвращает QSet для быстрого поиска по id.
 *
 * @param chatPartner Имя пользователя, для которого ищем существующие server_id
 * @return QSet уже существующих server_id
 */
    QSet<qint64> fetchExistingServerIds(const QString& chatPartner);

    /**
 * @brief Добавляет сообщения из списка в базу, применяя upsert и фильтрацию по уже существующим id.
 *
 * - Проверяет пустоту входящего списка сообщений — если пусто, выходим сразу.
 * - Получает множество всех имеющихся server_id для chatPartner для предотвращения дублей.
 * - Открывает транзакцию для ускоренной пакетной записи и атомарности.
 * - Для каждого сообщения:
 *   - Пропускает, если id уже в локальном списке server_id.
 *   - Формирует и выполняет запрос INSERT OR IGNORE с bind-значениями (safe).
 *   - Считает количество реально добавленных сообщений.
 * - После пакета коммитит транзакцию.
 * - Логгирует результат — сколько сообщений реально записано с фильтрацией.
 *
 * @param messages Список новых сообщений для вставки
 * @param chatPartner Имя собеседника (локального кеша для фильтрации server_id)
 */
    void insertMessagesWithUpsertFiltered(const QList<ChatMessage>& messages, const QString& chatPartner);

    // --- Database Backend ---
    /**
 * @brief Объект для управления всей работой с базой: инициализация, запись, чтение, транзакции.
 */
    DatabaseService* m_dbService = nullptr;

    /**
 * @brief Тип-указатель на функцию-член DataService для централизованной маршрутизации серверных событий.
 *
 * Используется как entries в m_responseHandlers.
 */
    using ResponseHandler = void (DataService::*)(const QJsonObject&);

    /**
 * @brief Карта для "роутинга" приходящих событий от сервера (или ядра) на соответствующие обработчики (методы-слоты).
 *
 * Ключ — строковый тип события ("login_success" и т.д.), значение — указатель на функцию.
 */
    QMap<QString, ResponseHandler> m_responseHandlers;

    // --- History/Cache ---
    /**
 * @brief Кеш истории сообщений по каждому диалогу (username → ChatCache).
 *
 * Используется для быстрого отображения сообщений, ускорения поиска, экономии обращений к диску.
 */
    QMap<QString, ChatCache> m_chatHistoryCache;

    /**
 * @brief Кеш всех пользователей-контактов (username → User).
 */
    QMap<QString, User> m_userCache;

    /**
 * @brief Карта непрочитанных сообщений в каждом чате (username → count).
 *
 * Общий источник непрочитанных для бейджей, sidebar и push-логики.
 */
    QMap<QString, int> m_unreadCounts;

    // --- User/State ---
    /**
 * @brief Текущий залогиненный пользователь (основные свойства профиля, username и т.д.).
 */
    User m_currentUser;

    /**
 * @brief Текущий выбранный собеседник для чата (например, для быстрого доступа к его данным).
 */
    User m_currentChatPartner;

    /**
 * @brief Флаг процесса загрузки истории чата (true — идёт вычитка с сервера или из БД).
 */
    bool m_isLoadingHistory = false;

    /**
 * @brief ID самого старого сообщения в текущей истории (нужен для подгрузки старых chunk-ов).
 */
    qint64 m_oldestMessageId = 0;

    /**
 * @brief ID сообщения, которое сейчас редактируется (если не редактируется — 0).
 */
    qint64 m_editingMessageId = 0;

    /**
 * @brief ID сообщения, на которое сейчас идёт ответ (если не используется — 0).
 */
    qint64 m_replyToMessageId = 0;

    // --- Timers ---
    /**
 * @brief Таймер для throttling/"дебаунса" длинных глобальных поисковых запросов.
 */
    QTimer* m_globalSearchTimer;

    /**
 * @brief Таймер для задержки отправки статуса "печатает" чтобы не спамить сеть.
 */
    QTimer* m_typingSendTimer;

    /**
 * @brief Таймеры (отдельный для каждого собеседника) для сброса статуса "печатает" при простое.
 */
    QMap<QString, QTimer*> m_typingReceiveTimers;

    // --- File Upload & Search ---
    /**
 * @brief Список файлов, которые сейчас загружаются в чат (пути на файловой системе).
 */
    QVector<QString> m_uploadingFilePath;

    /**
 * @brief Флаг активации режима поиска по всей истории чата.
 */
    bool m_isChatSearchActive = false;
};

#endif

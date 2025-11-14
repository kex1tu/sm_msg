#include "dataservice.h"
#include <QJsonArray>
#include <algorithm>  
#include <QDebug>
#include <QTimer>
#include <QListView>
#include <QScrollBar>

class MainWindow;

DataService::DataService(QObject *parent) : QObject(parent)
{
    // Создаём сервис БД и сразу пытаемся подключиться к файлу
    m_dbService = new DatabaseService(this);

    // Инициализация: если не удалось открыть базу — выводим подробное предупреждение в лог
    if (!m_dbService->initialize("database.db")) {
        qDebug() << "[DataService] WARNING: Database initialization failed";
    }

    // Инициализация таймера глобального поиска (например, для throttling поисковых запросов)
    m_globalSearchTimer = new QTimer(this);
    m_globalSearchTimer->setSingleShot(true); // таймер срабатывает только один раз за цикл
    m_globalSearchTimer->setInterval(300);    // интервал задержки в мс

    // Таймер отправки статуса "печатает" (чтобы не перегружать события на сервер)
    m_typingSendTimer = new QTimer(this);
    m_typingSendTimer->setSingleShot(true);
    m_typingSendTimer->setInterval(2000); // интервал уведомления для UX

    // Инициализация обработчиков входящих событий от сервера/клиента
    initResponseHandlers();
}


void DataService::processResponse(const QJsonObject& response)
{
    // Извлекаем тип события (например, "login_success", "private_message" и др.)
    QString type = response["type"].toString();

    // Проверяем наличие обработчика для этого типа события
    if (m_responseHandlers.contains(type)) {
        ResponseHandler handler = m_responseHandlers[type];   // Получаем указатель на обработчик

        // Вызов соответствующего обработчика через указатель на метод
        (this->*handler)(response);
    } else {
        // Нет обработчика для такого типа — пишем предупреждение и в лог, чтобы удобно искать пропущенные/новые события
        qDebug() << "[DataService] WARNING: No handler found for message type:" << type;
    }
}


void DataService::initResponseHandlers()
{
    // [DataService] События аутентификации
    m_responseHandlers["login_success"] = &DataService::handleLoginSuccess;
    m_responseHandlers["login_failure"] = &DataService::handleLoginFailure;
    m_responseHandlers["register_success"] = &DataService::handleRegisterSuccess;
    m_responseHandlers["register_failure"] = &DataService::handleRegisterFailure;
    m_responseHandlers["logout_request_success"] = &DataService::handleLogoutSuccess;
    m_responseHandlers["logout_request_failure"] = &DataService::handleLogoutFailure;

    // [DataService] События для контактов, списка пользователей, поиска
    m_responseHandlers["contact_list"] = &DataService::handleContactList;
    m_responseHandlers["user_list"] = &DataService::handleUserList;
    m_responseHandlers["search_results"] = &DataService::handleSearchResults;
    m_responseHandlers["add_contact_success"] = &DataService::handleAddContactSuccess;
    m_responseHandlers["add_contact_failure"] = &DataService::handleAddContactFailure;
    m_responseHandlers["incoming_contact_request"] = &DataService::handleIncomingContactRequest;
    m_responseHandlers["pending_requests_list"] = &DataService::handlePendingRequestsList;

    // [DataService] События истории сообщений
    m_responseHandlers["history_data"] = &DataService::handleHistoryData;
    m_responseHandlers["old_history_data"] = &DataService::handleOldHistoryData;
    m_responseHandlers["private_message"] = &DataService::handlePrivateMessage;
    m_responseHandlers["edit_message"] = &DataService::handleEditMessage;
    m_responseHandlers["delete_message"] = &DataService::handleDeleteMessage;

    // [DataService] Статусы сообщений, наборы событий (доставлено, прочитано, начало печати, непрочитанные)
    m_responseHandlers["message_delivered"] = &DataService::handleMessageDelivered;
    m_responseHandlers["message_read"] = &DataService::handleMessageRead;
    m_responseHandlers["typing"] = &DataService::handleTypingResponse;
    m_responseHandlers["unread_counts"] = &DataService::handleUnreadCounts;

    // [DataService] События звонков (VoIP/Call)
    m_responseHandlers["call_request_sent"] = &DataService::handleCallRequestSent;
    m_responseHandlers["call_request"] = &DataService::handleIncomingCall;
    m_responseHandlers["call_accepted"] = &DataService::handleCallAccepted;
    m_responseHandlers["call_rejected"] = &DataService::handleCallRejected;
    m_responseHandlers["call_end"] = &DataService::handleCallEnd;
    m_responseHandlers["call_history"] = &DataService::handleCallHistory;
    m_responseHandlers["call_stats"] = &DataService::handleCallStats;

    // [DataService] Профиль пользователя
    m_responseHandlers["update_profile_result"] = &DataService::handleUpdateProfileResult;

    // Можно добавить qDebug для контроля наполнения структуры
    qDebug() << "[DataService] Response handlers initialized:" << m_responseHandlers.size();
}


void DataService::handleUpdateProfileResult(const QJsonObject& response)
{
    // Сигнализируем подписчикам UI/логики о том, что пришёл результат обновления профиля
    emit profileUpdateResult(response);
}


void DataService::handleCallStats(const QJsonObject& response){
    // Диагностика: информируем о получении статистики (от сервера)
    qDebug() << "[DataService] Received call statistics";

    // Извлекаем параметры статистики из JSON
    int outgoing = response["outgoing"].toInt();
    int incoming = response["incoming"].toInt();
    int completed = response["completed"].toInt();
    int missed = response["missed"].toInt();
    int totalDuration = response["total_duration_sec"].toInt();

    // Подробный лог: все ключевые параметры одной строкой
    qDebug() << "[DataService] STATS: Outgoing:" << outgoing
             << "| Incoming:" << incoming
             << "| Completed:" << completed
             << "| Missed:" << missed
             << "| Total duration:" << totalDuration << "s";

    // Раздаём данные через сигнал — получение и визуализация статистики
    emit callStatsReceived(response);
}


void DataService::handleCallHistory(const QJsonObject& response){
    // Извлекаем массив звонков
    QJsonArray calls = response["calls"].toArray();

    // Лог: сколько звонков получено
    qDebug() << "[DataService] Received call history:" << calls.size() << "calls";

    // Перебираем все звонки для логирования, отображения ключевых данных каждого звонка для профилирования
    for (const QJsonValue& val : calls) {
        QJsonObject call = val.toObject();
        qDebug() << "[DataService] CALL HISTORY:"
                 << (call["call_type"].toString() == "outgoing" ? "OUT" : "IN")
                 << call["caller"].toString() << "→" << call["callee"].toString()
                 << "| Status:" << call["status"].toString()
                 << "| Duration:" << call["duration_seconds"].toInt() << "s";
    }

    // Передача массива истории по сигналу для дальнейшей обработки и UI-отображения
    emit callHistoryReceived(calls);
}


void DataService::requestCallHistory()
{
    // Здесь в дальнейшем будет отправка запроса на сервер для получения истории звонков
    qDebug() << "[DataService] Requesting call history from server";
}


void DataService::handleCallRequestSent(const QJsonObject& response)
{
    // Фиксация: запрос на звонок успешно отправлен
    qDebug() << "[DataService] CALL REQUEST SENT";

    // Получаем параметры звонка: кому и идентификатор сессии
    QString toUser = response.value("to").toString();
    QString callId = response.value("call_id").toString();

    // Логируем параметры отправки
    qDebug() << "[DataService] Call sent to:" << toUser << "call_id:" << callId;

    // Сигнал для реакции интерфейса/других компонентов
    emit callRequestSent(toUser, callId);
}


void DataService::handleIncomingCall(const QJsonObject& response)
{
    // Лог: фиксация входящего звонка — от кого, идентификатор
    qDebug() << "[DataService] INCOMING CALL";

    QString fromUser = response.value("from").toString();
    QString callId = response.value("call_id").toString();
    QString callerIp = response.value("caller_ip").toString();
    quint16 callerPort = response.value("caller_port").toInt();

    // Детальное логирование параметров для профилирования VoIP/сбора статистики
    qDebug() << "[DataService] Incoming call from:" << fromUser
             << "call_id:" << callId
             << "ip:" << callerIp
             << "port:" << callerPort;

    // Сигнал на все системы, способные принять звонок (UI, VoIP backend и т.д.)
    emit incomingCall(fromUser, callId, callerIp, callerPort);
}


void DataService::handleCallAccepted(const QJsonObject& response)
{
    // Фиксация факта принятия вызова — служебный лог для анализа VoIP-событий
    qDebug() << "[DataService] CALL ACCEPTED";

    QString fromUser = response.value("from").toString();
    QString calleeIp = response.value("callee_ip").toString();
    quint16 calleePort = response.value("callee_port").toInt();

    // Подробное логирование: кто, на какой адрес, какой порт
    qDebug() << "[DataService] Call accepted by:" << fromUser
             << "ip:" << calleeIp
             << "port:" << calleePort;

    // Генерируем сигнал для старта медиасоединения/смены UI
    emit callAccepted(fromUser, calleeIp, calleePort);
}


void DataService::handleCallRejected(const QJsonObject& response)
{
    // Фиксация отказа в логе (для сценариев busy, reject и т.д.)
    qDebug() << "[DataService] CALL REJECTED";

    QString fromUser = response.value("from").toString();
    QString reason = response.value("reason").toString();

    // Диагностика: кто отказал и по какой причине
    qDebug() << "[DataService] Call rejected by:" << fromUser
             << "reason:" << reason;

    // Генерируем сигнал для UI, backend и истории
    emit callRejected(fromUser, reason);
}


void DataService::handleCallEnd(const QJsonObject& response)
{
    QString fromUser = response["from"].toString();
    QString callId = response["call_id"].toString();

    // Подробное логирование
    qDebug() << "[DataService] CALL END from:" << fromUser << "call_id:" << callId;

    // Сигнализируем всем компонентам о завершении
    emit callEnded();
}


User* DataService::getUserFromCache(const QString& username) {
    if (m_userCache.contains(username)) {
        // Пользователь найден — возвращаем указатель
        return &m_userCache[username];
    }
    // Не найден в кеше — безопасно возвращаем nullptr
    return nullptr;
}


QMap<QString, ChatCache>* DataService::getChatCache() {
    if (!m_chatHistoryCache.isEmpty()) {
        return &m_chatHistoryCache;
    }
    return nullptr;
}


QMap<QString, int>* DataService::getUnreadCounts() {
    if (!m_unreadCounts.isEmpty()) {
        return &m_unreadCounts;
    }
    return &m_unreadCounts;
}


ChatCache* DataService::getChatCacheForUser(const QString& username) {
    if (m_chatHistoryCache.contains(username)) {
        return &m_chatHistoryCache[username];
    }
    return nullptr;
}


User* DataService::getCurrentChatPartner() {
    return &m_currentChatPartner;
}


QTimer* DataService::getGlobalSearchTimer() {
    return m_globalSearchTimer;
}


QTimer* DataService::getTypingSendTimer() {
    return m_typingSendTimer;
}


QMap<QString, QTimer*>* DataService::getTypingRecieveTimers() {
    if(!m_typingReceiveTimers.isEmpty()){
        // Если есть таймеры — возвращаем для управления ими
        return &m_typingReceiveTimers;
    }
    // Таймеров нет — возврат nullptr
    return nullptr;
}


QMap<QString, User>* DataService::getUserCache() {
    return &m_userCache;
}


qint64* DataService::getReplyToMessageId() {
    return &m_replyToMessageId;
}


qint64* DataService::getEditinigMessageId() {
    return &m_editingMessageId;
}


User* DataService::getCurrentUser() {
    return &m_currentUser;
}


bool* DataService::getIsLoadingHistory() {
    return &m_isLoadingHistory;
}


qint64* DataService::getOldestMessageId() {
    return &m_oldestMessageId;
}


void DataService::handleContactList(const QJsonObject& response) {
    // Логируем начало обработки
    qDebug() << "[DataService] Processing contact list.";
    QJsonArray usersFromServer = response["users"].toArray();

    // Очищаем кеш, чтобы оставить только актуальные контакты сервера
    User me = m_userCache[m_currentUser.username];
    m_userCache.clear();

    // Парсим массив, на каждом шаге логгируем имя контакта и сохраняем в кеш
    for (const QJsonValue &value : usersFromServer) {
        QJsonObject userObj = value.toObject();
        User user;
        user.username = userObj["username"].toString();
        user.displayName = userObj["displayname"].toString();
        user.lastSeen = userObj["last_seen"].toString();
        qDebug() << "[DataService] Loaded contact:" << user.username;
        m_userCache.insert(user.username, user);
    }

    // Сортируем для UI по displayName (case insensitive)
    QList<User> users = m_userCache.values();
    std::sort(users.begin(), users.end(), [](const User& a, const User& b) {
        return a.displayName.toLower() < b.displayName.toLower();
    });

    // Собираем отсортированные usernames (для сигнала)
    QStringList usernames;
    for (const User& user : users) {
        usernames.append(user.username);
    }
    // Эмитим сигнал чтобы UI или другие слушатели могли обновить контакты
    emit contactsUpdated(usernames);
}


void DataService::handleUserList(const QJsonObject& response) {
    QJsonArray onlineUsernamesArray = response["users"].toArray();
    QSet<QString> onlineUsers;

    // Формируем QSet всех онлайн-имен для быстрых проверок
    for (const QJsonValue &value : onlineUsernamesArray) {
        onlineUsers.insert(value.toString());
    }

    // По всему локальному кешу — отмечаем свойство isOnline у User
    for (auto it = m_userCache.begin(); it != m_userCache.end(); ++it) {
        it.value().isOnline = onlineUsers.contains(it.key());
    }

    // Эмит сигнал — переход UI/логики на новое состояние.
    emit onlineStatusUpdated();
}


void DataService::handleOldHistoryData(const QJsonObject& response)
{
    // Извлекаем идентификатор собеседника и массив сообщений
    QString historyForUser = response["with_user"].toString();
    QJsonArray history = response["history"].toArray();
    qDebug() << "[DataService] Received" << history.count() << "older messages for" << historyForUser;

    // Получаем кеш текущего чата для локального сохранения/фильтрации
    ChatCache& cache = m_chatHistoryCache[historyForUser];

    // Если сервер вернул пустую историю (выгружено всё) — отмечаем кеш-флаг
    if (history.isEmpty()) {
        cache.allMessagesLoaded = true;
        if (historyForUser == m_currentChatPartner.username) {
            // Для активного чата обновляем глобальный id и кеш-флаг
            m_oldestMessageId = 0;
            m_chatHistoryCache[historyForUser].allMessagesLoaded = true;
            // Эмитим сигнал для UI (пустой chunk)
            emit olderHistoryChunkPrepended(historyForUser, QList<ChatMessage>());
        }
        // Для любого случая — просто выходим
        return;
    }

    // Собираем, конвертим и статусы расставляем для каждой записи JSON -> ChatMessage
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
        msg.isOutgoing = (msg.fromUser == m_currentUser.username);
        msg.isEdited = msgObj["is_edited"].toInt();

        // Выставляем статус по приоритетам (read > delivered > sent)
        if(msgObj["is_delivered"].toInt() == 1){
            msg.status = ChatMessage::Delivered;
        } else {
            msg.status = ChatMessage::Sent;
        }
        if(msgObj["is_read"].toInt() == 1){
            msg.status = ChatMessage::Read;
        }
        messages.append(msg);
    }

    // Фильтруем и upsert в кеш (и БД, если нужно)
    insertMessagesWithUpsertFiltered(messages, historyForUser);

    // Препендим в локальный кеш вручную так, чтобы старые были впереди — для правильной динамики диалога
    for (int i = messages.count() - 1; i >= 0; --i) {
        cache.messages.prepend(messages.at(i));
    }

    // Сохраняем id самого старого сообщения для быстрого определения границ истории
    cache.oldestMessageId = messages.first().id;

    // Если это чат текущей сессии — обновляем глобальные флаги и отправляем сигнал UI с chunk'ом
    if (historyForUser == m_currentChatPartner.username) {
        qDebug() << "[DataService] Older history is for the current chat. Emitting signal.";
        m_oldestMessageId = cache.oldestMessageId;
        emit olderHistoryChunkPrepended(historyForUser, messages);
    } else {
        // Для фоновых чатов просто логгируем фоновое обновление
        qDebug() << "[DataService] Older history is for a background chat. Caching silently.";
    }
}


void DataService::handleHistoryData(const QJsonObject& response) {
    // Имя собеседника для которого история и сам массив истории
    QString historyForUser = response["with_user"].toString();
    QJsonArray history = response["history"].toArray();

    // Логфиксация длины истории
    qDebug() << "[DataService] Received" << history.count() << "messages for" << historyForUser;

    // Вытаскиваем каждое сообщение в отдельную структуру с выставлением статусов
    QList<ChatMessage> messages;
    for (const QJsonValue &value : history) {
        QJsonObject msgObj = value.toObject();
        ChatMessage msg;

        msg.id = msgObj["id"].toDouble();
        msg.fromUser = msgObj["fromUser"].toString();
        msg.toUser = msgObj["toUser"].toString();
        msg.payload = msgObj["payload"].toString();
        msg.timestamp = msgObj["timestamp"].toString();
        msg.replyToId = msgObj["reply_to_id"].toDouble();
        msg.isEdited = msgObj["is_edited"].toInt();
        msg.isOutgoing = (msg.fromUser == m_currentUser.username);

        // Корректная расстановка статусов с правильным приоритетом
        if(msgObj["is_read"].toInt() == 1) {
            msg.status = ChatMessage::Read;
        } else if (msgObj["is_delivered"].toInt() == 1) {
            msg.status = ChatMessage::Delivered;
        } else {
            msg.status = ChatMessage::Sent;
        }

        messages.append(msg);
    }

    // Перед апдейтом кеша — фильтруем и обновляем/вставляем сообщения методом upsert для консистентности
    insertMessagesWithUpsertFiltered(messages, m_currentChatPartner.username);

    // Обновляем кеш чата только если такого сообщения ещё нет (по id)
    ChatCache& cache = m_chatHistoryCache[historyForUser];
    for (const ChatMessage& msg : messages) {
        bool exists = false;
        for (const ChatMessage& cached : cache.messages) {
            if (cached.id == msg.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            cache.messages.append(msg);
        }
    }

    // Если это активный чат пользователя — обновляем id, выставлем флаг загрузки и эмитим сигнал
    if (historyForUser == m_currentChatPartner.username) {
        m_oldestMessageId = cache.messages.isEmpty() ? 0 : cache.messages.first().id;
        m_isLoadingHistory = false;

        qDebug() << "[DataService] Emitting historyLoaded for current chat";
        emit historyLoaded(historyForUser, messages);
    } else {
        // Для фонового чата — логгируем тихое обновление для возможной отладки/аналитики
        qDebug() << "[DataService] History cached silently for background chat";
    }
}


void DataService::handleUnreadCounts(const QJsonObject& response)
{
    // Логгируем сам факт получения — удобно искать массовые обновления после sync/старта
    qDebug() << "[DataService] Received initial unread counts from server.";
    QJsonArray countsArray = response["counts"].toArray();

    // Очищаем старое локальное состояние
    m_unreadCounts.clear();

    // Перебираем каждый entry в countsArray — username & count
    for (const QJsonValue &value : countsArray) {
        QJsonObject countObj = value.toObject();
        QString username = countObj["username"].toString();
        int count = countObj["count"].toInt();

        // Добавляем только если есть непрочитанные, чтобы не мусорить мапу лишними нулями
        if (count > 0) {
            m_unreadCounts[username] = count;
        }
    }

    // Применяем изменения: реагирует UI и логика диалогов
    emit unreadCountChanged();
}


void DataService::handleLoginSuccess(const QJsonObject& response)
{
    emit loginSuccess(response);
}


void DataService::handleLoginFailure(const QJsonObject& response)
{
    emit loginFailure(response["reason"].toString());
}


void DataService::handleRegisterSuccess(const QJsonObject& response)
{
    Q_UNUSED(response);
    emit registerSuccess();
}


void DataService::handleRegisterFailure(const QJsonObject& response)
{
    emit registerFailure(response["reason"].toString());
}


void DataService::handlePrivateMessage(const QJsonObject& response) {

    // Если сообщение - echo ранее отправленного, но ещё не подтвержденного сервером (есть temp_id)
    QString tempId = response["temp_id"].toString();
    if (!tempId.isEmpty()) {
        // Лог: пришёл echo для нашего сообщения с временным id
        qDebug() << "[DataService] Received ECHO for temp_id:" << tempId;

        // Формируем ChatMessage на базе ответа (в том числе присваиваем серверный id)
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

        // Подтвердить и сохранить серверный id через БД (если БД доступна)
        if (m_dbService && m_dbService->isConnected()) {
            m_dbService->confirmSentMessageByTempId(tempId, msg);
            qDebug() << "[DataService] Echo: updated local record by tempId";
        }
        // Эмит event для UI & логики
        qDebug() << "[DataService] Emit confirmMessageSent for tempId";
        emit confirmMessageSent(tempId, msg);

        // Найти и обновить в кеше чата по temp_id (чтобы статус был актуальным для UI)
        QString chatPartner = msg.toUser;
        if (m_chatHistoryCache.contains(chatPartner)) {
            QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
            for (int i = 0; i < messagesInCache.size(); ++i) {
                if (messagesInCache[i].tempId == tempId) {
                    messagesInCache[i] = msg;
                    qDebug() << "[DataService] CACHE: Confirmed message with temp_id:" << tempId << "in cache for" << chatPartner;
                    break;
                }
            }
        }
        // Echo обработан — дальше не идём
        return;
    }

    // Обработка нового входящего сообщения от собеседника
    ChatMessage incomingMsg;
    incomingMsg.id = response["id"].toDouble();
    incomingMsg.fromUser = response["fromUser"].toString();
    incomingMsg.toUser = response["toUser"].toString();
    incomingMsg.payload = response["payload"].toString();
    incomingMsg.timestamp = response["timestamp"].toString();
    incomingMsg.replyToId = response["reply_to_id"].toDouble();
    incomingMsg.isOutgoing = false;
    incomingMsg.isEdited = false;

    // Корректная расстановка статусов
    if (response["is_read"].toInt() == 1) {
        incomingMsg.status = ChatMessage::Read;
    } else if (response["is_delivered"].toInt() == 1) {
        incomingMsg.status = ChatMessage::Delivered;
    } else {
        incomingMsg.status = ChatMessage::Sent;
    }

    // Добавляем сообщение в кеш истории чата и обновляем границу, если кеш существует
    QString chatPartner = incomingMsg.fromUser;
    if (m_chatHistoryCache.contains(chatPartner)) {
        m_chatHistoryCache[chatPartner].messages.append(incomingMsg);
        m_chatHistoryCache[chatPartner].oldestMessageId = incomingMsg.id ;
    }

    // Сохраняем входящее сообщение в БД, если возможно
    if (m_dbService && m_dbService->isConnected()) {
        m_dbService->saveMessage(incomingMsg, m_currentUser.username);
        qDebug() << "[DataService] Incoming message saved to DB";
    }

    qDebug() << "[DataService] Emit newMessageReceived for incomingMsg";
    emit newMessageReceived(incomingMsg);
}


DatabaseService* DataService::getDatabaseService() {
    return m_dbService;
}


void DataService::handleMessageDelivered(const QJsonObject& response)
{
    qint64 messageId = response["id"].toDouble();
    qDebug() << "[DataService] Message" << messageId << "was delivered.";

    // Обновляем статус в базе, если есть соединение
    if (m_dbService && m_dbService->isConnected()) {
        m_dbService->updateMessageStatus(messageId, ChatMessage::Delivered);
        qDebug() << "[DataService] Status updated in DB for message" << messageId << "-> Delivered";
    }

    // Проставляем статус в кеше и сигнализируем UI (если это текущий чат)
    ChatMessage::MessageStatus newStatus = ChatMessage::Delivered;
    for (auto it = m_chatHistoryCache.begin(); it != m_chatHistoryCache.end(); ++it) {
        QList<ChatMessage>& messages = it.value().messages;
        for (int i = 0; i < messages.size(); ++i) {
            if (messages[i].id == messageId) {
                if (messages[i].status == newStatus) return; // Уже установлен — пропуск повторного вызова

                messages[i].status = newStatus;
                QString foundInChatWith = it.key();
                qDebug() << "[DataService] Status updated for message" << messageId << "in chat with" << foundInChatWith;
                if (foundInChatWith == m_currentChatPartner.username) {
                    emit messageStatusChanged(messageId, newStatus);
                }
                return;
            }
        }
    }
}


void DataService::handleMessageRead(const QJsonObject& response)
{
    qint64 messageId = response["id"].toDouble();
    qDebug() << "[DataService] Message" << messageId << "was read.";

    // Обновляем БД, если активна
    if (m_dbService && m_dbService->isConnected()) {
        m_dbService->updateMessageStatus(messageId, ChatMessage::Read);
        qDebug() << "[DataService] Status updated in DB for message" << messageId << "-> Read";
    }

    // Поднимаем статус до read в кеше (только если реально изменился)
    ChatMessage::MessageStatus newStatus = ChatMessage::Read;
    for (auto it = m_chatHistoryCache.begin(); it != m_chatHistoryCache.end(); ++it) {
        QList<ChatMessage>& messages = it.value().messages;
        for (int i = 0; i < messages.size(); ++i) {
            if (messages[i].id == messageId) {
                if (messages[i].status == newStatus) return; // Уже был выставлен — выходим

                messages[i].status = newStatus;
                QString foundInChatWith = it.key();
                qDebug() << "[DataService] Status updated for message" << messageId << "in chat with" << foundInChatWith;

                if (foundInChatWith == m_currentChatPartner.username) {
                    emit messageStatusChanged(messageId,newStatus);
                }
                return;
            }
        }
    }
}



void DataService::handleEditMessage(const QJsonObject& response)
{
    QString chatPartner = response["with_user"].toString();
    qint64 messageId = response["id"].toDouble();
    QString newPayload = response["payload"].toString();

    // Информируем о поступлении команды на редактирование сообщения
    qDebug() << "[DataService] Received command to edit message" << messageId;

    // Обновляем в БД
    if (m_dbService && m_dbService->isConnected()) {
        m_dbService->editMessage(messageId, newPayload);
        qDebug() << "[DataService] Message" << messageId << "edited in DB";
    }

    // Ищем и обновляем сообщение в локальном кеше чата (payload и isEdited)
    if (&m_chatHistoryCache[chatPartner] != nullptr) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                messagesInCache[i].payload = newPayload;
                messagesInCache[i].isEdited = true;
                qDebug() << "[DataService] Message" << messageId << "edited in cache for" << chatPartner;
                break;
            }
        }
    }

    // Информируем все заинтересованные слои (UI/логика) о факте изменения
    emit messageEdited(chatPartner, messageId, newPayload);
}


void DataService::handleDeleteMessage(const QJsonObject& response){
    qint64 messageId = response["id"].toDouble();
    QString chatUser = response["with_user"].toString();

    // Удаляем из базы, если возможно соединение для целостности истории
    if (m_dbService && m_dbService->isConnected()) {
        m_dbService->deleteMessage(messageId);
        qDebug() << "[DataService] Message" << messageId << "deleted from DB";
    }

    // Проверяем, в каком чате надо удалить (if self — значит этот пользователь чатился сам с собой)
    QString chatPartner = (chatUser == m_currentUser.username) ? m_currentChatPartner.username : chatUser;
    qDebug() << "[DataService] Received command to delete message" << messageId << "in chat with user" << chatPartner;

    // В кеше снимаем сообщение, если оно было
    if (m_chatHistoryCache.contains(chatPartner)) {
        QList<ChatMessage>& messagesInCache = m_chatHistoryCache[chatPartner].messages;
        for (int i = 0; i < messagesInCache.size(); ++i) {
            if (messagesInCache[i].id == messageId) {
                messagesInCache.removeAt(i);
                qDebug() << "[DataService] CACHE: Message" << messageId << "deleted from cache for" << chatPartner;
                break;
            }
        }
    }

    emit messageDeleted(chatPartner, messageId);
}


void DataService::handleSearchResults(const QJsonObject& response)
{
    QJsonArray users = response["users"].toArray();
    emit searchResultsReceived(users);
}


void DataService::handleAddContactSuccess(const QJsonObject& response)
{
    emit addContactSuccess(response["username"].toString());
}


void DataService::handleAddContactFailure(const QJsonObject& response)
{
    emit addContactFailure(response["reason"].toString());
}


void DataService::handleIncomingContactRequest(const QJsonObject& response)
{
    emit contactRequestReceived(response);
}


void DataService::handlePendingRequestsList(const QJsonObject& response)
{
    QJsonArray requests = response["requests"].toArray();
    emit pendingContactRequestsUpdated(requests);
}


void DataService::handleLogoutSuccess(const QJsonObject& response)
{
    Q_UNUSED(response);
    emit logoutSuccess();
}


void DataService::handleLogoutFailure(const QJsonObject& response)
{
    emit logoutFailure(response["reason"].toString());
}


void DataService::handleTypingResponse(const QJsonObject& response)
{
    QString fromUser = response["fromUser"].toString();

    // Если пользователя нет в кеше, не обрабатываем событие (может быть race)
    if (!m_userCache.contains(fromUser)) return;

    // Отмечаем флаг isTyping для данного пользователя
    m_userCache[fromUser].isTyping = true;

    // Уведомляем UI и логику об изменении статуса typing
    emit typingStatusChanged(fromUser, true);

    // Если для пользователя нет активного таймера, создаем новый singleShot таймер (отсчитает до сброса статуса)
    if (!m_typingReceiveTimers.contains(fromUser)) {
        QTimer* timer = new QTimer(this);
        timer->setInterval(2500);
        timer->setSingleShot(true);

        // По таймауту сбрасываем статус (isTyping=false) и эмитим изменение
        connect(timer, &QTimer::timeout, this, [this, fromUser](){
            if (m_userCache.contains(fromUser)) {
                m_userCache[fromUser].isTyping = false;
                emit typingStatusChanged(fromUser, false);
            }
        });
        m_typingReceiveTimers[fromUser] = timer;
    }

    // Каждый раз при новом событии "печатает" (и при первом) перезапускаем таймер
    m_typingReceiveTimers[fromUser]->start();
}


void DataService::clearAllData()
{
    // Очищаем все основные кеши и структуры
    m_chatHistoryCache.clear();
    m_userCache.clear();
    m_unreadCounts.clear();

    // Сбрасываем флаги и текущие состояния
    m_currentUser = User();
    m_currentChatPartner = User();
    m_isLoadingHistory = false;
    m_oldestMessageId = 0;
    m_editingMessageId = 0;
    m_replyToMessageId = 0;
    m_isChatSearchActive = false;

    // Удаляем все таймеры и из мапы
    qDeleteAll(m_typingReceiveTimers);
    m_typingReceiveTimers.clear();

    // Лог: для проверки целостности и корректности при тестах, сбросах, перезагрузках
    qDebug() << "[DataService] All data and state has been cleared.";
}


int DataService::getLastServerIdForChat(int chatId) {
    QSqlQuery query;
    query.prepare("SELECT MAX(server_id) FROM messages WHERE chat_id = ?");
    query.addBindValue(chatId);

    // Выполняем запрос и сразу анализируем результат (наличие строки)
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    // В случае неудачи либо отсутствия сообщений возвращаем 0 для корректной обработки синхронизации
    return 0;
}


void DataService::syncChatHistory(const QString& chatPartner) {
    // Лог — начало синхронизации
    qDebug() << "[DataService] Starting history sync for chat with:" << chatPartner;

    QList<ChatMessage> localHistory;
    QSqlQuery query;
    query.prepare("SELECT id, fromUser, toUser, payload, timestamp, is_edited, is_delivered, is_read "
                  "FROM messages WHERE (fromUser = ? OR toUser = ?) ORDER BY id ASC");
    query.addBindValue(m_currentUser.username);
    query.addBindValue(chatPartner);

    // Загружаем результаты запроса
    if (query.exec()) {
        while (query.next()) {
            ChatMessage msg;
            msg.id = query.value(0).toDouble();
            msg.fromUser = query.value(1).toString();
            msg.toUser = query.value(2).toString();
            msg.payload = query.value(3).toString();
            msg.timestamp = query.value(4).toString();
            msg.isEdited = query.value(5).toInt();
            msg.isOutgoing = (msg.fromUser == m_currentUser.username);

            // Отмечаем статусы delivered/read (если потребуется)
            if (query.value(6).toInt() == 1) {
                msg.status = ChatMessage::Delivered;
            }
            if (query.value(7).toInt() == 1) {
                msg.status = ChatMessage::Read;
            }

            localHistory.append(msg);
        }
    }

    // Заполняем кеш текущего чата
    m_chatHistoryCache[chatPartner].messages = localHistory;
    if (!localHistory.isEmpty()) {
        m_oldestMessageId = localHistory.first().id;
    }

    // Лог: сколько сообщений загрузили из локального кеша
    qDebug() << "[DataService] Loaded" << localHistory.size() << "messages from local cache";
    // Для отображения истории на UI
    emit historyLoaded(chatPartner, localHistory);

    // Для запроса новых сообщений на сервере — определяем lastId
    int lastId = 0;
    if (!localHistory.isEmpty()) {
        lastId = localHistory.last().id;
    }

    // Логгируем факт запроса следующей порции истории по последнему id локального кэша
    qDebug() << "[DataService] Requesting history from server after id:" << lastId;

    // Сигнал на сервер — выделить и прислать обновлённую историю после lastId
    emit requestServerHistory(chatPartner, lastId);
}


ChatCache& DataService::getChatCacheRef(const QString& username) {
    return m_chatHistoryCache[username];
}


QSet<qint64> DataService::fetchExistingServerIds(const QString& chatPartner) {
    QSet<qint64> serverIds;
    QSqlQuery query;

    query.prepare("SELECT server_id FROM messages WHERE (from_user = ? OR to_user = ?) AND server_id NOT NULL");
    query.addBindValue(chatPartner);
    query.addBindValue(chatPartner);
    if (query.exec()) {
        while (query.next()) {
            serverIds.insert(query.value(0).toLongLong());
        }
    }
    return serverIds;
}


void DataService::insertMessagesWithUpsertFiltered(const QList<ChatMessage>& messages, const QString& chatPartner) {
    // Если список пуст — нечего обрабатывать
    if (messages.isEmpty())
        return;

    // Получаем множество всех уже сохранённых server_id — для быстрой проверки и фильтрации дублей
    QSet<qint64> localServerIds = fetchExistingServerIds(chatPartner);

    // Начинаем транзакцию для гарантированной атомарности и скорости пакетной записи
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare(
        "INSERT OR IGNORE INTO messages "
        "(server_id, temp_id, from_user, to_user, payload, timestamp, status, is_edited, reply_to_id, is_outgoing) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

    int inserted = 0;
    for (const ChatMessage& msg : messages) {
        // Фильтруем: если серверный id уже есть, не пытаемся вставить вновь (предотвращение дублей)
        if (msg.id > 0 && localServerIds.contains(msg.id))
            continue;

        // Добавляем значения для bind — полностью защищённый запрос
        query.addBindValue(msg.id);
        query.addBindValue(msg.tempId);
        query.addBindValue(msg.fromUser);
        query.addBindValue(msg.toUser);
        query.addBindValue(msg.payload);
        query.addBindValue(msg.timestamp);
        query.addBindValue(msg.status);
        query.addBindValue(msg.isEdited);
        query.addBindValue(msg.replyToId);
        query.addBindValue(msg.isOutgoing);

        // Если вставка прошла успешно — увеличиваем счётчик новых сообщений
        if (query.exec())
            inserted++;
    }

    // После всех вставок — коммитим транзакцию
    QSqlDatabase::database().commit();

    // Логгируем сколько реально новых сообщений добавили (отсекаем дублей)
    qDebug() << "[DataService] insertMessagesWithUpsertFiltered: inserted" << inserted << "filtered (was" << messages.size() << ")";
}

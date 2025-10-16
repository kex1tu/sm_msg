/**
 * @file chatmessagemodel.cpp
 * @brief Реализация модели данных для списка сообщений.
 * @see ChatMessageModel
 * @author kex1tu
 */

#include "chatmessagemodel.h"
#include <QDebug> // Для отладочных сообщений qDebug().

/**
 * @brief Конструктор модели.
 * @param parent Родительский объект в иерархии Qt.
 */
ChatMessageModel::ChatMessageModel(QObject *parent) : QAbstractListModel(parent)
{
    // Регистрируем наш кастомный тип ChatMessage в мета-объектной системе Qt.
    // Это необходимо для того, чтобы мы могли безопасно хранить
    // объекты ChatMessage внутри QVariant и передавать их через сигналы/слоты.
    qRegisterMetaType<ChatMessage>();
}

/**
 * @brief Возвращает количество строк (сообщений) в модели.
 * @details Этот метод вызывается представлением (QListView), чтобы узнать, сколько
 *          элементов нужно отобразить.
 * @param parent Для одноуровневой списочной модели этот параметр всегда невалидный.
 * @return int Количество сообщений во внутреннем списке `m_messages`.
 */
int ChatMessageModel::rowCount(const QModelIndex &parent) const
{
    // Q_UNUSED(parent); // Можно добавить, чтобы компилятор не ругался на неиспользуемый параметр.
    return m_messages.count();
}

/**
 * @brief Основной метод для получения данных из модели.
 * @details Представление (View) и делегат (Delegate) вызывают этот метод для
 *          получения данных, которые нужно отобразить.
 * @param index Индекс элемента (строка, колонка), для которого запрашиваются данные.
 * @param role Тип (роль) запрашиваемых данных (например, текст, цвет, иконка).
 * @return QVariant Данные, обернутые в QVariant. Пустой QVariant, если данные не найдены.
 */
QVariant ChatMessageModel::data(const QModelIndex &index, int role) const
{
    // Проверка на валидность индекса, чтобы избежать выхода за пределы массива.
    if (!index.isValid() || index.row() >= m_messages.count())
        return QVariant();

    // Получаем константную ссылку на объект сообщения по индексу.
    const ChatMessage &message = m_messages.at(index.row());

    // Наша основная логика: по роли Qt::UserRole мы возвращаем весь объект ChatMessage.
    // Делегат затем извлечет этот объект и возьмет из него все необходимые поля для отрисовки.
    if (role == Qt::UserRole) {
        return QVariant::fromValue(message);
    }

    // Для других стандартных ролей (DisplayRole, DecorationRole и т.д.) мы не возвращаем ничего.
    return QVariant();
}

/**
 * @brief Метод для изменения данных в модели.
 * @details Позволяет изменять данные элемента по его индексу. В нашем случае,
 *          мы используем его для обновления всего объекта ChatMessage целиком.
 * @param index Индекс изменяемого элемента.
 * @param value Новое значение (QVariant, содержащий ChatMessage).
 * @param role Роль, по которой изменяются данные (мы используем Qt::UserRole).
 * @return bool `true` в случае успеха, иначе `false`.
 */
bool ChatMessageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::UserRole) {
        return false;
    }

    const ChatMessage& msg = value.value<ChatMessage>();

    // Убеждаемся, что мы обновляем правильное сообщение по ID.
    if (m_messages[index.row()].id == msg.id) {
        m_messages[index.row()] = msg;
        // Эмитируем сигнал dataChanged, чтобы уведомить представление (View)
        // о том, что данные изменились и элемент нужно полностью перерисовать.
        emit dataChanged(index, index, {Qt::UserRole});
        return true;
    }

    return false;
}

/**
 * @brief Добавляет одно сообщение в конец модели.
 * @param message Объект сообщения для добавления.
 */
void ChatMessageModel::addMessage(const ChatMessage &message)
{
    const int newRow = m_messages.count();

    // Обязательный вызов ПЕРЕД изменением внутренней структуры данных.
    // Уведомляет представления, что сейчас будут вставлены строки с newRow по newRow.
    beginInsertRows(QModelIndex(), newRow, newRow);

    // Выполняем фактическое изменение данных.
    m_messages.append(message);
    if (message.id > 0) m_messageMap[message.id] = message; // Обновляем и карту быстрого доступа.

    // Обязательный вызов ПОСЛЕ изменения. Завершает операцию вставки.
    endInsertRows();
}

/**
 * @brief Добавляет список сообщений в конец модели (оптимизировано для массовой вставки).
 * @param messages Список сообщений.
 */
void ChatMessageModel::addMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;

    int first = m_messages.count();
    int last = first + messages.count() - 1;

    beginInsertRows(QModelIndex(), first, last);

    m_messages.append(messages);
    for(const auto& msg : messages){
        m_messageMap[msg.id] = msg;
    }

    endInsertRows();
}

/**
 * @brief Добавляет список сообщений в начало модели (используется для подгрузки старой истории).
 * @param messages Список сообщений для добавления.
 */
void ChatMessageModel::prependMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;

    beginInsertRows(QModelIndex(), 0, messages.count() - 1);

    // Итерируемся в обратном порядке, чтобы сохранить правильный порядок сообщений при вставке.
    for (int i = messages.count() - 1; i >= 0; --i) {
        const auto& msg = messages.at(i);
        m_messages.prepend(msg);
        m_messageMap[msg.id] = msg;
    }

    endInsertRows();
}

/**
 * @brief Полностью очищает модель от всех сообщений.
 */
void ChatMessageModel::clearMessages()
{
    if (m_messages.isEmpty()) return;

    // beginResetModel/endResetModel - это сигнал для представлений,
    // что модель будет полностью изменена, и им нужно сбросить все свои данные.
    beginResetModel();
    m_messages.clear();
    m_messageMap.clear();
    endResetModel();
}

/**
 * @brief Удаляет сообщение из модели по его ID.
 * @param messageId Уникальный ID сообщения.
 */
void ChatMessageModel::removeMessage(qint64 messageId)
{
    // Используем линейный поиск, так как нам нужен индекс для удаления.
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_messages.removeAt(i);
            m_messageMap.remove(messageId);
            endRemoveRows();
            return; // Выходим, так как ID уникален.
        }
    }
}

/**
 * @brief Находит сообщение по временному ID и заменяет его на подтвержденное сервером.
 * @param tempId Временный ID, сгенерированный клиентом при отправке.
 * @param confirmedMessage Объект сообщения, полученный от сервера (с настоящим ID).
 */
void ChatMessageModel::confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].tempId == tempId && !m_messages[i].tempId.isEmpty()) {
            m_messages[i] = confirmedMessage;
            m_messageMap[confirmedMessage.id] = confirmedMessage;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole}); // Уведомляем о необходимости перерисовки.
            return;
        }
    }
}

/**
 * @brief Обновляет статус доставки/прочтения для сообщения по его ID.
 * @param messageId ID сообщения.
 * @param newStatus Новый статус из `ChatMessage::MessageStatus`.
 */
void ChatMessageModel::updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus newStatus)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {
            m_messages[i].status = newStatus;
            if(m_messageMap.contains(messageId)) m_messageMap[messageId].status = newStatus;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole});
            return;
        }
    }
}

/**
 * @brief Обновляет текст отредактированного сообщения по его ID.
 * @param messageId ID сообщения.
 * @param newPayload Новый текст сообщения.
 */
void ChatMessageModel::editMessage(qint64 messageId, const QString& newPayload)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {
            m_messages[i].payload = newPayload;
            m_messages[i].isEdited = true;
            if(m_messageMap.contains(messageId)) {
                m_messageMap[messageId].payload  = newPayload;
                m_messageMap[messageId].isEdited  = true;
            }

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole});
            return;
        }
    }
}

/**
 * @brief Осуществляет быстрый поиск сообщения по ID с использованием `m_messageMap`.
 * @param id ID искомого сообщения.
 * @param msg Выходной параметр; ссылка, куда будет скопирован найденный объект сообщения.
 * @return bool `true`, если сообщение найдено, иначе `false`.
 */
bool ChatMessageModel::getMessageById(qint64 id, ChatMessage &msg) const
{
    if (m_messageMap.contains(id)) {
        msg = m_messageMap.value(id);
        return true;
    }
    return false;
}

/**
 * @brief Помечает входящее сообщение как прочитанное.
 * @details Этот слот вызывается асинхронно из делегата, когда сообщение
 *          становится видимым на экране, или из `MainWindow` после прокрутки.
 * @param index Индекс сообщения в модели.
 */
void ChatMessageModel::markMessageAsRead(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_messages.count())
        return;

    // Проверяем, что сообщение имеет статус "Доставлено", чтобы избежать
    // повторной отправки уведомлений о прочтении.
    if (m_messages[index.row()].status == ChatMessage::Delivered) {
        qint64 messageId = m_messages[index.row()].id;

        // Обновляем статус в наших внутренних структурах.
        m_messages[index.row()].status = ChatMessage::Read;
        if (m_messageMap.contains(messageId)) {
            m_messageMap[messageId].status = ChatMessage::Read;
        }

        // Уведомляем view о необходимости перерисовать этот элемент (чтобы обновить иконку статуса).
        emit dataChanged(index, index, {Qt::UserRole});

        // Испускаем сигнал, который будет пойман MainWindow для отправки уведомления на сервер.
        emit messageNeedsReadReceipt(messageId);
    }
}

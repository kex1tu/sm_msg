#include "chatmessagemodel.h"

ChatMessageModel::ChatMessageModel(QObject *parent) : QAbstractListModel(parent)
{
    qRegisterMetaType<ChatMessage>();
}


int ChatMessageModel::rowCount(const QModelIndex &parent) const
{
    return m_messages.count();
}


QVariant ChatMessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.count())
        return QVariant();

    const ChatMessage &message = m_messages.at(index.row());

    if (role == Qt::UserRole) {
        return QVariant::fromValue(message);
    }

    return QVariant();
}


void ChatMessageModel::addMessage(const ChatMessage &message)
{

    qDebug() << "Model: Adding message. Current count:" << m_messages.count();
    const int newRow = m_messages.count();


    beginInsertRows(QModelIndex(), newRow, newRow);

    m_messages.append(message);
    if (message.id > 0) m_messageMap[message.id] = message;  

    endInsertRows();
    qDebug() << "Model: Message added. New count:" << m_messages.count();
}

void ChatMessageModel::confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].tempId == tempId) {
            m_messages[i] = confirmedMessage;
            m_messageMap[confirmedMessage.id] = confirmedMessage;  

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole});

            qDebug() << "[MODEL] Confirmed message with tempId:" << tempId;
            return;
        }
    }
    qDebug() << "[MODEL][WARNING] Could not find message with tempId to confirm:" << tempId;
}

void ChatMessageModel::updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus newStatus)
{

    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {
            m_messages[i].status = newStatus;
            m_messageMap[messageId].status = newStatus;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole});

            qDebug() << "[MODEL] Updated status for message ID:" << messageId << "to" << newStatus;
            return;
        }
    }
    qDebug() << "[MODEL][WARNING] Could not find message with ID to update status:" << messageId;
}

void ChatMessageModel::editMessage(qint64 messageId, const QString& newPayload)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {

            qDebug() << "[MODEL] Found message to edit at index:" << i;
            m_messages[i].payload = newPayload;
            m_messages[i].isEdited = true;
            m_messageMap[messageId].payload  = newPayload;
            m_messageMap[messageId].isEdited  = true;

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {Qt::UserRole});


            qDebug() << "[MODEL] Edited message with ID:" << messageId;
            return;
        }
    }
    qDebug() << "[MODEL][WARNING] Could not find message with ID to edit:" << messageId;
}

void ChatMessageModel::removeMessage(qint64 messageId)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].id == messageId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_messages.removeAt(i);
            m_messageMap.remove(messageId);
            endRemoveRows();
            return;
        }
    }
}
bool ChatMessageModel::getMessageById(qint64 id, ChatMessage &msg) const
{
    if (m_messageMap.contains(id)) {
        msg = m_messageMap.value(id);
        return true;
    }
    return false;
}
void ChatMessageModel::addMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;
    qDebug() << "Model: about to insert" << messages.count() << "rows.";

    int first = m_messages.count();
    int last = first + messages.count() - 1;

     
    beginInsertRows(QModelIndex(), first, last);

     
    m_messages.append(messages);
    for(const auto& msg : messages){
        m_messageMap[msg.id] = msg;
    }

     
    endInsertRows();

    qDebug() << "Model: insertion finished. New total rows:" << m_messages.count();
}
void ChatMessageModel::prependMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;
    qDebug() << "Model: about to insert" << messages.count() << "rows at the beginning.";

     
    beginInsertRows(QModelIndex(), 0, messages.count() - 1);

     
    for (int i = messages.count() - 1; i >= 0; --i) {
        const auto& msg = messages.at(i);
        m_messages.prepend(msg);
        m_messageMap[msg.id] = msg;
    }

     
    endInsertRows();
    qDebug() << "Model: insertion finished. New total rows:" << m_messages.count();
}
void ChatMessageModel::clearMessages()
{
    if (m_messages.isEmpty()) return;
    beginResetModel();
    m_messages.clear();
    m_messageMap.clear();
    endResetModel();
}
void ChatMessageModel::markMessageAsRead(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_messages.count())
        return;

     
    if (m_messages[index.row()].status == ChatMessage::Delivered) {
        qint64 messageId = m_messages[index.row()].id;

         
        m_messages[index.row()].status = ChatMessage::Read;
        if (m_messageMap.contains(messageId)) {
            m_messageMap[messageId].status = ChatMessage::Read;
        }

         
        emit dataChanged(index, index, {Qt::UserRole});

         
        qDebug() << "[MODEL] Message" << messageId << "was rendered. Emitting receipt signal.";
        emit messageNeedsReadReceipt(messageId);
    }
}

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

    endInsertRows();
    qDebug() << "Model: Message added. New count:" << m_messages.count();
}

void ChatMessageModel::confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages[i].tempId == tempId) {
            m_messages[i] = confirmedMessage;

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
            endRemoveRows();
            return;
        }
    }
}

void ChatMessageModel::addMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;
    qDebug() << "Model: about to insert" << messages.count() << "rows.";

    int first = m_messages.count();

    m_messages.append(messages);

    int last = m_messages.count() - 1;
    beginInsertRows(QModelIndex(), first, last);
    endInsertRows();

    qDebug() << "Model: insertion finished. New total rows:" << m_messages.count();
}

void ChatMessageModel::prependMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;
    qDebug() << "Model: about to insert" << messages.count() << "rows.";


    beginInsertRows(QModelIndex(), 0, messages.count() - 1);
    for (int i = messages.count() - 1; i >= 0; --i) {
        m_messages.prepend(messages.at(i));
    }

    endInsertRows();
    qDebug() << "Model: insertion finished. New total rows:" << m_messages.count();
}

void ChatMessageModel::clearMessages()
{
    if (m_messages.isEmpty()) return;
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

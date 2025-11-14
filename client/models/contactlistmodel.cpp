#include "contactlistmodel.h"
#include "mainwindow.h"
#include "dataservice.h"
#include <QSet>
#include <QDebug>


ContactListModel::ContactListModel(DataService* dataService, QObject *parent)
    : QAbstractListModel(parent)
    , m_dataService(dataService)
{
    // Подписываемся на сигнал обновления списка контактов от DataService
    connect(m_dataService, &DataService::contactsUpdated, this, &ContactListModel::updateContacts);
}


int ContactListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_contactUsernames.count();
}


QVariant ContactListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    QString username = m_contactUsernames.at(index.row());
    User* userPtr = m_dataService->getUserFromCache(username);
    if (!userPtr) return QVariant();
    const User& user = *userPtr;

    switch (role) {
    case Qt::DisplayRole:
        return user.displayName;
    case UsernameRole:
        return user.username;
    case IsOnlineRole:
        return user.isOnline;
    case IsTypingRole:
        // Не показываем "печатает..." для текущего собеседника (себя)
        if (m_dataService->getCurrentChatPartner()) {
            return (user.isTyping && username != m_dataService->getCurrentChatPartner()->username);
        }
        return user.isTyping;
    case LastMessageRole: {
        ChatCache* cache = m_dataService->getChatCacheForUser(username);
        if (cache && !cache->messages.isEmpty()) {
            return cache->messages.last().payload;
        }
        return QVariant();
    }
    case UnreadCountRole: {
        QMap<QString, int>* counts = m_dataService->getUnreadCounts();
        if (counts) {
            return counts->value(username, 0);
        }
        return 0;
    }
    }
    return QVariant();
}


void ContactListModel::clear()
{
    beginResetModel();
    m_contactUsernames.clear();
    endResetModel();
    qDebug() << "[ContactListModel] Contact list cleared";
}


void ContactListModel::refreshContact(const QString &username)
{
    qDebug() << "Update user by name: " << username;
    int row = m_contactUsernames.indexOf(username);
    if (row != -1) {
        QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx);
    }
}


void ContactListModel::refreshContact(const QModelIndex &User)
{
    qDebug() << "Update user by index: " << User.data(Qt::UserRole);
    emit dataChanged(User, User);
}


void ContactListModel::updateContacts(const QStringList &newUsernames)
{
    qDebug() << "Update contactlist";
    const QStringList oldUsernames = m_contactUsernames;
    const QSet<QString> newUsernamesSet = QSet<QString>(newUsernames.begin(), newUsernames.end());
    const QSet<QString> oldUsernamesSet = QSet<QString>(oldUsernames.begin(), oldUsernames.end());

    // Удаляем тех, кого нет в новом списке
    for (int i = oldUsernames.count() - 1; i >= 0; --i) {
        if (!newUsernamesSet.contains(oldUsernames[i])) {
            beginRemoveRows(QModelIndex(), i, i);
            m_contactUsernames.removeAt(i);
            endRemoveRows();
        }
    }

    // Добавляем новых из списка, если их нет
    for (int i = 0; i < newUsernames.count(); ++i) {
        if (!oldUsernamesSet.contains(newUsernames[i])) {
            beginInsertRows(QModelIndex(), i, i);
            m_contactUsernames.insert(i, newUsernames[i]);
            endInsertRows();
        }
    }
    qDebug() << "[ContactListModel] Contacts updated, total:" << m_contactUsernames.size();
}

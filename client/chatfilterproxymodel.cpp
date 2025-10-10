#include "chatmessagemodel.h"
#include "chatfilterproxymodel.h"
#include <QModelIndex>


ChatFilterProxyModel::ChatFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ChatFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    ChatMessage msg = sourceModel()->data(index, Qt::UserRole).value<ChatMessage>();

    return msg.payload.contains(filterRegularExpression());
}

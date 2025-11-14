#include "chatmessagemodel.h"
#include "chatfilterproxymodel.h"
#include <QModelIndex>


ChatFilterProxyModel::ChatFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    qDebug() << "[ProxyModel] Создан прокси-фильтр для истории чата";
}


bool ChatFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid()) {
        qDebug() << "[ProxyModel] filterAcceptsRow: invalid index" << source_row;
        return false;
    }

    ChatMessage msg = sourceModel()->data(sourceIndex, Qt::UserRole).value<ChatMessage>();
    const QRegularExpression& regex = filterRegularExpression();

    if (!regex.isValid() || regex.pattern().isEmpty()) {
        qDebug() << "[ProxyModel] filterAcceptsRow: пустой фильтр — показываем всё";
        return true;
    }

    bool matched = regex.match(msg.payload).hasMatch();
    qDebug() << "[ProxyModel] filterAcceptsRow:" << msg.id << "payload:" << msg.payload << "match:" << matched;
    return matched;
}

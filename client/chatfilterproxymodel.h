#ifndef CHATFILTERPROXYMODEL_H
#define CHATFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class ChatFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit ChatFilterProxyModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // CHATFILTERPROXYMODEL_H

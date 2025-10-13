
#ifndef CONTACTLISTDELEGATE_H
#define CONTACTLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QMap>  

class ContactListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
     
    explicit ContactListDelegate(const QMap<QString, int>* unreadCounts, QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const QMap<QString, int>* m_unreadCounts;
};

#endif  


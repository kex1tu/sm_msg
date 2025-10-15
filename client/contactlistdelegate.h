#ifndef CONTACTLISTDELEGATE_H
#define CONTACTLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QMap>
#include <QTimer>  
#include "structures.h"  

 
class MainWindow;

class ContactListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
     
    explicit ContactListDelegate(
        const QMap<QString, User>* userCache,
        const QMap<QString, ChatCache>* chatCache,
        const QMap<QString, int>* unreadCounts,
        const QString* currentChatUsername,
        QObject *parent = nullptr
        );

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
     
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const QMap<QString, User>* m_userCache;
    const QMap<QString, ChatCache>* m_chatCache;
    const QMap<QString, int>* m_unreadCounts;
    const QString* m_currentChatUsername;
};

#endif  

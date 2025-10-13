#ifndef CHATMESSAGEDELEGATE_H
#define CHATMESSAGEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include "structures.h"
class ChatMessageModel;  

class ChatMessageDelegate : public QStyledItemDelegate
{
public:
    explicit ChatMessageDelegate(const ChatMessageModel* model, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    const ChatMessageModel* m_model;  
};

#endif  

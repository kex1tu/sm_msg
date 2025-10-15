#ifndef CHATMESSAGEDELEGATE_H
#define CHATMESSAGEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QMap>
#include "structures.h"

class ChatMessageModel;
class QSvgRenderer;


class ChatMessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
public:
    explicit ChatMessageDelegate(const ChatMessageModel* model, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    const ChatMessageModel* m_model;
    static QMap<ChatMessage::MessageStatus, QSvgRenderer*> m_statusRenderers;
    static bool m_renderersInitialized;

    static void initRenderers(QObject* parent);
};

#endif

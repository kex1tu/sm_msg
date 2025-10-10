#include "chatmessagedelegate.h"
#include <QPainter>
#include "structures.h"

ChatMessageDelegate::ChatMessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rect = option.rect;
    int margin = 10;
    int padding = 10;
    int borderRadius = 15;

    QRect textRect = rect.adjusted(margin + padding, padding, -margin - padding, -padding);
    QFontMetrics fm(painter->font());
    textRect = fm.boundingRect(textRect, Qt::TextWordWrap, message.payload);

    QRect bubbleRect = textRect.adjusted(-padding, -padding, padding, padding);

    if (message.isOutgoing) {
        bubbleRect.moveRight(rect.right() - margin);
    } else {
        bubbleRect.moveLeft(rect.left() + margin);
    }

    textRect.moveCenter(bubbleRect.center());


    QColor bubbleColor = message.isOutgoing ? QColor("#E072A4") : QColor("#3D383A");
    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);

    painter->setPen(Qt::white);
    painter->drawText(textRect, Qt::TextWordWrap, message.payload);


    QRect timeRect = bubbleRect;
    timeRect.setTop(bubbleRect.bottom() + 2);
    timeRect.setHeight(fm.height());
    painter->setPen(Qt::gray);
    painter->drawText(timeRect, message.isOutgoing ? Qt::AlignRight : Qt::AlignLeft, message.timestamp.mid(11, 5)); // "HH:mm"

    painter->restore();
}

QSize ChatMessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    int margin = 10;
    int padding = 10;

    int textWidth = option.rect.width() - 2 * (margin + padding);

    QFontMetrics fm(option.font);
    QRect textRect = fm.boundingRect(QRect(0, 0, textWidth, 0), Qt::TextWordWrap, message.payload);

    QSize finalSize = QSize(option.rect.width(), textRect.height() + 2 * padding + fm.height());


    return finalSize;
}

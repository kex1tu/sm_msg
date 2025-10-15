#include "chatmessagedelegate.h"
#include <QPainter>
#include "structures.h"
#include "chatmessagemodel.h"
#include <algorithm>
#include <QSvgRenderer>
#include <QPainterPath>

QMap<ChatMessage::MessageStatus, QSvgRenderer*> ChatMessageDelegate::m_statusRenderers;
bool ChatMessageDelegate::m_renderersInitialized = false;

void ChatMessageDelegate::initRenderers(QObject* parent)
{
    if (m_renderersInitialized) return;

    qDebug() << "[Delegate] Инициализация SVG-рендереров...";

    m_statusRenderers[ChatMessage::Sending] = new QSvgRenderer(QString(":/icons/clock_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Sent] = new QSvgRenderer(QString(":/icons/message_send_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Delivered] = new QSvgRenderer(QString(":/icons/message_read_icon.svg"), parent);

    m_renderersInitialized = true;
    qDebug() << "[Delegate] Рендереры созданы.";
}


ChatMessageDelegate::ChatMessageDelegate(const ChatMessageModel* model, QObject *parent)
    : QStyledItemDelegate(parent), m_model(model)
{
    initRenderers(this);
}



void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    // ChatMessage messageForStatusCheck = index.data(Qt::UserRole).value<ChatMessage>();

     
    // if (!messageForStatusCheck.isOutgoing && messageForStatusCheck.status == ChatMessage::Delivered) {
         
         
         
         
    //     QMetaObject::invokeMethod(
    //         const_cast<QAbstractItemModel*>(index.model()),
    //         "markMessageAsRead",
    //         Qt::QueuedConnection,
    //         Q_ARG(QModelIndex, index)
    //         );
    // }


    QStyleOptionViewItem opt = option;
    opt.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);


    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    QRect originalRect = option.rect;
    int margin = 10;
    int padding = 10;
    int borderRadius = 15;
    int verticalSpacing = 10;
    QFontMetrics fm(painter->font());


    QRect contentRect = originalRect.adjusted(0, verticalSpacing / 2, 0, -verticalSpacing / 2);


    ChatMessage repliedMsg;



    int quoteHeight = 0;
    bool foundReply = false;
    if (message.replyToId > 0) {
        foundReply = m_model->getMessageById(message.replyToId, repliedMsg);
    }
     


    if (foundReply) {
        quoteHeight = fm.height() * 2 + 15;




        int nameWidth = fm.horizontalAdvance(repliedMsg.fromUser);


        QString elidedText = fm.elidedText(repliedMsg.payload, Qt::ElideRight, 250);
        int textWidth = fm.horizontalAdvance(elidedText);


        int quoteContentWidth = std::max(nameWidth, textWidth);
        int quoteTotalWidth = quoteContentWidth + padding + 4 + 8;


        QRectF quoteRect(0, 0, quoteTotalWidth, quoteHeight - 5);


        if (message.isOutgoing) {
            quoteRect.moveTopRight(contentRect.topRight() - QPoint(margin, 0));
        } else {
            quoteRect.moveTopLeft(contentRect.topLeft() + QPoint(margin, 0));
        }


        QRectF colorBarRect = quoteRect.adjusted(padding, 4, 0, -4);
        colorBarRect.setWidth(4);
        painter->setBrush(QColor("#E072A4"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(colorBarRect, 2, 2);


        QRectF quoteTextRect = quoteRect.adjusted(colorBarRect.width() + padding + 4, 5, -padding, -5);
        painter->setPen(QColor("#E072A4"));
        painter->drawText(quoteTextRect, Qt::AlignTop | Qt::AlignLeft, repliedMsg.fromUser);

        painter->setPen(Qt::gray);
        QRectF repliedTextRect = quoteTextRect.adjusted(0, fm.height(), 0, 0);

        painter->drawText(repliedTextRect, Qt::AlignTop | Qt::AlignLeft, elidedText);


        contentRect.setTop(contentRect.top() + quoteHeight);

    }


    int textWidth = contentRect.width() * 0.75 - 2 * padding;
    if (textWidth <= 0) textWidth = 300;

    QRect payloadRect = fm.boundingRect(QRect(0, 0, textWidth, 0), Qt::TextWrapAnywhere, message.payload);
    int metaDataHeight = fm.height();

    QString metaText;
    if (message.isEdited) metaText += "(изм.) ";
    metaText += message.timestamp.mid(11, 5);

    int metaTextWidth = fm.horizontalAdvance(metaText);

    int bubbleContentWidth = std::max(payloadRect.width(), metaTextWidth);
    int bubbleContentHeight = payloadRect.height() + metaDataHeight + 4;

    QRect bubbleRect(0, 0, bubbleContentWidth + 2 * padding, bubbleContentHeight + 2 * padding);

    int minBubbleWidth = 100;
    if (bubbleRect.width() < minBubbleWidth) bubbleRect.setWidth(minBubbleWidth);

    if (message.isOutgoing) bubbleRect.moveTopRight(contentRect.topRight() - QPoint(margin, 0));
    else bubbleRect.moveTopLeft(contentRect.topLeft() + QPoint(margin, 0));




    QColor bubbleColor = message.isOutgoing ? QColor("#E072A4").darker(150) : QColor("#3D383A");
     

    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);


    QRect textDrawRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    textDrawRect.setHeight(payloadRect.height());
    painter->setPen(Qt::white);
     
    QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);  

     
    textOption.setWrapMode(QTextOption::WrapAnywhere);

     
    painter->drawText(textDrawRect, message.payload, textOption);


    QRect baseMetaRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    baseMetaRect.setTop(textDrawRect.bottom() + 4);
    baseMetaRect.setHeight(metaDataHeight);

    if (message.isOutgoing) {
        int iconSize = fm.height() - 2;
        int iconPadding = 3;

        QRect iconRect(
            baseMetaRect.right() - iconSize,
            baseMetaRect.top() + (baseMetaRect.height() - iconSize) / 2,
            iconSize,
            iconSize
            );

        QRect textMetaRect = baseMetaRect;
        textMetaRect.setRight(iconRect.left() - iconPadding);

        QPen textPen = (message.isOutgoing && message.status == ChatMessage::Read) ? QColor(70, 150, 255) : Qt::gray;
        painter->setPen(textPen);
        painter->drawText(textMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);

        if (message.isOutgoing) {
            ChatMessage::MessageStatus statusToRender = message.status;
            if (statusToRender == ChatMessage::Read) {
                statusToRender = ChatMessage::Delivered;
            }

            QSvgRenderer* renderer = m_statusRenderers.value(statusToRender, nullptr);

            if (renderer && renderer->isValid()) {
                QPixmap pixmap(iconRect.size());
                pixmap.fill(Qt::transparent);

                QPainter pixmapPainter(&pixmap);

                renderer->render(&pixmapPainter);
                pixmapPainter.end();

                QPainter effectPainter(&pixmap);
                effectPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);

                QColor iconColor = (message.status == ChatMessage::Read) ? QColor(70, 150, 255) : Qt::gray;

                effectPainter.fillRect(pixmap.rect(), iconColor);
                effectPainter.end();

                painter->drawPixmap(iconRect, pixmap);
            }
        }
    }
    else {
        painter->setPen(Qt::gray);
        painter->drawText(baseMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);
    }

    painter->restore();
}

QSize ChatMessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    int padding = 10;
    int verticalSpacing = 10;
    QFontMetrics fm(option.font);


    int quoteHeight = 0;
    ChatMessage repliedMsg;
    bool foundReply = false;
    if (message.replyToId > 0) {
        foundReply = m_model->getMessageById(message.replyToId, repliedMsg);
    }
     


    if (foundReply) {
        quoteHeight = fm.height() * 2 + 15;
    }




    int textWidth = option.rect.width() * 0.75 - 2 * padding;
    if (textWidth <= 0) textWidth = 300;


    QRect payloadRect = fm.boundingRect(QRect(0, 0, textWidth, 0), Qt::TextWrapAnywhere, message.payload);


    int metaDataHeight = fm.height();


    int bubbleHeight = payloadRect.height() + metaDataHeight + 4 + (2 * padding);


    int totalHeight = quoteHeight + bubbleHeight + verticalSpacing;

    return QSize(option.rect.width(), totalHeight);
}

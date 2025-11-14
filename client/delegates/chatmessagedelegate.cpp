#include "chatmessagedelegate.h"
#include "chatmessagemodel.h"
#include "structures.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QPainterPath>
#include <algorithm>
#include <QTextDocument>


QMap<ChatMessage::MessageStatus, QSvgRenderer*> ChatMessageDelegate::m_statusRenderers;


bool ChatMessageDelegate::m_renderersInitialized = false;


ChatMessageDelegate::ChatMessageDelegate(const ChatMessageModel* model, QObject *parent)
    : QStyledItemDelegate(parent), m_model(model)
{
    // При создании любого делегата гарантируем инициализацию SVG-рендереров (статически, только 1 раз)
    initRenderers(this);
}


ChatMessageDelegate::~ChatMessageDelegate()
{
    // Освобождаем кешируются QTextDocument (пересоздаются по необходимости)
    qDeleteAll(m_documentCache);
}


void ChatMessageDelegate::initRenderers(QObject* parent)
{
    // Не допускаем повторной инициализации (есть static-флаг, один раз на всё приложение)
    if (m_renderersInitialized) return;

    qDebug() << "[Delegate] Инициализация SVG-рендереров...";

    // Для каждого статуса создаём SVG-рендерер и вешаем его на parent
    m_statusRenderers[ChatMessage::Sending]   = new QSvgRenderer(QString(":/icons/icons/clock_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Sent]      = new QSvgRenderer(QString(":/icons/icons/message_send_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Delivered] = new QSvgRenderer(QString(":/icons/icons/message_read_icon.svg"), parent);

    m_renderersInitialized = true;
    qDebug() << "[Delegate] Рендереры созданы.";
}


void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Базовая подготовка: сброс, сохранение настроек и антиалиасинг для плавного текста и формы пузыря
    QStyleOptionViewItem opt = option;
    QStyledItemDelegate::paint(painter, opt, index);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Извлекаем сообщение и основные размеры
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    const QRect& originalRect = option.rect;
    QFontMetrics fm(painter->font());

    // Настройки визуала пузыря чата
    const int margin = 10, padding = 10, borderRadius = 15, verticalSpacing = 10, minBubbleWidth = 100;
    int currentY = originalRect.top() + verticalSpacing / 2;

    // Подготовка цитаты, если это reply
    int quoteHeight = 0, quoteTextWidth = 0;
    if (message.replyToId > 0) {
        ChatMessage repliedMsg;
        m_model->getMessageById(message.replyToId, repliedMsg);

        QString fromUser = repliedMsg.fromUser.isEmpty() ? "НЕ ЗАГРУЖЕНО" : repliedMsg.fromUser;
        QString payload = repliedMsg.payload.isEmpty() ? "НЕ ЗАГРУЖЕНО" : repliedMsg.payload;

        int fromUserWidth = fm.horizontalAdvance(fromUser);
        int payloadWidth = fm.horizontalAdvance(payload);
        quoteTextWidth = std::max(fromUserWidth, payloadWidth) + 3 * padding + 5;
        if (quoteTextWidth > 400) quoteTextWidth = 400;
        quoteHeight = fm.height() * 2 + 15;
    }

    // Ширина области для текста сообщения
    int textMaxWidth = originalRect.width() * 0.75 - 2 * padding;
    if (textMaxWidth <= 0) textMaxWidth = 400;
    if (textMaxWidth > 400) textMaxWidth = 400;

    // Считаем/достаём готовую QTextDocument по ключу (id + ширина)
    QPair<qint64, int> cacheKey(message.id > 0 ? message.id : -index.row(), textMaxWidth);
    QTextDocument* doc = m_documentCache.value(cacheKey, nullptr);
    qreal textHeight = doc ? doc->size().height() : fm.boundingRect(QRect(0, 0, textMaxWidth, 0), Qt::TextWrapAnywhere, message.payload).height();
    qreal textActualWidth = doc ? doc->idealWidth() : fm.boundingRect(QRect(0, 0, textMaxWidth, 0), Qt::TextWrapAnywhere, message.payload).width();

    // Формируем мету: "изм. 12:34" + длина
    QString metaText;
    if (message.isEdited) metaText += "(изм.) ";
    metaText += message.timestamp.mid(11, 5);
    int metaTextWidth = fm.horizontalAdvance(metaText);
    int metaDataHeight = fm.height();
    if (message.isOutgoing) metaTextWidth += fm.height();

    // Определяем ширину и высоту пузыря: учитываем все виды ширин (text, meta, quote)
    int bubbleContentWidth = std::max({static_cast<int>(textActualWidth), metaTextWidth, quoteTextWidth});
    int bubbleContentHeight = textHeight + metaDataHeight + quoteHeight;
    QRect bubbleRect(0, 0, bubbleContentWidth + 2 * padding, bubbleContentHeight + 2 * padding);
    if (bubbleRect.width() < minBubbleWidth) bubbleRect.setWidth(minBubbleWidth);

    // Смещаем пузырь: вправо для исходящих, влево для входящих
    if (message.isOutgoing)
        bubbleRect.moveTopRight(QPoint(originalRect.right() - margin, currentY));
    else
        bubbleRect.moveTopLeft(QPoint(originalRect.left() + margin, currentY));

    // Основные цвета фона: outgoing/incoming
    QColor bubbleColor = message.isOutgoing ? QColor("#753955") : QColor("#3D383A");
    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);

    // Рисуем цитату (quote/reply) если есть replyToId
    if (quoteHeight > 0) {
        ChatMessage repliedMsg;
        QString fromUser = "НЕ ЗАГРУЖЕНО", payload = "НЕ ЗАГРУЖЕНО";
        if(m_model->getMessageById(message.replyToId, repliedMsg)){
            fromUser = repliedMsg.fromUser;
            payload = repliedMsg.payload;
        }

        QRectF quoteRect = bubbleRect.adjusted(padding, padding,  -padding, -padding - metaDataHeight - textHeight);
        QColor quoteRectcolor = message.isOutgoing ? QColor("#ff7fbb").darker(150) : QColor("#ff7fbb");
        painter->setBrush(quoteRectcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(quoteRect, 5, 5);

        // Белая полоса сбоку цитаты
        QRectF colorBarRect = quoteRect.adjusted(0, 0, 0, 0);
        colorBarRect.setWidth(5);
        painter->setBrush(QColor("#ffffff"));
        painter->drawRoundedRect(colorBarRect, 0, 0);

        // Текст цитаты: от кого и payload
        QRectF quoteTextRect = quoteRect.adjusted(colorBarRect.width() + padding, 5, -padding, -5);
        painter->setPen(QColor("#ffffff"));
        painter->drawText(quoteTextRect, Qt::AlignTop | Qt::AlignLeft, fm.elidedText(fromUser, Qt::ElideRight, bubbleContentWidth));
        QRectF repliedTextRect = quoteTextRect.adjusted(0, fm.height(), 0, 0);
        painter->drawText(repliedTextRect, Qt::AlignTop | Qt::AlignLeft, fm.elidedText(payload, Qt::ElideRight, bubbleContentWidth));
    }

    // Основной текст сообщения внутри пузыря
    QRect textDrawRect = bubbleRect.adjusted(padding, padding , -padding, -padding);
    textDrawRect.setHeight(textHeight);
    textDrawRect.moveTop(bubbleRect.top() +  quoteHeight + padding);
    painter->setPen(Qt::white);

    if (doc) {
        painter->save();
        painter->translate(textDrawRect.topLeft());
        painter->setPen(Qt::white);
        doc->drawContents(painter);
        painter->restore();
        qDebug() << "[Delegate] Отрисован QTextDocument для сообщения" << message.id;
    }  else {
        QTextOption textOption(Qt::AlignLeft | Qt::AlignTop);
        textOption.setWrapMode(QTextOption::WrapAnywhere);
        painter->drawText(textDrawRect, message.payload, textOption);
        qDebug() << "[Delegate] Отрисован drawText для сообщения" << message.id;
    }

    // Meta-инфо и иконка статуса
    QRect baseMetaRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    baseMetaRect.setHeight(metaDataHeight);
    baseMetaRect.moveBottom(bubbleRect.bottom() - padding);

    if (message.isOutgoing) {
        int iconSize = fm.height() - 2;
        QRect iconRect(baseMetaRect.right() - iconSize, baseMetaRect.top() + (baseMetaRect.height() - iconSize) / 2, iconSize, iconSize);
        QRect textMetaRect = baseMetaRect;
        textMetaRect.setRight(iconRect.left() - 3);

        QPen textPen = (message.status == ChatMessage::Read) ? QColor(70, 150, 255) : Qt::gray;
        painter->setPen(textPen);
        painter->drawText(textMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);

        // Иконка статуса сообщения (SVG)
        ChatMessage::MessageStatus statusToRender = message.status;
        if (statusToRender == ChatMessage::Read) statusToRender = ChatMessage::Delivered;

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
        } else {
            painter->setPen(Qt::gray);
            painter->drawText(baseMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);
        }
    } else {
        painter->setPen(Qt::gray);
        painter->drawText(baseMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);
    }

    painter->restore();
}


QSize ChatMessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    QFontMetrics fm(option.font);

    const int padding = 10;
    const int verticalSpacing = 10;
    const int minBubbleWidth = 100;

    // Quote-calc: если есть replyToId — расчитываем высоту и ширину цитаты
    int quoteHeight = 0, quoteTextWidth = 0;
    if (message.replyToId > 0) {
        ChatMessage repliedMsg;
        m_model->getMessageById(message.replyToId, repliedMsg);

        QString fromUser = repliedMsg.fromUser.isEmpty() ? "НЕ ЗАГРУЖЕНО" : repliedMsg.fromUser;
        QString payload = repliedMsg.payload.isEmpty() ? "НЕ ЗАГРУЖЕНО" : repliedMsg.payload;
        int fromUserWidth = fm.horizontalAdvance(fromUser);
        int payloadWidth = fm.horizontalAdvance(payload);

        quoteTextWidth = std::max(fromUserWidth, payloadWidth) + 3 * padding + 5;
        if (quoteTextWidth > 400) quoteTextWidth = 400;
        quoteHeight = fm.height() * 2 + 15;
        qDebug() << "[Delegate] sizeHint: расчет цитаты для" << message.id << quoteTextWidth << quoteHeight;
    }

    // Максимальная ширина текста (bubble), коррекция под ограничения
    int textMaxWidth = option.rect.width() * 0.75 - 2 * padding;
    if (textMaxWidth <= 0) textMaxWidth = 400;
    if (textMaxWidth > 400) textMaxWidth = 400;

    // Использование/создание QTextDocument для кеша ширины текста
    QTextDocument* doc = nullptr;
    QPair<qint64, int> cacheKey(message.id > 0 ? message.id : -index.row(), textMaxWidth);
    if (m_documentCache.contains(cacheKey)) {
        doc = m_documentCache.value(cacheKey);
        // Обновляем если текст изменился (например, редактирование или смена ширины)
        if (doc->toPlainText() != message.payload) {
            doc->setPlainText(message.payload);
            doc->setTextWidth(textMaxWidth);
            qDebug() << "[Delegate] sizeHint: обновлён QTextDocument кеш для" << message.id;
        }
    } else {
        doc = new QTextDocument();
        doc->setDefaultFont(option.font);
        doc->setPlainText(message.payload);
        doc->setTextWidth(textMaxWidth);
        m_documentCache.insert(cacheKey, doc);
        qDebug() << "[Delegate] sizeHint: создан новый QTextDocument для" << message.id;
    }

    qreal textHeight = doc->size().height();
    qreal textActualWidth = doc->idealWidth();

    // Meta-string (время и статус, например "(изм.) 12:34")
    QString metaText;
    if (message.isEdited) metaText += "(изм.) ";
    metaText += message.timestamp.mid(11, 5);
    int metaTextWidth = fm.horizontalAdvance(metaText);
    int metaDataHeight = fm.height();
    if (message.isOutgoing) metaTextWidth += fm.height();

    // Bubble-width: максимум из текста, meta и quote
    int bubbleContentWidth = std::max({static_cast<int>(textActualWidth), metaTextWidth, quoteTextWidth});
    if (bubbleContentWidth < minBubbleWidth)
        bubbleContentWidth = minBubbleWidth;

    // Итоговая высота: текст + meta + цитата + padding
    int bubbleContentHeight = textHeight + metaDataHeight + (2 * padding) + quoteHeight;

    // Итог по всему элементу (bubble + verticalSpacing между сообщениями)
    int totalHeight = bubbleContentHeight + verticalSpacing;

    return QSize(bubbleContentWidth + 2 * padding, totalHeight);
}


void ChatMessageDelegate::clearSizeHintCache()
{
    m_sizeHintCache.clear();
    qDebug() << "[Delegate] Кеш размеров sizeHint очищен";
}


void ChatMessageDelegate::clearCaches()
{
    qDeleteAll(m_documentCache);
    m_documentCache.clear();
    qDebug() << "[Delegate] Все кеши QTextDocument очищены";
}

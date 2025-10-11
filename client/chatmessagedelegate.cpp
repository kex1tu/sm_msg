#include "chatmessagedelegate.h"
#include <QPainter>
#include "structures.h"

ChatMessageDelegate::ChatMessageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

/*
void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rect = option.rect;
    int verticalSpacing = 20;
    int margin = 10;
    int padding = 10;
    int borderRadius = 15;
    rect.adjust(0, verticalSpacing / 2, 0, -verticalSpacing / 2);
    QFontMetrics fm(painter->font());

    QString payloadText = message.payload;
    QString metaText;
    if (message.isEdited) {
        metaText += "(изм.) ";
    }
    metaText += message.timestamp.mid(11, 5); // "HH:mm"

    if (message.isOutgoing) {
        if (message.status == ChatMessage::Read) metaText += " ✔✔";
        else if (message.status == ChatMessage::Delivered) metaText += " ✔✔";
        else if (message.status == ChatMessage::Sent) metaText += " ✔";
        else if (message.status == ChatMessage::Sending) metaText += " 🕒";
    }


    // --- 2. РАССЧИТЫВАЕМ НЕОБХОДИМУЮ ГЕОМЕТРИЮ ---

    // Прямоугольник для основного текста
    QRect availableTextRect = rect.adjusted(margin + padding, padding, -margin - padding, -padding);
    QRect payloadBoundingRect = fm.boundingRect(availableTextRect, Qt::TextWordWrap, payloadText);

    // Ширина, необходимая для метаданных (без переноса строк)
    int metaTextWidth = fm.horizontalAdvance(metaText);

    // --- КЛЮЧЕВОЕ ИЗМЕНЕНИЕ ---
    // Ширина "пузыря" - это МАКСИМУМ из ширины payload и ширины метаданных.
    int bubbleContentWidth = std::max(payloadBoundingRect.width(), metaTextWidth);

    // Рассчитываем финальный bubbleRect
    QRect bubbleRect(0, 0, bubbleContentWidth + 2 * padding, payloadBoundingRect.height() + 2 * padding);

    // --- 3. ВЫРАВНИВАНИЕ И ОТРИСОВКА ---

    // Выравниваем пузырь
    if (message.isOutgoing) {
        bubbleRect.moveTopRight(rect.topRight() - QPoint(margin, -padding)); // отступ сверху
    } else {
        bubbleRect.moveTopLeft(rect.topLeft() + QPoint(margin, padding));
    }

    // 2. РИСУЕМ ФОН ПУЗЫРЯ САМИ, с учетом :hover
    QColor bubbleColor = message.isOutgoing ? QColor("#E072A4") : QColor("#3D383A");

    // Если на элемент наведен курсор, делаем цвет пузыря чуть светлее
    if (option.state & QStyle::State_MouseOver) {
        bubbleColor = bubbleColor.lighter(115); // Увеличить яркость на 15%
    }

    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);

    // Рисуем основной текст
    QRect textDrawRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    painter->setPen(Qt::white);
    painter->drawText(textDrawRect, Qt::TextWordWrap |Qt::AlignRight, payloadText);

    // Рисуем метаданные
    QRect metaRect = bubbleRect;
    metaRect.setTop(bubbleRect.bottom());
    metaRect.setHeight(fm.height() + 4);
    metaRect.adjust(0, 0, -padding/2, 0); // Небольшой отступ справа

    painter->setPen(Qt::gray);
    if (message.isOutgoing && message.status == ChatMessage::Read) {
        painter->setPen(QColor(70, 150, 255));
    }
    painter->drawText(metaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);

    painter->restore();
}
*/

void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Отключаем стандартную отрисовку фона, чтобы самим все контролировать
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_Selected;
    opt.state &= ~QStyle::State_MouseOver;
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // --- 1. Получаем данные и базовую геометрию ---
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    QRect rect = option.rect;
    int margin = 10;
    int padding = 10;
    int borderRadius = 15;
    int verticalSpacing = 10;
    QFontMetrics fm(painter->font());

    // --- 2. Рассчитываем необходимую ширину ---
    int contentWidth = rect.width() - 2 * (margin + padding);
    if (contentWidth <= 0) contentWidth = 300;

    QRect payloadRect = fm.boundingRect(QRect(0, 0, contentWidth, 0), Qt::TextWordWrap, message.payload);

    QString metaText;

    if (message.isEdited) {
        metaText += "(изм.) ";
    }
    metaText += message.timestamp.mid(11, 5); // "HH:mm"

    if (message.isOutgoing) {
        if (message.status == ChatMessage::Read) metaText += " ✔✔";
        else if (message.status == ChatMessage::Delivered) metaText += " ✔✔";
        else if (message.status == ChatMessage::Sent) metaText += " ✔";
        else if (message.status == ChatMessage::Sending) metaText += " 🕒";
    }
    int metaTextWidth = fm.horizontalAdvance(metaText);

    // Ширина контента = максимум из ширины текста и метаданных
    int bubbleContentWidth = std::max(payloadRect.width(), metaTextWidth);

    // --- 3. Рассчитываем финальную геометрию ---

    // Высота контента
    int bubbleContentHeight = payloadRect.height() + fm.height() + 4; // текст + метаданные + зазор

    // Финальный "пузырь"
    QRect bubbleRect(0, 0, bubbleContentWidth + 2 * padding, bubbleContentHeight + 2 * padding);

    // Выравниваем пузырь
    if (message.isOutgoing) {
        bubbleRect.moveTopRight(rect.topRight() - QPoint(margin, -(verticalSpacing / 2)));
    } else {
        bubbleRect.moveTopLeft(rect.topLeft() + QPoint(margin, verticalSpacing / 2));
    }

    // --- 4. Отрисовка ---

    // Рисуем фон пузыря с учетом наведения
    QColor bubbleColor = message.isOutgoing ? QColor("#E072A4") : QColor("#3D383A");
    if (option.state & QStyle::State_MouseOver) {
        bubbleColor = bubbleColor.lighter(120);
    }
    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);

    // Рисуем основной текст в верхней части пузыря
    QRect textDrawRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    textDrawRect.setHeight(payloadRect.height()); // Ограничиваем высоту
    painter->setPen(Qt::white);
    painter->drawText(textDrawRect, Qt::TextWordWrap, message.payload);

    // Рисуем метаданные ВНУТРИ пузыря, в правом нижнем углу
    QRect metaRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    painter->setPen(Qt::gray);
    if (message.isOutgoing && message.status == ChatMessage::Read) {
        painter->setPen(QColor(220, 240, 255, 200)); // Светлый сине-белый для прочитанных
    }
    painter->drawText(metaRect, Qt::AlignRight | Qt::AlignBottom, metaText);

    painter->restore();
}

QSize ChatMessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();

    int margin = 10;
    int padding = 10;
    int verticalSpacing = 10;
    QFontMetrics fm(option.font);

    // Ширина, доступная для контента
    int contentWidth = option.rect.width() - 2 * (margin + padding);
    if (contentWidth <= 0) contentWidth = 300;

    // --- НОВАЯ ЛОГИКА РАСЧЕТА ВЫСОТЫ ---

    // 1. Рассчитываем высоту для основного текста (payload)
    QRect payloadRect = fm.boundingRect(QRect(0, 0, contentWidth, 0), Qt::TextWordWrap, message.payload);

    // 2. Рассчитываем высоту для строки метаданных
    int metaDataHeight = fm.height();

    // 3. Общая высота контента = высота текста + высота метаданных + небольшой зазор между ними
    int totalContentHeight = payloadRect.height() + metaDataHeight + 4;

    // 4. Полная высота ячейки = высота контента + верхний/нижний паддинги + отступ между сообщениями
    int finalHeight = totalContentHeight + 2 * padding + verticalSpacing;

    return QSize(option.rect.width(), finalHeight);
}

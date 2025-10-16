/**
 * @file chatmessagedelegate.cpp
 * @brief Реализация делегата для списка сообщений.
 * @see ChatMessageDelegate
 * @author kex1tu
 */

#include "chatmessagedelegate.h"
#include "chatmessagemodel.h"
#include "structures.h"
#include <QPainter>
#include <QSvgRenderer>
#include <QPainterPath>
#include <algorithm> // Для std::max

// Инициализируем статические члены класса.
QMap<ChatMessage::MessageStatus, QSvgRenderer*> ChatMessageDelegate::m_statusRenderers;
bool ChatMessageDelegate::m_renderersInitialized = false;

/**
 * @brief Конструктор делегата.
 * @param model Указатель на модель данных, необходим для получения информации о цитируемых сообщениях.
 * @param parent Родительский объект.
 */
ChatMessageDelegate::ChatMessageDelegate(const ChatMessageModel* model, QObject *parent)
    : QStyledItemDelegate(parent), m_model(model)
{
    // Безопасно инициализируем SVG-ресурсы один раз.
    initRenderers(this);
}

/**
 * @brief Статический метод для однократной инициализации SVG-рендереров.
 * @details Эта функция загружает SVG-иконки статусов в память и кэширует их
 *          в статической QMap. Это предотвращает дорогостоящую загрузку файлов
 *          с диска при отрисовке каждого сообщения.
 * @param parent Родительский объект для создаваемых QSvgRenderer,
 *               чтобы Qt автоматически управлял их памятью.
 */
void ChatMessageDelegate::initRenderers(QObject* parent)
{
    if (m_renderersInitialized) return; // Предотвращаем повторную инициализацию.

    qDebug() << "[Delegate] Инициализация SVG-рендереров...";

    // Загружаем каждую SVG-иконку и связываем ее с соответствующим статусом сообщения.
    m_statusRenderers[ChatMessage::Sending] = new QSvgRenderer(QString(":/icons/clock_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Sent] = new QSvgRenderer(QString(":/icons/message_send_icon.svg"), parent);
    m_statusRenderers[ChatMessage::Delivered] = new QSvgRenderer(QString(":/icons/message_read_icon.svg"), parent);

    m_renderersInitialized = true;
    qDebug() << "[Delegate] Рендереры созданы.";
}

/**
 * @brief Основной метод отрисовки одного элемента (сообщения) в списке.
 * @param painter QPainter, предоставляющий API для рисования.
 * @param option QStyleOptionViewItem, содержащий геометрию и состояние элемента.
 * @param index QModelIndex, предоставляющий доступ к данным сообщения в модели.
 */
void ChatMessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // --- 1. Подготовка ---
    // Сбрасываем стандартное выделение и наведение, чтобы управлять ими вручную (hover-эффект пузыря).
    QStyleOptionViewItem opt = option;
    opt.state &= ~(QStyle::State_Selected | QStyle::State_MouseOver | QStyle::State_HasFocus);
    QStyledItemDelegate::paint(painter, opt, index); // Рисуем фон и другие базовые вещи.

    painter->save(); // Сохраняем текущее состояние painter'а (трансформации, кисти, перья).
    painter->setRenderHint(QPainter::Antialiasing); // Включаем сглаживание для красивых фигур.


    // Получаем объект сообщения из модели.
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    QRect originalRect = option.rect;

    // Определяем базовые константы геометрии.
    int margin = 10;          // Внешний отступ пузыря от краев элемента.
    int padding = 10;         // Внутренний отступ от краев пузыря до контента.
    int borderRadius = 15;    // Радиус скругления углов пузыря.
    int verticalSpacing = 10; // Вертикальный отступ между сообщениями.
    QFontMetrics fm(painter->font());


    // Уменьшаем рабочую область, чтобы учесть вертикальный отступ.
    QRect contentRect = originalRect.adjusted(0, verticalSpacing / 2, 0, -verticalSpacing / 2);

    // --- 2. Отрисовка цитаты (ответа) ---
    ChatMessage repliedMsg;
    int quoteHeight = 0;
    bool foundReply = false;
    if (message.replyToId > 0) {
        // Запрашиваем данные цитируемого сообщения у модели по его ID.
        foundReply = m_model->getMessageById(message.replyToId, repliedMsg);
    }


    if (foundReply) {
        quoteHeight = fm.height() * 2 + 15; // Расчетная высота блока цитаты.

        // Рассчитываем геометрию блока цитаты.
        int nameWidth = fm.horizontalAdvance(repliedMsg.fromUser);
        QString elidedText = fm.elidedText(repliedMsg.payload, Qt::ElideRight, 250); // Обрезаем длинный текст.
        int textWidth = fm.horizontalAdvance(elidedText);
        int quoteContentWidth = std::max(nameWidth, textWidth);
        int quoteTotalWidth = quoteContentWidth + padding + 4 + 8; // Ширина = контент + отступы + полоса.

        QRectF quoteRect(0, 0, quoteTotalWidth, quoteHeight - 5);

        // Выравниваем блок цитаты по правому или левому краю.
        if (message.isOutgoing) {
            quoteRect.moveTopRight(contentRect.topRight() - QPoint(margin, 0));
        } else {
            quoteRect.moveTopLeft(contentRect.topLeft() + QPoint(margin, 0));
        }

        // Рисуем цветную вертикальную полосу слева от цитаты.
        QRectF colorBarRect = quoteRect.adjusted(padding, 4, 0, -4);
        colorBarRect.setWidth(4);
        painter->setBrush(QColor("#E072A4"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(colorBarRect, 2, 2);

        // Рисуем имя автора и текст цитаты.
        QRectF quoteTextRect = quoteRect.adjusted(colorBarRect.width() + padding + 4, 5, -padding, -5);
        painter->setPen(QColor("#E072A4"));
        painter->drawText(quoteTextRect, Qt::AlignTop | Qt::AlignLeft, repliedMsg.fromUser);
        painter->setPen(Qt::gray);
        QRectF repliedTextRect = quoteTextRect.adjusted(0, fm.height(), 0, 0);
        painter->drawText(repliedTextRect, Qt::AlignTop | Qt::AlignLeft, elidedText);

        // Сдвигаем область для отрисовки основного сообщения вниз.
        contentRect.setTop(contentRect.top() + quoteHeight);
    }


    // --- 3. Расчет геометрии основного пузыря сообщения ---
    // Ограничиваем максимальную ширину пузыря (75% от ширины виджета).
    int textWidth = contentRect.width() * 0.75 - 2 * padding;
    if (textWidth <= 0) textWidth = 300; // Минимальная ширина на случай ошибок.

    // Рассчитываем прямоугольник, необходимый для отрисовки текста с переносами.
    QRect payloadRect = fm.boundingRect(QRect(0, 0, textWidth, 0), Qt::TextWrapAnywhere, message.payload);
    int metaDataHeight = fm.height(); // Высота для строки с временем и статусом.

    // Формируем строку метаданных.
    QString metaText;
    if (message.isEdited) metaText += "(изм.) ";
    metaText += message.timestamp.mid(11, 5); // "HH:mm"
    int metaTextWidth = fm.horizontalAdvance(metaText);

    // Ширина пузыря - это максимум из ширины текста и ширины метаданных.
    int bubbleContentWidth = std::max(payloadRect.width(), metaTextWidth);
    int bubbleContentHeight = payloadRect.height() + metaDataHeight + 4;

    QRect bubbleRect(0, 0, bubbleContentWidth + 2 * padding, bubbleContentHeight + 2 * padding);

    // Устанавливаем минимальную ширину, чтобы короткие сообщения (типа "ок") не выглядели слишком узкими.
    int minBubbleWidth = 100;
    if (bubbleRect.width() < minBubbleWidth) bubbleRect.setWidth(minBubbleWidth);

    // Выравниваем пузырь по правому (исходящие) или левому (входящие) краю.
    if (message.isOutgoing) bubbleRect.moveTopRight(contentRect.topRight() - QPoint(margin, 0));
    else bubbleRect.moveTopLeft(contentRect.topLeft() + QPoint(margin, 0));

    // --- 4. Отрисовка основного пузыря ---
    QColor bubbleColor = message.isOutgoing ? QColor("#E072A4").darker(150) : QColor("#3D383A");
    painter->setBrush(bubbleColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);

    // Рисуем текст сообщения.
    QRect textDrawRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    textDrawRect.setHeight(payloadRect.height());
    painter->setPen(Qt::white);
    QTextOption textOption(Qt::AlignLeft | Qt::AlignVCenter);
    textOption.setWrapMode(QTextOption::WrapAnywhere); // Включаем перенос слов.
    painter->drawText(textDrawRect, message.payload, textOption);

    // --- 5. Отрисовка метаданных (время и статус) ---
    QRect baseMetaRect = bubbleRect.adjusted(padding, padding, -padding, -padding);
    baseMetaRect.setTop(textDrawRect.bottom() + 4);
    baseMetaRect.setHeight(metaDataHeight);

    if (message.isOutgoing) {
        // Для исходящих сообщений рисуем и текст, и иконку статуса.
        int iconSize = fm.height() - 2;
        int iconPadding = 3;

        // Рассчитываем место для иконки справа.
        QRect iconRect(baseMetaRect.right() - iconSize, baseMetaRect.top() + (baseMetaRect.height() - iconSize) / 2, iconSize, iconSize);
        // Оставшееся место слева от иконки - для текста.
        QRect textMetaRect = baseMetaRect;
        textMetaRect.setRight(iconRect.left() - iconPadding);

        // Рисуем текст (время, "изм.").
        QPen textPen = (message.status == ChatMessage::Read) ? QColor(70, 150, 255) : Qt::gray;
        painter->setPen(textPen);
        painter->drawText(textMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);

        // --- Продвинутая отрисовка и перекрашивание SVG иконки ---
        ChatMessage::MessageStatus statusToRender = message.status;
        if (statusToRender == ChatMessage::Read) statusToRender = ChatMessage::Delivered; // Для "прочитано" используем иконку "доставлено".

        QSvgRenderer* renderer = m_statusRenderers.value(statusToRender, nullptr);
        if (renderer && renderer->isValid()) {
            QPixmap pixmap(iconRect.size());
            pixmap.fill(Qt::transparent);

            // 1. Рендерим SVG на временный QPixmap.
            QPainter pixmapPainter(&pixmap);
            renderer->render(&pixmapPainter);
            pixmapPainter.end();

            // 2. "Заливаем" отрендеренное изображение нужным цветом.
            QPainter effectPainter(&pixmap);
            effectPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            QColor iconColor = (message.status == ChatMessage::Read) ? QColor(70, 150, 255) : Qt::gray;
            effectPainter.fillRect(pixmap.rect(), iconColor);
            effectPainter.end();

            // 3. Рисуем итоговый перекрашенный QPixmap на основной холст.
            painter->drawPixmap(iconRect, pixmap);
        }
    } else {
        // Для входящих сообщений просто рисуем текст справа.
        painter->setPen(Qt::gray);
        painter->drawText(baseMetaRect, Qt::AlignRight | Qt::AlignVCenter, metaText);


    }

    painter->restore(); // Восстанавливаем состояние painter'а.
}


/**
 * @brief Метод для расчета высоты элемента.
 * @details View вызывает этот метод, чтобы знать, сколько места выделить под сообщение.
 *          Расчет должен быть консистентным с логикой в `paint()`.
 * @param option Опции, включая доступную ширину.
 * @param index Индекс для доступа к данным сообщения.
 * @return QSize Требуемая высота и ширина.
 */
QSize ChatMessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ChatMessage message = index.data(Qt::UserRole).value<ChatMessage>();
    int padding = 10;
    int verticalSpacing = 10;
    QFontMetrics fm(option.font);

    // 1. Рассчитываем высоту блока цитаты, если он есть.
    int quoteHeight = 0;
    if (message.replyToId > 0) {
        ChatMessage repliedMsg;
        if (m_model->getMessageById(message.replyToId, repliedMsg)) {
            quoteHeight = fm.height() * 2 + 15;
        }
    }

    // 2. Рассчитываем высоту основного текста с учетом переносов.
    int textWidth = option.rect.width() * 0.75 - 2 * padding;
    if (textWidth <= 0) textWidth = 300;
    QRect payloadRect = fm.boundingRect(QRect(0, 0, textWidth, 0), Qt::TextWrapAnywhere, message.payload);

    // 3. Высота строки метаданных.
    int metaDataHeight = fm.height();

    // 4. Суммарная высота пузыря.
    int bubbleHeight = payloadRect.height() + metaDataHeight + 4 + (2 * padding);

    // 5. Итоговая высота всего элемента.
    int totalHeight = quoteHeight + bubbleHeight + verticalSpacing;

    return QSize(option.rect.width(), totalHeight);
}

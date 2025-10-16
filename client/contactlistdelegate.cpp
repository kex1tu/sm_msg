/**
 * @file contactlistdelegate.cpp
 * @brief Реализация кастомного делегата для отрисовки элементов в списке контактов.
 * @see ContactListDelegate
 * @author kex1tu
 */

#include "contactlistdelegate.h"
#include "contactlistmodel.h"
#include <QPainter>
#include <QApplication> // Для доступа к глобальным настройкам, например, шрифту.

/**
 * @brief Конструктор.
 * @details Делегат полностью отвязан от MainWindow и не хранит собственного состояния,
 *          поэтому конструктор максимально прост.
 * @param parent Родительский объект.
 */
ContactListDelegate::ContactListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

/**
 * @brief Основной метод отрисовки одного элемента (контакта) в списке.
 * @details Этот метод вызывается QListView для каждого видимого элемента. Он запрашивает
 *          все необходимые данные у модели через кастомные роли и использует QPainter
 *          для создания сложного двухстрочного представления.
 * @param painter "Холст" (QPainter) для рисования.
 * @param option Содержит геометрию и состояние элемента (выделен, наведен и т.д.).
 * @param index Индекс элемента в ContactListModel, предоставляющий доступ к данным.
 */
void ContactListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save(); // Сохраняем текущее состояние painter'а (трансформации, кисти, перья).

    // --- 1. Отрисовка фона ---
    // Вручную рисуем фон для состояний "выделен" и "наведен", чтобы иметь полный контроль.
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight()); // Цвет выделения из палитры.
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.color(QPalette::AlternateBase)); // Альтернативный цвет фона.
    }

    // --- 2. Получение данных из модели ---
    // Запрашиваем всю необходимую информацию у модели по кастомным ролям.
    QString displayName = index.data(Qt::DisplayRole).toString();
    QString lastMessage = index.data(ContactListModel::LastMessageRole).toString();
    bool isOnline = index.data(ContactListModel::IsOnlineRole).toBool();
    bool isTyping = index.data(ContactListModel::IsTypingRole).toBool();
    int unreadCount = index.data(ContactListModel::UnreadCountRole).toInt();

    // --- 3. Подготовка текста для нижней строки ---
    QString bottomText = lastMessage;
    QColor bottomTextColor = Qt::gray;

    // Статус "печатает..." имеет наивысший приоритет.
    if (isTyping) {
        bottomText = "печатает...";
        bottomTextColor = QColor("#F4ABC4"); // Акцентный цвет.
    }

    // --- 4. Расчет геометрии для текста ---
    int padding = 12; // Боковые отступы.
    QRect contentRect = option.rect.adjusted(padding, padding / 2, -padding, -padding / 2);
    // Делим доступное пространство на два прямоугольника: для верхней и нижней строки.
    QRect topRect(contentRect.left(), contentRect.top(), contentRect.width(), contentRect.height() / 2);
    QRect bottomRect(contentRect.left(), contentRect.top() + contentRect.height() / 2, contentRect.width(), contentRect.height() / 2);

    // --- 5. Отрисовка верхней строки (Имя пользователя) ---
    QFont nameFont = painter->font();
    nameFont.setBold(isOnline); // Жирный шрифт для онлайн-пользователей.
    painter->setFont(nameFont);
    painter->setPen(isOnline ? Qt::white : QColor("#EAEAEA"));
    painter->drawText(topRect, Qt::AlignLeft | Qt::AlignBottom, displayName);

    // --- 6. Отрисовка нижней строки (Превью сообщения или статус) ---
    QFontMetrics fm(painter->font());
    // Обрезаем длинный текст и добавляем многоточие.
    QString elidedBottomText = fm.elidedText(bottomText, Qt::ElideRight, bottomRect.width());
    painter->setFont(QApplication::font()); // Возвращаем стандартный шрифт.
    painter->setPen(bottomTextColor);
    painter->drawText(bottomRect, Qt::AlignLeft | Qt::AlignTop, elidedBottomText);

    // --- 7. Отрисовка "бейджа" с количеством непрочитанных сообщений ---
    if (unreadCount > 0) {
        painter->setRenderHint(QPainter::Antialiasing); // Включаем сглаживание для круга.

        int badgeSize = 20; // Диаметр кружка.
        QColor badgeColor = option.palette.highlight().color(); // Используем цвет выделения для фона.
        QColor textColor = option.palette.highlightedText().color(); // Цвет текста на фоне выделения.

        // Рассчитываем положение "бейджа" в правом углу элемента.
        QRect badgeRect(
            option.rect.right() - badgeSize - padding,
            option.rect.top() + (option.rect.height() - badgeSize) / 2,
            badgeSize, badgeSize
            );

        // Рисуем фон.
        painter->setBrush(badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);

        // Рисуем текст (цифру).
        painter->setPen(textColor);
        painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(unreadCount));
    }

    painter->restore(); // Восстанавливаем состояние painter'а.
}

/**
 * @brief Метод для расчета размера элемента списка.
 * @details QListView вызывает этот метод, чтобы определить, сколько места выделить
 *          для каждого контакта.
 * @param option Опции представления.
 * @param index Индекс элемента.
 * @return QSize Рекомендуемый размер.
 */
QSize ContactListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option); // Макрос, чтобы компилятор не ругался на неиспользуемые параметры.
    Q_UNUSED(index);

    // Рассчитываем высоту, достаточную для двух строк текста + вертикальные отступы.
    QFontMetrics fm(QApplication::font());
    int singleLineHeight = fm.height();
    return QSize(200, singleLineHeight * 2 + 16); // 2 строки + 16px на отступы.
}

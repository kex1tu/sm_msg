/**
 * @file smoothlistview.cpp
 * @brief Реализация кастомного QListView с плавной попиксельной прокруткой.
 * @see SmoothListView
 * @author kex1tu
 */

#include "smoothlistview.h"
#include <QWheelEvent>  // Для обработки событий колеса мыши.
#include <QResizeEvent> // Для обработки событий изменения размера.
#include <QEvent>       // Для обработки общих событий.

/**
 * @brief Конструктор SmoothListView.
 * @details Здесь задаются ключевые свойства, которые отличают этот виджет
 *          от стандартного QListView и обеспечивают плавность и производительность.
 * @param parent Родительский виджет.
 */
SmoothListView::SmoothListView(QWidget *parent)
    : QListView(parent)
{
    // 1. Устанавливаем режим попиксельной прокрутки.
    //    Вместо стандартного режима "по элементам" (ScrollPerItem), этот режим
    //    позволяет плавно сдвигать содержимое на любое количество пикселей.
    //    Это основа для плавной прокрутки.
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 2. Отключаем горизонтальный скроллбар, так как наш контент (сообщения)
    //    должен всегда подстраиваться под ширину виджета.
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 3. Устанавливаем режим пакетной компоновки (Batched layout).
    //    Это оптимизация производительности. Вместо того чтобы пересчитывать
    //    геометрию всех элементов при каждом небольшом изменении, QListView
    //    будет делать это "пачками" по `batchSize` элементов, что снижает
    //    нагрузку при работе с очень большими списками.
    setLayoutMode(QListView::Batched);
    setBatchSize(100); // Размер "пачки".
}

/**
 * @brief Переопределенный обработчик событий колеса мыши для замедления прокрутки.
 * @param e Указатель на событие колеса мыши.
 */
void SmoothListView::wheelEvent(QWheelEvent *e)
{
    // Стандартная прокрутка в режиме ScrollPerPixel может быть слишком быстрой.
    // Мы перехватываем событие, чтобы уменьшить его интенсивность.

    // 1. Получаем "дельта-угол" — насколько было повернуто колесо.
    QPoint angleDelta = e->angleDelta();

    // 2. Определяем коэффициент замедления. 0.5 означает, что прокрутка
    //    будет в два раза медленнее стандартной.
    double scrollFactor = 0.5;

    // 3. Создаем НОВОЕ событие QWheelEvent, копируя все поля из старого,
    //    но умножая `angleDelta` на наш коэффициент.
    QWheelEvent newEvent(
        e->position(),
        e->globalPosition(),
        e->pixelDelta(),
        QPoint(angleDelta.x() * scrollFactor, angleDelta.y() * scrollFactor),
        e->buttons(),
        e->modifiers(),
        e->phase(),
        e->inverted()
        );

    // 4. Передаем наше измененное, "ослабленное" событие в стандартный
    //    обработчик базового класса QListView.
    QListView::wheelEvent(&newEvent);
}

/**
 * @brief Переопределенный обработчик событий изменения размера виджета.
 * @param e Указатель на событие изменения размера.
 */
void SmoothListView::resizeEvent(QResizeEvent *e)
{
    // 1. Сначала вызываем стандартный обработчик.
    QListView::resizeEvent(e);

    // 2. Затем принудительно вызываем `doItemsLayout()`.
    //    Это необходимо, потому что в режиме пакетной компоновки и попиксельной
    //    прокрутки QListView может не сразу обновить расположение элементов
    //    при изменении ширины, что приводит к визуальным артефактам.
    //    Этот вызов заставляет его немедленно пересчитать геометрию всех видимых элементов.
    doItemsLayout();
}

/**
 * @brief Переопределенный обработчик событий для области просмотра (viewport).
 * @details Мы перехватываем события, связанные с наведением мыши, чтобы
 *          гарантировать, что они будут обработаны. Это позволяет нашему
 *          делегату (`ChatMessageDelegate`) получать информацию о состоянии
 *          `State_MouseOver` и корректно отрисовывать hover-эффекты для элементов.
 * @param event Указатель на событие.
 * @return `true` если событие было обработано, иначе `false`.
 */
bool SmoothListView::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    // Если мышь вошла в область виджета, двигается над ним или покинула его...
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    case QEvent::HoverLeave:
        // ... мы "принимаем" событие. Это гарантирует, что Qt продолжит
        // отслеживать положение мыши и отправлять события наведения,
        // которые нужны для hover-эффектов в делегате.
        event->accept();
        return true;
    default:
        break;
    }

    // Для всех остальных типов событий вызываем стандартный обработчик.
    return QListView::viewportEvent(event);
}

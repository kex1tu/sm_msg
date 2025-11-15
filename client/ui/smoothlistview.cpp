#include "smoothlistview.h"
#include <QWheelEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QScrollBar>
#include <QEasingCurve>

SmoothListView::SmoothListView(QWidget *parent)
    : QListView(parent)
{
    // По пикселям — плавная прокрутка, а не по строкам
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Скрывать горизонтальный скроллбар — всегда
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Для оптимизации рендеринга/производительности — SinglePass, 100 элементов за проход
    setLayoutMode(QListView::SinglePass);
    setBatchSize(100);

    // Анимация прокрутки (используется при wheelEvent)
    m_scrollAnimation = new QPropertyAnimation(this);
    m_scrollAnimation->setTargetObject(verticalScrollBar());
    m_scrollAnimation->setPropertyName("value");
    m_scrollAnimation->setDuration(200);               // 200мс

    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // Никакого выделения элементов
    setSelectionMode(QAbstractItemView::NoSelection);

    // Запрет на перетаскивания
    setDragDropMode(QAbstractItemView::NoDragDrop);
}


void SmoothListView::wheelEvent(QWheelEvent *e)
{
    // Останавливаем любую прошлую анимацию скролла (если работает)
    m_scrollAnimation->stop();

    // Используем вертикальный скроллбар
    QScrollBar* bar = verticalScrollBar();
    int currentValue = bar->value();

    // delta — сколько прокрутить. Умножаем на 2 для быстрого пролистывания
    int delta = - (e->angleDelta().y() * 2);

    // Вычисляем куда скроллить — не выходим за границы
    int targetValue = currentValue + delta;
    if (targetValue < bar->minimum()) {
        targetValue = bar->minimum();
    }
    if (targetValue > bar->maximum()) {
        targetValue = bar->maximum();
    }

    // Запускаем анимацию плавного скроллинга
    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(targetValue);
    m_scrollAnimation->start();

    // Приняли событие (не отдаём наверх родителю)
    e->accept();
}


void SmoothListView::resizeEvent(QResizeEvent *e)
{
    // Стандартное поведение при ресайзе (QListView)
    QListView::resizeEvent(e);

    // При изменении размера нужно обновить layout элементов (особенно для custom sizeHint)
    doItemsLayout();
    scheduleDelayedItemsLayout();
}


void SmoothListView::stopScrollAnimation()
{
    // Останавливаем анимацию скролла, если она идёт прямо сейчас
    if (m_scrollAnimation->state() == QAbstractAnimation::Running) {
        m_scrollAnimation->stop();
    }
}


bool SmoothListView::viewportEvent(QEvent *event)
{
    // Обработка событий наведения мыши (hover)
    switch (event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    case QEvent::HoverLeave:
        // Не даём родителю обрабатывать, чтобы не было лишних redraw и event bubbling
        event->accept();
        return true;
    default:
        break;
    }
    // Всё остальное — стандартная обработка viewport'а
    return QListView::viewportEvent(event);
}

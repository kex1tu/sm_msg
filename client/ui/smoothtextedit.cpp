#include "smoothtextedit.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QEasingCurve>


SmoothTextEdit::SmoothTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    // Создаём анимацию для вертикального скроллбара
    m_scrollAnimation = new QPropertyAnimation(this);

    // Устанавливаем объект анимации (verticalScrollBar)
    m_scrollAnimation->setTargetObject(verticalScrollBar());

    // Свойство, которое хотим анимировать — value scrollBar'а
    m_scrollAnimation->setPropertyName("value");

    // Длительность анимации — 200ms
    m_scrollAnimation->setDuration(200);

    // Кривая анимации — OutCubic (быстрое начало, плавный конец)
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);
}


void SmoothTextEdit::wheelEvent(QWheelEvent *e)
{
    // Останавливаем старую анимацию, если она ещё в процессе
    m_scrollAnimation->stop();

    // Текущая позиция скролла
    int currentValue = verticalScrollBar()->value();

    // delta — насколько прокрутить. Минус, чтоб колесо вверх было scroll-down.
    int delta = e->angleDelta().y() * -0.4;

    // Устанавливаем начальное и конечное значение для анимации
    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(currentValue + delta);

    // Запускаем анимацию движения scrollBar'а
    m_scrollAnimation->start();

    // Помечаем событие как обработанное
    e->accept();
}

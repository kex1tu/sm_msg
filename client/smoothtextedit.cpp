/**
 * @file smoothtextedit.cpp
 * @brief Реализация кастомного QTextEdit с плавной анимацией прокрутки.
 * @see SmoothTextEdit
 * @author kex1tu
 */

#include "smoothtextedit.h"
#include <QWheelEvent>      // Для обработки событий колеса мыши.
#include <QScrollBar>       // Для доступа к вертикальному скроллбару.
#include <QEasingCurve>     // Для задания кривой анимации (плавное замедление).

/**
 * @brief Конструктор SmoothTextEdit.
 * @details Здесь создается и настраивается объект `QPropertyAnimation`, который
 *          будет отвечать за всю "магию" плавной прокрутки.
 * @param parent Родительский виджет.
 */
SmoothTextEdit::SmoothTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    // 1. Создаем экземпляр анимации.
    m_scrollAnimation = new QPropertyAnimation(this);

    // 2. Устанавливаем цель анимации. Мы будем анимировать не сам QTextEdit,
    //    а его вертикальный скроллбар.
    m_scrollAnimation->setTargetObject(verticalScrollBar());

    // 3. Указываем, какое свойство цели мы будем анимировать.
    //    У QScrollBar есть свойство "value", которое хранит его текущую позицию.
    m_scrollAnimation->setPropertyName("value");

    // 4. Задаем длительность анимации в миллисекундах.
    m_scrollAnimation->setDuration(200);

    // 5. Устанавливаем "кривую замедления" (easing curve).
    //    OutCubic означает, что анимация начнется быстро и плавно замедлится к концу,
    //    что создает приятный и естественный эффект.
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

/**
 * @brief Переопределенный обработчик событий колеса мыши.
 * @details Этот метод является ядром функциональности. Он перехватывает
 *          стандартное событие прокрутки и вместо мгновенного перемещения
 *          скроллбара запускает плавную анимацию.
 * @param e Указатель на событие колеса мыши.
 */
void SmoothTextEdit::wheelEvent(QWheelEvent *e)
{
    // 1. Если анимация уже проигрывается, останавливаем ее.
    //    Это важно, если пользователь быстро крутит колесо, чтобы
    //    новая анимация начиналась с текущей фактической позиции.
    m_scrollAnimation->stop();

    // 2. Получаем текущую позицию скроллбара.
    int currentValue = verticalScrollBar()->value();

    // 3. Вычисляем, на сколько пикселей нужно прокрутить.
    //    - e->angleDelta().y() возвращает "шаги" колеса (обычно кратно 120).
    //    - Знак "-" инвертирует направление (стандартно для Qt).
    //    - Коэффициент 0.4 уменьшает скорость/дистанцию прокрутки.
    int delta = e->angleDelta().y() * -0.4;

    // 4. Настраиваем анимацию: откуда и куда двигаться.
    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(currentValue + delta);

    // 5. Запускаем анимацию.
    m_scrollAnimation->start();

    // 6. "Принимаем" событие, сообщая Qt, что мы его обработали.
    //    Это предотвращает передачу события родительским виджетам.
    e->accept();
}

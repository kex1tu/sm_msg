#ifndef SMOOTHTEXTEDIT_H
#define SMOOTHTEXTEDIT_H

#include <QTextEdit>          // Включаем базовый класс QTextEdit.
#include <QPropertyAnimation> // Включаем класс для анимации свойств объектов Qt.

/**
 * @class SmoothTextEdit
 * @brief Кастомный QTextEdit с плавной анимацией прокрутки.
 *
 * @details Этот класс наследуется от QTextEdit и переопределяет стандартное
 * поведение прокрутки колесом мыши. Вместо мгновенного "прыжка" скроллбара,
 * он использует QPropertyAnimation для создания плавной и анимированной
 * прокрутки содержимого, если текст не помещается в видимую область.
 * @author kex1tu
 */
class SmoothTextEdit : public QTextEdit
{
    Q_OBJECT // Обязательный макрос для классов Qt.

public:
    /**
     * @brief Конструктор.
     * @param parent Родительский виджет.
     */
    explicit SmoothTextEdit(QWidget *parent = nullptr);

protected:
    /**
     * @brief Переопределенный обработчик событий колеса мыши.
     *
     * @details Этот метод перехватывает событие прокрутки. Вместо того чтобы
     * передавать его в базовый класс для стандартной обработки, он
     * вычисляет новую целевую позицию скроллбара и запускает
     * QPropertyAnimation для плавного перемещения к этой позиции.
     *
     * @param e Указатель на событие колеса мыши.
     */
    void wheelEvent(QWheelEvent *e) override;

private:
    /**
     * @brief Указатель на объект анимации.
     * @details Этот объект управляет анимацией свойства "value" (текущая позиция)
     *          вертикального скроллбара виджета. Он создается один раз
     *          в конструкторе и используется повторно при каждом событии прокрутки.
     */
    QPropertyAnimation *m_scrollAnimation;
};

#endif // SMOOTHTEXTEDIT_H

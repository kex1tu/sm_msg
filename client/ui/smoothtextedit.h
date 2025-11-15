#ifndef SMOOTHTEXTEDIT_H
#define SMOOTHTEXTEDIT_H

#include <QTextEdit>           
#include <QPropertyAnimation>  

 
class SmoothTextEdit : public QTextEdit
{
    Q_OBJECT  

public:
    /**
 * @brief Конструктор SmoothTextEdit — QTextEdit с плавной анимацией прокрутки колёсиком мыши.
 * @param parent Родительский виджет
 */
    explicit SmoothTextEdit(QWidget *parent = nullptr);

protected:
    /**
 * @brief wheelEvent — плавная прокрутка с анимацией при взаимодействии мышиным колесом.
 * @param e Событие колеса мыши
 */

    void wheelEvent(QWheelEvent *e) override;

private:

    /**
 * @brief Анимация плавной прокрутки списка (wheelEvent/scroll).
 * Используется для smooth-scroll при взаимодействии с мышью.
 */
    QPropertyAnimation *m_scrollAnimation;
};

#endif  

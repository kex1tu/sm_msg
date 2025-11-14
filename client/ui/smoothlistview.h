#ifndef SMOOTHLISTVIEW_H
#define SMOOTHLISTVIEW_H

#include <QListView>  
#include <QPropertyAnimation>
 
class SmoothListView : public QListView
{
    Q_OBJECT  
public slots:
    /**
 * @brief Принудительно останавливает текущую анимацию скролла (если идёт).
 */
    void stopScrollAnimation();

public:
    /**
 * @brief Конструктор виджета плавного QListView с анимацией скролла.
 * @param parent Родительский виджет
 */
    explicit SmoothListView(QWidget *parent = nullptr);

protected:
    /**
 * @brief Обрабатывает колесо мыши: реализует анимацию плавного скролла.
 * @param e Событие колеса мыши
 */
    void wheelEvent(QWheelEvent *e) override;
    /**
 * @brief Реакция на изменение размера — обновляет layout для корректного отображения кастомных элементов.
 * @param e Событие изменения размера
 */
    void resizeEvent(QResizeEvent *e) override;
    /**
 * @brief Обработка событий наведения для отключения bubbling и лишних обновлений.
 * @param event QEvent*
 * @return true если обработано, иначе стандартная обработка
 */
    bool viewportEvent(QEvent *event) override;
private:
    /**
 * @brief Анимация плавной прокрутки списка (wheelEvent/scroll).
 * Используется для smooth-scroll при взаимодействии с мышью.
 */
    QPropertyAnimation* m_scrollAnimation;
};

#endif  

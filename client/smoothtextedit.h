#ifndef SMOOTHTEXTEDIT_H
#define SMOOTHTEXTEDIT_H

#include <QTextEdit>
#include <QPropertyAnimation>  

class SmoothTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit SmoothTextEdit(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *e) override;

private:
    QPropertyAnimation *m_scrollAnimation;
};

#endif  

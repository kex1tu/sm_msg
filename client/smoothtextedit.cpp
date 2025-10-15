#include "smoothtextedit.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QEasingCurve>

SmoothTextEdit::SmoothTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
     
    m_scrollAnimation = new QPropertyAnimation(this);
    m_scrollAnimation->setTargetObject(verticalScrollBar());  
    m_scrollAnimation->setPropertyName("value");              
    m_scrollAnimation->setDuration(200);                      
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);  
}

void SmoothTextEdit::wheelEvent(QWheelEvent *e)
{
     
    m_scrollAnimation->stop();

     
    int currentValue = verticalScrollBar()->value();

     
     
    int delta = e->angleDelta().y() * -0.4;

     
    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(currentValue + delta);

     
    m_scrollAnimation->start();

     
    e->accept();
}

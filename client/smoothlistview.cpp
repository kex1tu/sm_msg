 
#include "smoothlistview.h"
#include <QWheelEvent>
#include <QResizeEvent>
#include <QEvent>

SmoothListView::SmoothListView(QWidget *parent)
    : QListView(parent)
{
     
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setLayoutMode(QListView::Batched);
    setBatchSize(100);
}

void SmoothListView::wheelEvent(QWheelEvent *e)
{
     

     
    QPoint angleDelta = e->angleDelta();

     
     
    double scrollFactor = 0.5;

     
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

     
    QListView::wheelEvent(&newEvent);
}

void SmoothListView::resizeEvent(QResizeEvent *e)
{
     
    QListView::resizeEvent(e);

     
     
     
    doItemsLayout();
}
bool SmoothListView::viewportEvent(QEvent *event)
{
     
    switch (event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    case QEvent::HoverLeave:
         
         
         
        event->accept();
        return true;
    default:
        break;  
    }

     
     
    return QListView::viewportEvent(event);
}

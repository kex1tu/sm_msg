 
#ifndef SMOOTHLISTVIEW_H
#define SMOOTHLISTVIEW_H

#include <QListView>

class SmoothListView : public QListView
{
    Q_OBJECT
public:
    explicit SmoothListView(QWidget *parent = nullptr);

protected:
     
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool viewportEvent(QEvent *event) override;
};

#endif  

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "videoWidget.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void resizeEvent(QResizeEvent *event);

private:
    VideoThread *pVideoThread;
    VideoWidget *pVideoWidget;

};
#endif // WIDGET_H

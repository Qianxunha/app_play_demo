#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumSize(QSize(1280, 720));

    /*
     * VideoThread类
     * true: GPU 硬解码
     * false: CPU 软解码
     */
    pVideoThread = new VideoThread(true);
    pVideoWidget = new VideoWidget(1280, 720, pVideoThread, this);
    this->setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");

    connect(pVideoWidget,   &VideoWidget::emitFullScreen,   this,   &Widget::setFullScreen);
}

Widget::~Widget()
{
}

void Widget::resizeEvent(QResizeEvent *event)
{
    qDebug()<<this->width()<<this->height();
    pVideoWidget->setGeometry(0, 0, this->width(), this->height());
}


void Widget::setFullScreen()
{
    if(this->isFullScreen())
    {
        this->showNormal();
    }
    else
    {
        this->showFullScreen();
    }
    pVideoWidget->setGeometry(0, 0, this->width(), this->height());
}

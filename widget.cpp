#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumSize(QSize(1280, 720));

    pVideoThread = new VideoThread(true);
    pVideoWidget = new VideoWidget(1280, 720, pVideoThread, this);
    this->setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
}

Widget::~Widget()
{
}

void Widget::resizeEvent(QResizeEvent *event)
{
    pVideoWidget->setGeometry(0, 0, this->width(), this->height());
}


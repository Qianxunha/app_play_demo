#include "appconfig.h"


/*
 * function: 构造函数
 */
HwDecodeVideo::HwDecodeVideo()
{
    hwList.clear();
}

HwDecodeVideo::~HwDecodeVideo()
{

}


/*
 * function:寻找电脑支持哪些解码器
 */
void HwDecodeVideo::getHWdecode(QStringList &hwList)
{
    int i = 0;
    QString hwString;
    enum AVHWDeviceType type;
    type = av_hwdevice_find_type_by_name("");
    while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
    {
        hwString = QString("%1").arg(av_hwdevice_get_type_name(type));
        hwList.insert(i, hwString);
        i++;
    }
}

APPCustom::APPCustom()
{

}
APPCustom::~APPCustom()
{

}

void APPCustom::customQMessage(QString message)
{
    QMessageBox mess(QMessageBox::Information,tr("提示"),message);
    QPushButton *okbutton = (mess.addButton(tr("确定"),QMessageBox::AcceptRole));
    mess.setWindowIcon(QIcon(APP_ICON_PATH));
#if OS_WINDOWS_QT
    mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
#elif OS_MAC_QT
    mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
#endif
    mess.exec();
}

/*
 * function:获取当前时间(年份+月+日+当天时间)
 */
void APPCustom::getCurrentTime(QString &str)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int year  = currentTime.date().year();  // 年
    int month = currentTime.date().month(); // 月
    int day   = currentTime.date().day();   // 日
    int hour  = currentTime.time().hour();  // 时
    int minute= currentTime.time().minute(); // 分
    int sec   = currentTime.time().second(); // 秒

    QString strMonth, strDay, strHour, strMintue, strSec;

    if(month>0 && month<10)
        strMonth = QString("0%1").arg(month);
    else
        strMonth = QString("%1").arg(month);

    if(day >0 && day<10)
        strDay = QString("0%1").arg(day);
    else
        strDay = QString("%1").arg(day);

    if(hour >0 && hour<10)
        strHour = QString("0%1").arg(hour);
    else
        strHour = QString("%1").arg(hour);

    if(minute >0 && minute<10)
        strMintue = QString("0%1").arg(minute);
    else
        strMintue = QString("%1").arg(minute);

    if(sec >0 && sec<10)
        strSec = QString("0%1").arg(sec);
    else
        strSec = QString("%1").arg(sec);


    str = QString("%1%2%3%4%5%6").arg(year).arg(strMonth).arg(strDay).arg(strHour).arg(strMintue).arg(strSec);
}

CustomPushButton::CustomPushButton(QString filename,bool isCheckable)
{
    image = QIcon(filename + ".png");
    hoverImage = QIcon(filename + "_hover.png");
    pressedImage = QIcon(filename + "_pressed.png");
    setIcon(image);
    this->setCheckable(isCheckable);
    if (this->isCheckable())
    {
        checked_image = QIcon(filename + "_checked.png");
        checked_hoverImage = QIcon(filename + "_checked_hover.png");
        checked_pressedImage = QIcon(filename + "_checked_pressed.png");
    }
}

void CustomPushButton::setUpImage(QString filename)
{
    image = QIcon(filename + ".png");
    hoverImage = QIcon(filename + "_hover.png");
    pressedImage = QIcon(filename + "_pressed.png");
    setIcon(image);
}

void CustomPushButton::setSwitchIcon()
{
    setChecked(true);
    setIcon(checked_image);
}

void CustomPushButton::setStopIcon()
{
    setChecked(false);
    setIcon(image);
}

void CustomPushButton::setmaxIcon()
{
    setChecked(false);
    setIcon(image);
}


bool CustomPushButton::event(QEvent* e)
{
    if(!this->isCheckable())
    {
        switch (e->type())
        {
        case QEvent::Enter:
                setIcon(hoverImage);
            break;
        case QEvent::Leave:
                setIcon(image);
            break;
        case QEvent::MouseButtonPress:
                setIcon(pressedImage);
            break;
        case QEvent::MouseButtonRelease:
                setIcon(hoverImage);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (e->type())
        {
        case QEvent::Enter:
            if (this->isChecked())
                setIcon(checked_hoverImage);
            else
                setIcon(hoverImage);
            break;
        case QEvent::Leave:
            if (this->isChecked())
                setIcon(checked_image);
            else
                setIcon(image);
            break;
        case QEvent::MouseButtonPress:
            if (this->isChecked())
                setIcon(checked_pressedImage);
            else
                setIcon(pressedImage);
            break;
        case QEvent::MouseButtonRelease:
            if (this->isChecked())
                setIcon(hoverImage);
            else
            {
                if(isPlayResourceAvailable)
                    setIcon(checked_hoverImage);
                else
                {
                    setIcon(hoverImage);
                    isPlayResourceAvailable = true;
                }
            }
            break;
        case QEvent::MouseButtonDblClick:
            if (this->isChecked())
                setIcon(checked_image);
            else
                setIcon(image);
            break;
        default:
            break;
        }
    }
    return QPushButton::event(e);
}

CustomQSlider::CustomQSlider(QWidget*parent):QSlider(parent)
{
    m_value=0;
    m_mouseMove  = false;
    m_mousePress = false;
    setOrientation(Qt::Orientation::Horizontal);

    setStyleSheet("QSlider{background-color:rgb(0,0,0)}"\
                               "QSlider::add-page:horizontal{   background-color: rgb(87, 97, 106);  height:4px; }"\
                               "QSlider::sub-page:horizontal {background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "\
                               "stop:0 rgba(231,80,229, 255), stop:1 rgba(7,208,255, 255));"\
                               "height:4px;}"\

                               "QSlider::add-page:hover:horizontal{   background-color: rgb(87, 97, 106);  height:6px; }"\
                               "QSlider::sub-page:hover:horizontal {background-color:qlineargradient(spread:pad, x1:0, y1:0, "\
                               "x2:1, y2:0, stop:0 rgba(231,80,229, 255), stop:1 rgba(7,208,255, 255));"
                               "height:6px;}"\

                               "QSlider::handle:horizontal {margin-top:-2px; margin-bottom:-2px;"\
                               "border-radius:4px;  background: rgb(222,222,222); width:18px; height:18px;}"\

                               "QSlider::groove:horizontal {background:transparent;height:4px}"\
                               "QSlider::groove:hover:horizontal {background:transparent;height:6px}"\
                               "QSlider{background-color: rgba(255, 0, 0, 0);}");
}

CustomQSlider::~CustomQSlider()
{

}

void CustomQSlider::mousePressEvent(QMouseEvent *event)
{
    //    this.x:控件原点到界面边缘的x轴距离；
    //    globalPos.x：鼠标点击位置到屏幕边缘的x轴距离；
    //    pos.x：鼠标点击位置到本控件边缘的距离；
    //    this.width:本控件的宽度;
    //注意应先调用父类的鼠标点击处理事件，这样可以不影响拖动的情况
    QSlider::mousePressEvent(event);
    m_mouseMove  = false;
    m_mousePress = true;
    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
    double pos = event->pos().x() / (double)width();
    double value = pos * (maximum() - minimum()) + minimum();
    //value + 0.5 四舍五入
    if(value>maximum()){
        value=maximum();
    }
    if(value<minimum()){
        value=minimum();
    }
    m_value=value+0.5;

    setValue(m_value);
    //emit sliderMoved( m_value );
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    //QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));

    //QCoreApplication::sendEvent(parentWidget(), &evEvent);


}

void CustomQSlider::mouseMoveEvent(QMouseEvent *event)
{
    QSlider::mouseMoveEvent(event);

    double pos = event->pos().x() / (double)width();
    double value = pos * (maximum() - minimum()) + minimum();
    if(value>maximum()){
        value=maximum();
    }
    if(value<minimum()){
        value=minimum();
    }
    //value + 0.5 四舍五入
    if(m_mousePress){
        m_value = value + 0.5;
        m_mouseMove=true;

    }
    setValue(value + 0.5);


    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    //QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    //QCoreApplication::sendEvent(parentWidget(), &evEvent);
}

void CustomQSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);

    m_mousePress = false;
    m_mouseMove  = false;

    //抛出有用信号
    emit emitSliderValue(m_value);
}


customQProxyStyle::customQProxyStyle()
{

}

customQProxyStyle::~customQProxyStyle()
{

}

void customQProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget)const
{
    if(PE_FrameFocusRect == element)
    {

    }
    else
    {
        QProxyStyle::drawPrimitive(element,option, painter, widget);
    }
}


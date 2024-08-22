#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QIcon>
#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTextCodec>
#include <QDateTime>
#include <QPushButton>
#include <QSlider>
#include <QProxyStyle>
#include <QScrollBar>
#include <QTimer>
#include <QException>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>

#include <QFileDialog>

extern "C" {

#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"

#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"

#include "libavformat/avio.h"
#include "libavformat/version.h"
#include "libavformat/avformat.h"

#include "libavutil/mem.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_qsv.h"

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#include <iostream>
using namespace std;

#ifdef _QT_APP_PLATFORM
#define  OS_WINDOWS_QT         1            // windows
#define  OS_MAC_QT             0            // mac

#pragma execution_character_set("utf-8")

#else
#define  OS_WINDOWS_QT         0            // windows
#define  OS_MAC_QT             1            // mac
#endif

//应用程序图标
#define  APP_ICON_PATH

typedef struct DecodeContext
{
    AVBufferRef *hw_device_ref;
}DecodeContext;

//time=20200916
class  HwDecodeVideo
{
public:
    HwDecodeVideo();
    ~HwDecodeVideo();

    void getHWdecode(QStringList &hwList);                          //ffmpeg 寻找硬解码器

    //存储电脑支持硬解码器名字
    QStringList hwList;

};

class APPCustom :public QObject
{
Q_OBJECT
public:
    APPCustom();
    ~APPCustom();

public:
    static void customQMessage(QString message);
    static void getCurrentTime(QString &str);
};

class CustomPushButton : public QPushButton
{
    Q_OBJECT
public:
    bool isPlayResourceAvailable = true;

private:
    QIcon image;
    QIcon hoverImage;
    QIcon pressedImage;
    QIcon checked_image;
    QIcon checked_hoverImage;
    QIcon checked_pressedImage;

public:
    CustomPushButton(QString filename, bool isCheckable);

public:
    void setUpImage(QString filename);

    //专为play按钮使用
    void setSwitchIcon();
    void setStopIcon();

    //专为 标题栏max按钮使用
    void setmaxIcon();

public:
    bool event(QEvent* e)Q_DECL_OVERRIDE;
};

class CustomQSlider :public QSlider
{
    Q_OBJECT
public:
    CustomQSlider(QWidget *parent=nullptr);
    ~CustomQSlider();

signals:
    void emitSliderValue(int );

protected:
    void mousePressEvent(QMouseEvent *event)override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event)override;

    int  m_value;
    bool m_mouseMove;
    bool m_mousePress;
};

class customQProxyStyle: public QProxyStyle
{
public:
    customQProxyStyle();
    ~customQProxyStyle();

protected:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override;
};

class CustomListWidgetItem : public QListWidgetItem
{

public:
    QUrl url;
    int index = 0;
};



#endif // APPCONFIG_H

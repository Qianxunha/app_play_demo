#include "videoWidget.h"

//系统 拍照录像设置
QString sysPhotoPath = "D:/app_play_demo";

/*
 * 构造函数
 * @param:
 */
VideoWidget::VideoWidget(int mwidth, int mheight, VideoThread *videoThread, QWidget*parent):QOpenGLWidget(parent)
{
    pVideoThread = videoThread;
    this->hwDecodeFlag = pVideoThread->hwDecodeFlag;
    imgWidth = 80;
    imgHeight = 45;

    //图标路径
    this->strImg_min      = ":/new/prefix1/videoImage/min";
    this->strImg_max      = ":/new/prefix1/videoImage/title_max";
    this->strImg_close    = ":/new/prefix1/videoImage/close";
    this->strImg_openFile = ":/new/prefix1/videoImage/openFile";
    this->strImg_previous = ":/new/prefix1/videoImage/previous";
    this->strImg_next     = ":/new/prefix1/videoImage/next";
    this->strImg_stop     = ":/new/prefix1/videoImage/stop";
    this->strImg_play     = ":/new/prefix1/videoImage/play";
    this->strImg_voice    = ":/new/prefix1/videoImage/voice";
    this->strImg_photo    = ":/new/prefix1/videoImage/videoPhoto";
    this->strImg_videoMax = ":/new/prefix1/videoImage/max1";

    this->setGeometry(0, 0, mwidth, mheight);
    this->setMinimumSize(QSize(mwidth, mheight));
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setMouseTracking(true);

    //设置父窗体的背景颜色后，可影响到子控件圆角按钮的边界缝隙颜色
    this->setStyleSheet("background:rgb(255,255,255);color:rgb(255,99,45);");
    titleWidget = new QWidget;
    titleWidget->setFixedHeight(40);
    titleWidget->setStyleSheet("background:rgb(33,33,33);");

    minBtn = new CustomPushButton(this->strImg_min, false);
    minBtn->setStyleSheet("border-radius:1px");
    minBtn->setFixedSize(QSize(40, 30));
    minBtn->setCursor(QCursor(Qt::PointingHandCursor));
    minBtn->hide();

    maxBtn = new CustomPushButton(this->strImg_max, true);
    maxBtn->setStyleSheet("border-radius:1px");
    maxBtn->setFixedSize(QSize(40,30));
    maxBtn->setCursor(QCursor(Qt::PointingHandCursor));
    maxBtn->setToolTip("最大化");

    closeBtn = new CustomPushButton(this->strImg_close, false);
    closeBtn->setStyleSheet("border-radius:1px");
    closeBtn->setFixedSize(QSize(40,30));
    closeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    closeBtn->setToolTip("返回");

    connect(minBtn,             &CustomPushButton::clicked,     this, &VideoWidget::on_minBtn_clicked);
    connect(maxBtn,             &CustomPushButton::clicked,     this, &VideoWidget::on_maxBtn_clicked);
    connect(closeBtn,           &CustomPushButton::clicked,     this, &VideoWidget::on_closeBtn_clicked);

    titleLayout = new QHBoxLayout;
    titleLayout->addWidget(maxBtn);
    titleLayout->addWidget(closeBtn);
    titleLayout->setAlignment(Qt::AlignRight);
    titleLayout->setSpacing(0);
    titleLayout->setMargin(0);
    titleWidget->setLayout(titleLayout);

    //播放列表窗口
    listMenu        = new QMenu;
    openAction      = new QAction("添加");
    deleteAction    = new QAction("删除");
    connect(openAction,     &QAction::triggered, this, &VideoWidget::on_addAction_triggered);
    connect(deleteAction,   &QAction::triggered, this, &VideoWidget::on_deleteAction_triggered);

    playControlWidget = new QWidget;
    playControlWidget->setFixedHeight(80);
    playControlWidget->setStyleSheet("QWidget{background-color: rgba(255, 0, 0, 0);}"); //最右边的0 0是全透明 1是不透明 0.5透明度

    durationLabel = new QLabel;
    durationLabel->setFixedWidth(150);
    durationLabel->setStyleSheet("color:white");
    durationLabel->setText("00:00:00 / 00:00:00");

    //视频播放进度条
    videoSlider = new CustomQSlider(nullptr);
    videoSlider->setTracking(true);
    videoSlider->setCursor(QCursor(Qt::PointingHandCursor));
    videoSlider->setEnabled(false);

    //音频播放进度条
    audioCurValue = 10;
    audioSlider = new CustomQSlider(nullptr);
    audioSlider->setCursor(QCursor(Qt::PointingHandCursor));
    audioSlider->setRange(0, 10);
    audioSlider->setValue(audioCurValue);
    audioSlider->setStyleSheet("QSlider{background-color:rgb(0,0,0)}"\
                                 "QSlider::add-page:horizontal{   background-color: rgb(73, 73, 73);  height:4px; }"\
                                 "QSlider::sub-page:horizontal {background-color:rgb(60,170,255);"\
                                 "height:4px;}"\
                                 "QSlider::handle:horizontal {margin-top:-2px; margin-bottom:-2px;"\
                                 "border-radius:4px;  background: rgb(222,222,222); width:8px; height:8px;}"\
                                 "QSlider::groove:horizontal {background:transparent;height:4px}"\
                                 "QSlider::groove:hover:horizontal {background:transparent;height:4px}"\
                                 "QSlider{background-color: rgba(255, 0, 0, 0);}");

    connect(audioSlider, &CustomQSlider::emitSliderValue, this, &VideoWidget::setAudioValue);
    connect(videoSlider, &CustomQSlider::emitSliderValue, this, &VideoWidget::setVideoValue);

    //停止 上一首 下一首 暂停 播放 拍照 视频窗口最大化控制按钮
    previousBtn = new CustomPushButton(this->strImg_previous,false);
    previousBtn->setFixedSize(QSize(30, 30));
    previousBtn->setIconSize(QSize(25, 25));
    previousBtn->setCursor(QCursor(Qt::PointingHandCursor));

    nextBtn = new CustomPushButton(this->strImg_next,false);
    nextBtn->setFixedSize(QSize(30, 30));
    nextBtn->setIconSize(QSize(25, 25));
    nextBtn->setCursor(QCursor(Qt::PointingHandCursor));

    stopBtn = new CustomPushButton(this->strImg_stop,false);
    stopBtn->setFixedSize(QSize(30, 30));
    stopBtn->setIconSize(QSize(20, 20));
    stopBtn->setEnabled(false);
    stopBtn->setCursor(QCursor(Qt::PointingHandCursor));

    playBtn = new CustomPushButton(this->strImg_play,true);
    playBtn->isPlayResourceAvailable = false;
    playBtn->setFixedSize(QSize(30, 30));
    playBtn->setIconSize(QSize(30, 30));
    playBtn->setCursor(QCursor(Qt::PointingHandCursor));

    voiceBtn = new CustomPushButton(this->strImg_voice,true);
    voiceBtn->setFixedSize(QSize(30, 30));
    voiceBtn->setIconSize(QSize(20, 20));
    voiceBtn->setCursor(QCursor(Qt::PointingHandCursor));

    photoBtn= new CustomPushButton(this->strImg_photo, false);
    photoBtn->setFixedSize(QSize(30, 30));
    photoBtn->setIconSize(QSize(23, 23));
    photoBtn->setCursor(QCursor(Qt::PointingHandCursor));
    photoBtn->setEnabled(false);

    videoMaxBtn = new CustomPushButton(this->strImg_videoMax, true);
    videoMaxBtn->setFixedSize(QSize(30, 30));
    videoMaxBtn->setIconSize(QSize(20, 20));
    videoMaxBtn->setCursor(QCursor(Qt::PointingHandCursor));

    //播放倍速框
    speekQCombox = new QComboBox;
    speekQCombox->setFixedSize(QSize(55, 30));
    speekQCombox->setEnabled(false);
    speekQCombox->setCursor(QCursor(Qt::PointingHandCursor));
    speekQCombox->addItem("1.0x", 0);
    speekQCombox->addItem("1.5x", 1);
    speekQCombox->addItem("2.0x", 2);
    speekQCombox->hide();

    connect(stopBtn,        &CustomPushButton::clicked, this, &VideoWidget::on_stopBtn_clicked);
    connect(previousBtn,    &CustomPushButton::clicked, this, &VideoWidget::on_previousBtn_clicked);
    connect(playBtn,        &CustomPushButton::clicked, this, &VideoWidget::on_playBtn_clicked);
    connect(nextBtn,        &CustomPushButton::clicked, this, &VideoWidget::on_nextBtn_clicked);
    connect(voiceBtn,       &CustomPushButton::clicked, this, &VideoWidget::on_voiceBtn_clicked);
    connect(photoBtn,       &CustomPushButton::clicked, this, &VideoWidget::on_photoBtn_clicked);
    connect(videoMaxBtn,    &CustomPushButton::clicked, this, &VideoWidget::on_videoMaxBtn_clicked);
    connect(speekQCombox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,&VideoWidget::Dealmp4FilePlaySpeek);

    //控制窗口部件部件
    controlLayout = new QHBoxLayout;
    controlLayout->setSpacing(20);
    controlLayout->addSpacing(20);
    //这句设置空白因子，占总空白的百分比
    controlLayout->addWidget(durationLabel);
    controlLayout->addStretch(1);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(previousBtn);
    controlLayout->addWidget(playBtn);
    controlLayout->addWidget(nextBtn);
    controlLayout->addWidget(voiceBtn);
    controlLayout->addWidget(audioSlider);
    controlLayout->addStretch(1);
    controlLayout->addSpacing(100);
    controlLayout->addWidget(speekQCombox);
    controlLayout->addWidget(photoBtn);
    controlLayout->addSpacing(0);
    controlLayout->addWidget(videoMaxBtn);
    controlLayout->addSpacing(20);
    controlLayout->setAlignment(Qt::AlignCenter);

    playControlLayout = new QVBoxLayout;
    playControlLayout->setMargin(0);
    playControlLayout->addWidget(videoSlider);
    playControlLayout->addLayout(controlLayout);
    playControlWidget->setLayout(playControlLayout);

    //播放窗口列表标签
    listTitleLabel = new QLabel;
    listTitleLabel->setFixedHeight(30);
    listTitleLabel->setAlignment(Qt::AlignTop);
    listTitleLabel->setStyleSheet("background-color: rgb(33,33,33);color:rgb(255,99,45);");
    listTitleLabel->setText(tr("  播放列表          共 0 个视频"));


    //播放窗口列表
    playListWidget = new QListWidget;
    playListWidget->setFixedWidth(280);
    playListWidget->setStyleSheet("background:rgb(33,33,33);");
    playListWidget->setFrameShape(QFrame::NoFrame);
    playListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    playListWidget->setFrameShape(QListWidget::NoFrame);
    playListWidget->setIconSize(QSize(80,45));          //设置图片视频缩略图大小
    playListWidget->setSpacing(5);                      //设置单元格间距
    playListWidget->setResizeMode(QListWidget::Adjust);
    playListWidget->setStyle(new customQProxyStyle);
    //playListWidget->setMouseTracking(true);
    playListWidget->setWordWrap(true);
    playListWidget->setTextElideMode(Qt::ElideNone);
    playListWidget->setCursor(QCursor(Qt::PointingHandCursor)); //


    connect(playListWidget, &QListWidget::itemClicked,                  this, &VideoWidget::playVideo);
    //connect(playListWidget, &QListWidget::itemEntered,                  this, &VideoWidget::DealPlayListMouseEnter);
    connect(playListWidget, &QListWidget::customContextMenuRequested,   this, &VideoWidget::listWidgetMenu);


    playListWidget->verticalScrollBar()->setStyleSheet("QScrollBar{ background:rgb(204,204,204); width:18px ;margin-top:16px;margin-bottom:16px}"\
                                                       "QScrollBar::handle:vertical:hover{width:18px;background:rgb(167,167,167);}"\
                                                       "QScrollBar::sub-line:vertical{height:16px;subcontrol-position:top;subcontrol-origin:margin;}"\
                                                       "QScrollBar::add-line:vertical{height:16px;subcontrol-position:bottom;subcontrol-origin:margin;}");


    playListWidget->horizontalScrollBar()->setStyleSheet("QScrollBar{background:rgb(204,204,204); height:18px ;margin-left:16px;margin-right:16px}"\
                                                         "QScrollBar::handle:horizontal:hover{height:18px;background:rgb(167,167,167);}"\
                                                         "QScrollBar::sub-line:horizontal{width:16px;subcontrol-position:left;subcontrol-origin:margin;}"\
                                                         "QScrollBar::add-line:horizontal{width:16px;subcontrol-position:right;subcontrol-origin:margin;}");

    rightBarWidget = new QWidget;
    rightBarWidget->setFixedWidth(280);
    rightBarLayout = new QVBoxLayout;
    rightBarLayout->setSpacing(0);
    rightBarLayout->setMargin(0);
    rightBarLayout->addWidget(listTitleLabel);
    rightBarLayout->addWidget(playListWidget);
    rightBarWidget->setLayout(rightBarLayout);


    playLayout = new QVBoxLayout;
    playLayout->addStretch();
    playLayout->addWidget(playControlWidget);

    //widget 在隐藏之后会自动被其他layout占用，而layout在隐藏之后则不行
    playAndListLayout = new QHBoxLayout;
    playAndListLayout->addLayout(playLayout);
    playAndListLayout->addWidget(rightBarWidget);

    displayLayout = new QVBoxLayout;
    displayLayout->addWidget(titleWidget);
    displayLayout->addLayout(playAndListLayout);

    displayLayout->setSpacing(0);
    displayLayout->setMargin(0);
    displayLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    this->setLayout(displayLayout);


    //打开视频文件
    openFileBtn = new CustomPushButton(this->strImg_openFile,false);
    openFileBtn->setCursor(QCursor(Qt::PointingHandCursor));
    openFileBtn->setFixedSize(QSize(170, 50));
    openFileBtn->setIconSize(QSize(50, 30));
    openFileBtn->setText("打开文件");
    openFileBtn->setStyleSheet(
                                "QPushButton:hover:pressed{;border-color:red;color: rgb(0,97,178);background-color: rgb(0,0,0);}"\
                                "QPushButton:hover{background-color:rgb(0,0,0);color:rgb(18,150,219);}"\
                                "QPushButton{background-color: rgb(0,0,0);color:rgb(250,250,250);}"\
                                "QPushButton{font-size:20px;font:bold;border-radius:1px;}");

    openFileBtn->setParent(this);
    openFileBtn->setFlat(true);

    connect(openFileBtn,&QPushButton::clicked,                  this,   &VideoWidget::on_openFileBtn_clicked);
    connect(this,       &VideoWidget::emitVideoWidgetMaxMain,   this,   &VideoWidget::DealVideoWidgetMaxMain);

    //全屏后显示正在播放的文件名
    labelVideoName = new QLabel(this);
    labelVideoName->setFixedSize(QSize(300, 30));
    labelVideoName->move((this->width()-300)/2-50,10);
    labelVideoName->setAlignment(Qt::AlignCenter);
    labelVideoName->setStyleSheet("QLabel{background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255);};");
    labelVideoName->hide();

    closeBtn1 = new CustomPushButton(this->strImg_close, false);
    closeBtn1->setStyleSheet("border-radius:1px");
    closeBtn1->setFixedSize(QSize(40,30));
    closeBtn1->setCursor(QCursor(Qt::PointingHandCursor));
    closeBtn1->setToolTip("返回");
    closeBtn1->setParent(this);
    closeBtn1->move(this->width()-40, 5);
    closeBtn1->setFlat(true);
    closeBtn1->setStyleSheet("QPushButton{background-color: rgba(255, 255, 255, 0)};");
    connect(closeBtn1,           &CustomPushButton::clicked,     this, &VideoWidget::on_closeBtn1_clicked);

    //进入视频回放模块，开始检测
    //TimerMp4FileCheck = new QTimer;

    TimerMp4FilePlay = new QTimer;
    TimerMp4FilePlay->setTimerType(Qt::PreciseTimer);
    connect(TimerMp4FilePlay,   &QTimer::timeout,              this,   &VideoWidget::DealMp4FilePlayTimer);

    connect(pVideoThread,  &VideoThread::emitMP4Nv12Data,      this,   &VideoWidget::recvNv12pDataShow);
    connect(pVideoThread,  &VideoThread::emitMP4Yuv420pData,   this,   &VideoWidget::recvYuv420pDataShow);
    connect(pVideoThread,  &VideoThread::emitOpenFileButton,   this,   &VideoWidget::DealemitOpenFileButton);
    connect(pVideoThread,  &VideoThread::emitGetMp4FileTime,   this,   &VideoWidget::DealemitGetMp4FileTime);
    connect(pVideoThread,  &VideoThread::emitStopPlayTimes,    this,   &VideoWidget::DealemitStopPlayTimes);
    connect(pVideoThread,  &VideoThread::emitGetPlayStatus,    this,   &VideoWidget::DealemitGetPlayStatus);
    connect(pVideoThread,  &VideoThread::emitGetStopStatus,    this,   &VideoWidget::DealemitGetStopStatus);
    connect(pVideoThread,  &VideoThread::emitMp4FileError,     this,   &VideoWidget::DealemitMp4FileError);

    //音频添加 20210107
    connect(pVideoThread,  &VideoThread::emitAudioData,        this,   &VideoWidget::DealemitAudioData);
    connect(pVideoThread,  &VideoThread::emitGetAudioFormat,   this,   &VideoWidget::DealemitAudioFormat);

#if OS_WINDOWS_QT
    maxBtn->setStyleSheet("color:rgb(255,255,255);font-family:Microsoft YaHei;font-size:15px;border:none;");
    closeBtn->setStyleSheet("color:rgb(255,255,255);font-family:Microsoft YaHei;font-size:15px;border:none;");
    listMenu->setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
    listTitleLabel->setFont(QFont("Microsoft YaHei", 10,40));
#elif OS_MAC_QT
    maxBtn->setStyleSheet("color:rgb(255,255,255);font-family:Helvetica;font-size:15px;border:none;");
    closeBtn->setStyleSheet("color:rgb(255,255,255);font-family:Helvetica;font-size:15px;border:none;");
    listMenu->setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
    listTitleLabel->setFont(QFont("Helvetica", 10,40));
#endif

}

/*
 * function:析构函数
 */
VideoWidget::~VideoWidget()
{

}

/*******  start openGL 渲染   **********************************************************************/


/*
 * function: OpenGL渲染初始化
 */
void VideoWidget:: initializeGL()
{
    if(this->hwDecodeFlag)
    {
        initialize_Nv12();
    }
    else
    {
        initialize_yuv420p();
    }
}

/*
 * function:openGL渲染mp4文件
 */
void VideoWidget:: paintGL()
{
    if(m_type == 1)
    {
        render(m_py, m_puv, m_width, m_height, m_type);

    }
    else if(m_type == 2)
    {
        render(m_py, m_pu, m_pv, m_width, m_height, m_type);
    }
}

/*
 * function:重载虚函数
 */
void VideoWidget::resizeGL(int w, int h)
{
    //重置当前的视口
    glViewport(0, 0, (GLint)w, (GLint)h);

    if(rightBarWidget->isHidden())
        openFileBtn->move((this->width()-50)/2-85, this->height()/2-25);
    else
        openFileBtn->move((this->width()-340)/2-85, this->height()/2-25);
    labelVideoName->move((this->width()-300)/2-50,10);
    closeBtn1->move(this->width()-40, 5);
}

/*
 * function:检测是否有音频设备添加
 */
bool VideoWidget::isAudioDeviceOk()
{
    for(auto &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        if(deviceInfo.deviceName()!=nullptr)
        {
            return true;
        }
    }
    return false;
}

/*
 * function: 检测音频设备是否有 无则提示没有
 */
void VideoWidget::initAudioDeviceOk()
{
    //检测音频设备是否连接
    //if(isAudioDeviceOk())
    {
        if(audioDeviceHaveFlag)
        {
            audioDeviceFlag     = true;
            audioDeviceHaveFlag = false;
            initAudioDevice(mp4_sampleRate, mp4_sampleSize, mp4_channelCount);
            //APPCustom::customQMessage(tr("已插入扬声器或耳机"));
        }
    }
    //else
    //{
    //    if(audioDeviceFlag)
    //    {
    //        audioDeviceFlag     = false;
    //        audioDeviceHaveFlag = true;
    //        APPCustom::customQMessage(tr("未插入扬声器或耳机"));
    //    }
    //}
}

/*
 * function: videoThread线程中 解码mp4文件后 nv12数据 OpenGL渲染显示
 * @param: m_py  y分量
 * @param: m_puv uv分量
 * @param: width
 * @param: height
 * @param: hw_type
 *
 * @return :
 */
void VideoWidget::recvNv12pDataShow(uint8_t* m_py, uint8_t* m_puv, int width, int height, int hw_type)
{
    //initAudioDeviceOk();
    this->m_py = m_py;
    this->m_puv = m_puv;
    this->m_width = width;
    this->m_height = height;
    this->m_type = hw_type;
    this->update();
}

/*
 * function: videoThread线程中 解码mp4文件后 yuv数据 OpenGL渲染显示
 * @param: m_py y分量
 * @param: m_pu u分量
 * @param: m_pv v分量
 * @param: width
 * @param: height
 * @param: hw_type
 *
 * @return :
 */
void VideoWidget::recvYuv420pDataShow(uint8_t* m_py, uint8_t *m_pu, uint8_t*m_pv, int width, int height, int hw_type)
{
    //initAudioDeviceOk();
    this->m_py = m_py;
    this->m_pu = m_pu;
    this->m_pv = m_pv;
    this->m_width  = width;
    this->m_height = height;
    this->m_type   = hw_type;
    this->update();
}

/*
 * function: 在paintGL中渲染
 * @param: py
 * @param: puv
 * @param: width
 * @param: height
 * @param: type
 */
void VideoWidget::render(uchar* py,uchar* puv, int w, int h, int m_type)
{
    if(w == 0 || h ==0 || py ==nullptr || puv==nullptr)
        return;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    try{
        program.bind();
        vbo.bind();
        program.enableAttributeArray("vertexIn");
        program.enableAttributeArray("textureIn");
        program.setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
        program.setAttributeBuffer("textureIn",GL_FLOAT,2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D,idY);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RED,w,h,0,GL_RED,GL_UNSIGNED_BYTE,py);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D,idUV);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RG,w >> 1,h >> 1,0,GL_RG,GL_UNSIGNED_BYTE,puv);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        program.setUniformValue("textureUV",0);
        program.setUniformValue("textureY",1);
        glDrawArrays(GL_QUADS,0,4);
        program.disableAttributeArray("vertexIn");
        program.disableAttributeArray("textureIn");
        program.release();
    }
    catch(QException&err)
    {
        emit emitOpenglError(1);
    }
}

/*
 * function: 在paintGL中渲染
 * @param: py
 * @param: pu
 * @param: pv
 * @param: width
 * @param: height
 * @param: type
 */
void VideoWidget::render(uchar* py,uchar* pu,uchar* pv,int width,int height,int type)
{
    if(width == 0 || height == 0 || py == nullptr || pu ==nullptr || pv==nullptr)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_idy);
    //修改纹理内容(复制内存内容)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE,py);
    //与shader 关联
    glUniform1i(m_textureUniformY, 0);

    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, m_idu);
    //修改纹理内容(复制内存内容)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, pu);
    //与shader 关联
    glUniform1i(m_textureUniformU,1);

    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, m_idv);
     //修改纹理内容(复制内存内容)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width/2, height/2, 0, GL_RED, GL_UNSIGNED_BYTE, pv);
    //与shader 关联
    glUniform1i(m_textureUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void VideoWidget::initialize_Nv12()
{
    initializeOpenGLFunctions();
    const char *vsrc =
            "attribute vec4 vertexIn; \
             attribute vec4 textureIn; \
             varying vec4 textureOut;  \
             void main(void)           \
             {                         \
                 gl_Position = vertexIn; \
                 textureOut = textureIn; \
             }";

    const char *fsrc =
            "varying mediump vec4 textureOut;\n"
            "uniform sampler2D textureY;\n"
            "uniform sampler2D textureUV;\n"
            "void main(void)\n"
            "{\n"
            "vec3 yuv; \n"
            "vec3 rgb; \n"
            "yuv.x = texture2D(textureY, textureOut.st).r - 0.0625; \n"
            "yuv.y = texture2D(textureUV, textureOut.st).r - 0.5; \n"
            "yuv.z = texture2D(textureUV, textureOut.st).g - 0.5; \n"
            "rgb = mat3( 1,       1,         1, \n"
                        "0,       -0.39465,  2.03211, \n"
                        "1.13983, -0.58060,  0) * yuv; \n"
            "gl_FragColor = vec4(rgb, 1); \n"
            "}\n";
    program.addShaderFromSourceCode(QOpenGLShader::Vertex,vsrc);
    program.addShaderFromSourceCode(QOpenGLShader::Fragment,fsrc);
    program.link();

    GLfloat points[]{
        -1.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, -1.0f,
        -1.0f, -1.0f,

        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
        0.0f,1.0f
    };


    vbo.create();
    vbo.bind();
    vbo.allocate(points,sizeof(points));

    GLuint ids[2];
    glGenTextures(2,ids);
    idY = ids[0];
    idUV = ids[1];
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void VideoWidget::initialize_yuv420p()
{
    initializeOpenGLFunctions();
    //顶点shader
    const char *vString =
       "attribute vec4 vertexPosition;\
        attribute vec2 textureCoordinate;\
        varying vec2 texture_Out;\
        void main(void)\
        {\
            gl_Position = vertexPosition;\
            texture_Out = textureCoordinate;\
        }";
    //片元shader
    const char *tString =
        "varying vec2 texture_Out;\
        uniform sampler2D tex_y;\
        uniform sampler2D tex_u;\
        uniform sampler2D tex_v;\
        void main(void)\
        {\
            vec3 YUV;\
            vec3 RGB;\
            YUV.x = texture2D(tex_y, texture_Out).r;\
            YUV.y = texture2D(tex_u, texture_Out).r - 0.5;\
            YUV.z = texture2D(tex_v, texture_Out).r - 0.5;\
            RGB = mat3(1.0, 1.0, 1.0,\
                0.0, -0.39465, 2.03211,\
                1.13983, -0.58060, 0.0) * YUV;\
            gl_FragColor = vec4(RGB, 1.0);\
        }";

    //m_program加载shader（顶点和片元）脚本
    //片元（像素）
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, tString);
    //顶点shader
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vString);

    //设置顶点位置
    m_program.bindAttributeLocation("vertexPosition",ATTRIB_VERTEX);
    //设置纹理位置
    m_program.bindAttributeLocation("textureCoordinate",ATTRIB_TEXTURE);

    //编译shader,并检测电脑是否支持OpenGL
    m_program.link();
    emit emitComputerSupportOpenGL(m_program.bind());

    //传递顶点和纹理坐标
    //顶点
    static const GLfloat ver[] = {
        -1.0f,-1.0f,
        1.0f,-1.0f,
        -1.0f, 1.0f,
        1.0f,1.0f
    };
    //纹理
    static const GLfloat tex[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    //设置顶点,纹理数组并启用
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);

    //从shader获取地址
    m_textureUniformY = m_program.uniformLocation("tex_y");
    m_textureUniformU = m_program.uniformLocation("tex_u");
    m_textureUniformV = m_program.uniformLocation("tex_v");

    //创建纹理
    glGenTextures(1, &m_idy);
    //Y
    glBindTexture(GL_TEXTURE_2D, m_idy);
    //放大过滤，线性插值   GL_NEAREST(效率高，但马赛克严重)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //U
    glGenTextures(1, &m_idu);
    glBindTexture(GL_TEXTURE_2D, m_idu);
    //放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //V
    glGenTextures(1, &m_idv);
    glBindTexture(GL_TEXTURE_2D, m_idv);
    //放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

/*******  end openGL 渲染   **********************************************************************/

/*
 * function:鼠标点击事件
 */
void VideoWidget::mousePressEvent(QMouseEvent * event)
{

}


/*
 * function:鼠标移动事件
 */
void VideoWidget::mouseMoveEvent(QMouseEvent * event)
{

}

/*
 * function: 鼠标释放事件
 */
void VideoWidget::mouseReleaseEvent(QMouseEvent * event)
{

}

/*
 * function: 鼠标双击事件
 */
void VideoWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
    emit emitDoubleClick(event);
}

/*
 * function:重写改变事件
 */
void VideoWidget::changeEvent(QEvent* event)
{

}

/*
 * function: 重写关闭事件
 */
void VideoWidget::closeEvent(QCloseEvent *event)
{

}

void VideoWidget::startCheckVideoFile(QString videoDirPath)
{
    this->videoDirPath = videoDirPath;
}


void VideoWidget::getPlayVideoTimes(int num, QString &str)
{
    QString strH, strM, strS;
    if((num/3600)>0 )
    {
        if(num/3600 >=0 && num/3600 <10)
            strH = QString("0%1").arg(num/3600);
        else if( num/3600 >=10 && num/3600 <24)
            strH = QString("%1").arg(num/3600);
    }
    else
    {
        strH="00";
    }

    if((num/60)>0)
    {
        int k = num/60;
        if(k>=0&&k<10)
        {
            strM = QString("0%1").arg(k);
        }
        else if(k >=10 &&k< 60)
        {
            strM = QString("%1").arg(k);
        }
        else if(k>=60)
        {
            k=k%60;
            if(k>=0 && k<10)
                strM = QString("0%1").arg(k);
            else if(k>=10 && k<60)
                strM = QString("%1").arg(k);
        }
    }
    else
    {
        strM="00";
    }

    if(num%60)
    {
        if(num%60>0 && num%60<10)
            strS = QString("0%1").arg(num%60);
        else if(num%60>=10 && num%60<60)
            strS = QString("%1").arg(num%60);
    }
    else
        strS = "00";
    str = QString("%1:%2:%3").arg(strH).arg(strM).arg(strS);
}


QString VideoWidget::getFileNameSamba(QString filePath)
{
    QStringList list = filePath.split('/');
    if(list.count())
        return list.at(list.count()-1);
    else
        return filePath;
}

/*
 * function: using samba protocol to play video
 */
void VideoWidget::playVideoUsingSamba(QStringList list)
{
    if(!list.count())
        return;
    QString filePath;
    QString fileName;
    QString jpgPath;

    if(playListWidget->count() == 0)
    {
        for(int i=0; i<list.count(); i++)
        {
            filePath = list.at(i);
            fileName = getFileNameSamba(filePath);

            CustomListWidgetItem* item = new CustomListWidgetItem;
            item->index = 1;
            item->url = "file:///"+filePath;
            item->setText(fileName);
            item->setToolTip(filePath);
            item->setTextColor(QColor(255,255,255));
            item->setSizeHint(QSize(270, 45));
            QFont list_font;
        #if OS_WINDOWS_QT
            list_font.setFamily("Microsoft YaHei");
            list_font.setPointSize(10);
        #elif OS_MAC_QT
            list_font.setFamily("Helvetica");
            list_font.setPointSize(14);
        #endif
            item->setFont(list_font);

            //显示视频缩略图
            getJpgImageName(fileName,jpgPath);
            if(getVideoFirstFrame(filePath.toStdString().c_str(), jpgPath.toStdString().c_str())<0)
            {
                emit emitGetFirstFrameImage(fileName);
                delete item;
                item = nullptr;
                continue;
            }
            item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
            playListWidget->insertItem(0, item);
        }
    }
    else
    {
        for(int i=0; i<list.count(); i++)
        {
            int n =0; //统计播放列表和视频文件夹中相同的视频个数
            filePath = list.at(i);
            fileName = getFileNameSamba(filePath);

            CustomListWidgetItem* item = new CustomListWidgetItem;
            item->index = 1;
            item->url = "file:///"+filePath;
            item->setText(fileName);
            item->setToolTip(filePath);
            item->setTextColor(QColor(255,255,255));
            item->setSizeHint(QSize(270, 45));
            QFont list_font;
        #if OS_WINDOWS_QT
            list_font.setFamily("Microsoft YaHei");
            list_font.setPointSize(10);
        #elif OS_MAC_QT
            list_font.setFamily("Helvetica");
            list_font.setPointSize(14);
        #endif
            item->setFont(list_font);

            for(int j=0; j<playListWidget->count(); j++)
            {
                if(fileName == playListWidget->item(j)->text())
                {
                    n++;
                    break;
                }
            }
            if(n==0)
            {
                getJpgImageName(fileName,jpgPath);
                if(getVideoFirstFrame(filePath.toStdString().c_str(), jpgPath.toStdString().c_str())< 0)
                {
                    emit emitGetFirstFrameImage(fileName);
                    delete item;
                    item = nullptr;
                    continue;
                }
                item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
                playListWidget->insertItem(0, item);
            }
        }
    }

    updateVideoNumber();
}

void VideoWidget::deleteVideoUsingSamba()
{
    CustomListWidgetItem* item = nullptr;
    for(int i=0; i<playListWidget->count(); i++)
    {
        item = dynamic_cast<CustomListWidgetItem*>(playListWidget->takeItem(i));
        if(item == nullptr)
            continue;
        if(!item->index)
        {
            continue;
        }
        else
        {
            playListWidget->removeItemWidget(item);
            delete item;
            item = nullptr;
            i = i-1;
        }
    }
    updateVideoNumber();
}

void VideoWidget::updateFlagStatus(bool flag)
{
    this->loginSuccessFlag = flag;
}

/*
 * function:中英文切换
 */
void VideoWidget::retranslateUI()
{
    openAction->setText(tr("添加"));
    deleteAction->setText(tr("删除"));
    listTitleLabel->setText(tr("播放列表"));
    openFileBtn->setText(tr("打开文件"));
    updateVideoNumber();
    closeBtn->setToolTip(tr("返回"));
    closeBtn1->setToolTip(tr("返回"));
}

/*
 * function: check video file is exits
 */
bool VideoWidget::exists(QString path, CustomListWidgetItem* item)
{
    QFile file(path);
    if(!file.exists())
    {
        //文件不存在则提示 然后删除单元格，更新播放列表 返回
        QMessageBox mess(QMessageBox::Information,tr("提示"),tr("视频文件不存在"));
        QPushButton *okbutton = (mess.addButton(tr("确定"),QMessageBox::AcceptRole));
        mess.setWindowIcon(QIcon(APP_ICON_PATH));
    #if OS_WINDOWS_QT
        mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
    #elif OS_MAC_QT
        mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
    #endif
        mess.exec();
        playListWidget->removeItemWidget(item);
        delete item;
        item = nullptr;
        updateVideoNumber();
        return  false;
    }
    return true;
}

/*
 * function:选中视频，视频文件播放
 * @param：item qlistwidget列表单元项
 */
void VideoWidget::playVideo(QListWidgetItem *item)
{
    if(pVideoThread->mp4FileIsPlay)
        on_stopBtn_clicked();
    if (CustomListWidgetItem* listItem = dynamic_cast<CustomListWidgetItem*>(item))
    {
        /*
         * filePath = "/C:/MVS2/video/2020-11-05-18-46-41.mp4"
         */
        QFile file(listItem->url.path().toStdString().c_str()+1);
        if(!file.exists())
        {
            //文件不存在则提示 然后删除单元格，更新播放列表 返回
            QMessageBox mess(QMessageBox::Information,tr("提示"),tr("视频文件不存在"));
            QPushButton *okbutton = (mess.addButton(tr("确定"),QMessageBox::AcceptRole));
            mess.setWindowIcon(QIcon(APP_ICON_PATH));
        #if OS_WINDOWS_QT
            mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
        #elif OS_MAC_QT
            mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
        #endif
            mess.exec();
            playListWidget->removeItemWidget(listItem);
            delete listItem;
            listItem = nullptr;
            updateVideoNumber();
            return;
        }

        urlVideo = listItem->url.path();
        playBtn->setSwitchIcon();
        pVideoThread->mp4FileIsPlay = true;
        labelVideoName->setText(listItem->url.fileName());
        emit pVideoThread->emitGetMp4FilePath(urlVideo.section("/",1));
    }
}



/*
 * function: 导入视频文件,然后播放视频文件
 */
void VideoWidget::on_openFileBtn_clicked()
{
    QList<QUrl> urlList = QFileDialog::getOpenFileUrls(nullptr, nullptr, QUrl(), "(*.mp4)");
    int i = 0;
    if(urlList.count() == 0)
    {
        return;
    }
    foreach(QUrl url, urlList)
    {
    #if OS_WINDOWS_QT
        CustomListWidgetItem* item = new CustomListWidgetItem;
        item->url = url;
        item->setText(url.fileName());
        item->setToolTip(url.path().toStdString().c_str()+1);

        item->setTextColor(QColor(255,255,255));
        item->setSizeHint(QSize(270, 45));


        QFont list_font;
        list_font.setFamily("Microsoft YaHei");
        list_font.setPointSize(10);
        item->setFont(list_font);

        //显示视频缩略图
        QString jpgPath;
        getJpgImageName(url.fileName(),jpgPath);
        if(getVideoFirstFrame(item->url.path().toStdString().c_str()+1, jpgPath.toStdString().c_str())<0)
        {
            emit emitGetFirstFrameImage(url.fileName());
            delete item;
            item = nullptr;
            return;
        }
        item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
        playListWidget->insertItem(0, item);
        if (i == 0)
        {
            playListWidget->setCurrentItem(item);
            i = 1;
        }
     #elif OS_MAC_QT
        CustomListWidgetItem* item = new CustomListWidgetItem;
        item->url = "file:///" +url.path();
        item->setText(url.fileName());
        item->setToolTip(url.path());

        item->setTextColor(QColor(255,255,255));
        item->setSizeHint(QSize(270, 45));
        QFont list_font;
        list_font.setFamily("Helvetica");
        list_font.setPointSize(14);
        item->setFont(list_font);

        //显示视频缩略图
        QString jpgPath;
        getJpgImageName(url.fileName(),jpgPath);
        if(getVideoFirstFrame(item->url.path().toStdString().c_str(), jpgPath.toStdString().c_str())< 0)
        {
            emit emitGetFirstFrameImage(url.fileName());
            delete item;
            item = nullptr;
            return;
        }
        item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
        playListWidget->insertItem(0, item);
        if (i == 0)
        {
            playListWidget->setCurrentItem(item);
            i = 1;
        }
     #endif
    }
    updateVideoNumber();

    if(CustomListWidgetItem* listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem()))
    {
        urlVideo = listItem->url.path();
        pVideoThread->mp4FileIsPlay = true;
        labelVideoName->setText(listItem->url.fileName());
        emit pVideoThread->emitGetMp4FilePath(urlVideo.section("/",1));
    }
}

/*
 * function: 停止播放
 */
void VideoWidget::on_stopBtn_clicked()
{
    pVideoThread->mp4FileIsStop = true;
    openFileBtn->show();
    playBtn->setStopIcon();
    this->update();

}

/*
 * function: 上一首播放
 */
void VideoWidget::on_previousBtn_clicked()
{
    if (playListWidget->count() == 0)
    {
        return;
    }
    if (playListWidget->currentRow() - 1 < 0)
        playListWidget->setCurrentRow(playListWidget->count() - 1);
    else
        playListWidget->setCurrentRow(playListWidget->currentRow() - 1);
    if(CustomListWidgetItem* listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem()))
    {
        //关闭上一首播放
        if(pVideoThread->mp4FileIsPlay)
            pVideoThread->mp4FilePreviousPlay = true;

        urlVideo = listItem->url.path();
        if(!exists(urlVideo.section("/", 1), listItem))
        {
            return;
        }

        emit pVideoThread->emitGetMp4FilePath(urlVideo.section("/",1));
        labelVideoName->setText(listItem->url.fileName());
        if(this->isFullScreen())
            labelVideoName->show();
    }
}

/*
 * function: 播放控制 暂停
 */
void VideoWidget::on_playBtn_clicked()
{
    if(!pVideoThread->mp4FileIsPlay)
    {
        if (CustomListWidgetItem* listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem()))
       {
           //关闭当前播放的视频文件
           if(pVideoThread->mp4FileIsPlay)
               pVideoThread->mp4FileNextPlay = true;

           urlVideo = listItem->url.path();
           if(!exists(urlVideo.section("/", 1), listItem))
           {
               return;
           }

           emit pVideoThread->emitGetMp4FilePath(urlVideo.section("/", 1));
           labelVideoName->setText(listItem->url.fileName());
           if(this->isFullScreen())
               labelVideoName->show();
           return;
       }
       else
       {
           playBtn->isPlayResourceAvailable = false;
           playBtn->setChecked(false);
           if(playListWidget->count()>0)
           {
               QMessageBox mess(QMessageBox::Information,tr("提示"),tr("请选择需要播放的视频文件"));
               QPushButton *okbutton = (mess.addButton(tr("确定"),QMessageBox::AcceptRole));
               mess.setWindowIcon(QIcon(APP_ICON_PATH));
            #if OS_WINDOWS_QT
                mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
            #elif OS_MAC_QT
                mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
            #endif
               mess.exec();
           }
           else
           {
               QMessageBox mess(QMessageBox::Information,tr("提示"),tr("当前播放列表中没有视频文件"));
               QPushButton *okbutton = (mess.addButton(tr("确定"),QMessageBox::AcceptRole));
               mess.setWindowIcon(QIcon(APP_ICON_PATH));
            #if OS_WINDOWS_QT
                mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
            #elif OS_MAC_QT
                mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
            #endif
               mess.exec();
           }

           return;
       }
    }

    if(pVideoThread->mp4FileIsPause)
    {
        pVideoThread->mp4FileIsPause = false;
        pVideoThread->mp4FileIsPlay  = true;
        playBtn->isPlayResourceAvailable = false;
        playBtn->setChecked(false);
        TimerMp4FilePlay->stop();
    }
    else
    {
        pVideoThread->mp4FileIsPause = true;
        pVideoThread->mp4FileIsPlay  = true;
        playBtn->isPlayResourceAvailable = true;
        playBtn->setChecked(true);
        playBtn->setIcon(QIcon(this->strImg_play +"_checked.png"));
        TimerMp4FilePlay->start(mp4_playtimes);
    }
}

/*
 * function: 下一首播放
 */
void VideoWidget::on_nextBtn_clicked()
{
    if(playListWidget->count() == 0)
    {
        return;
    }
    if (playListWidget->currentRow() + 1 >= playListWidget->count())
        playListWidget->setCurrentRow(0);
    else
        playListWidget->setCurrentRow(playListWidget->currentRow() + 1);
    if (CustomListWidgetItem* listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem()))
    {
        //关闭当前播放的视频文件
        urlVideo = listItem->url.path();
        if(pVideoThread->mp4FileIsPlay)
            pVideoThread->mp4FileNextPlay = true;

        if(!exists(urlVideo.section("/", 1), listItem))
        {
            return;
        }

        emit pVideoThread->emitGetMp4FilePath(urlVideo.section("/", 1));
        labelVideoName->setText(listItem->url.fileName());
        if(this->isFullScreen())
            labelVideoName->show();
    }
}

/*
 * function: 调节声音按钮 禁音
 */
void VideoWidget::on_voiceBtn_clicked()
{
    if(voiceBtn->isChecked())
    {
        if(!audioDeviceOk)
            return;
        audioOutput->setVolume(0);
    }
    else
    {
        if(!audioDeviceOk)
            return;
        audioOutput->setVolume((qreal)audioCurValue/10);
    }
}

/*
 * function: 拍照
 */
void VideoWidget::on_photoBtn_clicked()
{
    //urlVideo.toStdString().c_str()+1 为何+1："/C:/MVS2/video/2020-10-20-09-31-10.mp4"
    // 播放 "C:/MVS2/video/2020-10-20-09-31-10.mp4"

    int ret = playVideoPhoto(urlVideo.toStdString().c_str()+1 ,photoNum);
    if(ret == 0)
    {
        //拍照成功,进行提示
        QMessageBox *successBox = new QMessageBox(QMessageBox::Information,tr("提示"),tr("拍照成功"));
        QPushButton *okbutton = (successBox->addButton(tr("确定"),QMessageBox::AcceptRole));
        successBox->setWindowIcon(QIcon(APP_ICON_PATH));
        successBox->setModal(true);
    #if OS_WINDOWS_QT
        successBox->setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
    #elif OS_MAC_QT
        successBox->setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
    #endif

        QTimer *mytime = new QTimer;
        mytime->start(1000);
        connect(mytime, &QTimer::timeout, successBox, [&]()
        {
            mytime->stop();
            successBox->close();
        });

        successBox->exec();
        delete mytime;
        delete successBox;
        mytime = nullptr;
        successBox = nullptr;

    }
    else
    {
        //拍照失败,进行提示
        QMessageBox *successBox = new QMessageBox(QMessageBox::Information,tr("提示"),tr("拍照失败"));
        QPushButton *okbutton = (successBox->addButton(tr("确定"),QMessageBox::AcceptRole));
        successBox->setWindowIcon(QIcon(APP_ICON_PATH));
        successBox->setModal(true);
    #if OS_WINDOWS_QT
        successBox->setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
    #elif OS_MAC_QT
        successBox->setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
    #endif

        QTimer *mytime = new QTimer;
        mytime->start(1500);
        connect(mytime, &QTimer::timeout, successBox, [&]()
        {
            mytime->stop();
            successBox->close();
        });

        successBox->exec();
        delete mytime;
        delete successBox;
        mytime = nullptr;
        successBox = nullptr;

    }

}

/*
 * function: 视频最大化
 */
void VideoWidget::on_videoMaxBtn_clicked()
{
    if (videoMaxBtn->isChecked())
    {
        rightBarWidget->hide();
        titleWidget->hide();
        this->showFullScreen();
        openFileBtn->move((this->width()-80)/2 -85 ,this->height()/2-25);

        //显示文件名
        if(openFileBtn->isHidden()){
            labelVideoName->show();
        }
    }
    else
    {
        openFileBtn->move((this->width()-340)/2 -85 ,this->height()/2-25);

        this->showNormal();
        rightBarWidget->show();
        titleWidget->show();
        labelVideoName->hide();
    }
}

/*
 * function: 窗口最小化
 */
void VideoWidget::on_minBtn_clicked()
{
    on_stopBtn_clicked();
    this->hide();
    this->update();
    if(loginSuccessFlag)
        emit emitOpenVideoSignals();
}

/*
 * function: 窗口最大化
 */
void VideoWidget::on_maxBtn_clicked()
{
    emit emitVideoWidgetMax();
}

/*
 * function: 视频模块窗口关闭
 */
void VideoWidget::on_closeBtn_clicked()
{
    on_stopBtn_clicked();
    if(loginSuccessFlag)
        emit emitOpenVideoSignals();
    this->hide();

}

void VideoWidget::on_closeBtn1_clicked()
{
    on_stopBtn_clicked();
    if(loginSuccessFlag)
        emit emitOpenVideoSignals();
    this->hide();
}

/*
 * function: 设置音量大小
 */
void VideoWidget::setAudioValue(int value)
{
    audioCurValue = value;
    audioSlider->setValue(value);

    //当声音按钮处于静音状态时
    if(voiceBtn->isChecked())
    {
        voiceBtn->setIcon(QIcon(this->strImg_voice +".png"));
        voiceBtn->click();
    }

    if(!audioDeviceOk)
        return;
    audioOutput->setVolume((qreal)value/10);

}


/*
 * function: 视频播放进度条
 */
void VideoWidget::setVideoValue(int value)
{
    mp4File_playTimes = value;
    pVideoThread->mp4FileSeek = true;
    pVideoThread->posSeek = value;

    if(!pVideoThread->mp4FileIsPause)
    {
        QString tStr, str1;
        photoNum = mp4File_playTimes++;

        getPlayVideoTimes(mp4File_playTimes, str1);

        tStr = str1 + " / " + mp4File_timesStr;
        durationLabel->setText(tStr);
    }
}

/*
 * function: 视频文件列表 选中视频文件右击显示菜单栏
 */
void VideoWidget::listWidgetMenu()
{
    //选中视频，右击菜单栏
    listMenu->clear();

    if(playListWidget->selectedItems().count() > 0)
    {
        listMenu->addAction(openAction);
        listMenu->addSeparator();
        listMenu->addAction(deleteAction);
        listMenu->addSeparator();
        listMenu->exec(QCursor::pos());
        return;
    }

    if(playListWidget->itemAt(mapFromGlobal(QCursor::pos())) == nullptr)
    {
        listMenu->addAction(openAction);
        listMenu->exec(QCursor::pos());
        return;
    }
}

/*
 * function: 添加视频文件
 */
void VideoWidget::on_addAction_triggered()
{
    QList<QUrl> urlList = QFileDialog::getOpenFileUrls(nullptr,nullptr,QUrl(),"(*.mp4 )");
    int i = 0;
    foreach(QUrl url, urlList)
    {
    #if OS_WINDOWS_QT
        CustomListWidgetItem* item = new CustomListWidgetItem;
        item->url = url;
        item->setText(url.fileName());
        item->setToolTip(url.path().toStdString().c_str()+1);
        item->setTextColor(QColor(255,255,255));
        item->setSizeHint(QSize(270, 45));
        QFont list_font;
        list_font.setFamily("Microsoft YaHei");
        list_font.setPointSize(10);
        item->setFont(list_font);

        //视频缩略图
        QString jpgPath;
        getJpgImageName(url.fileName(),jpgPath);
        if(getVideoFirstFrame(item->url.path().toStdString().c_str()+1, jpgPath.toStdString().c_str())<0)
        {
            emit emitGetFirstFrameImage(url.fileName());
            delete item;
            item = nullptr;
            return;
        }
        item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
        playListWidget->insertItem(0, item);
        if (i == 0)
        {
            playListWidget->setCurrentItem(item);
            i = 1;
        }
    #elif OS_MAC_QT
        CustomListWidgetItem* item = new CustomListWidgetItem;
        item->url = "file:///" + url.path();
        item->setText(url.fileName());
        item->setToolTip(url.path());
        item->setTextColor(QColor(255,255,255));
        item->setSizeHint(QSize(270, 45));
        QFont list_font;
        list_font.setFamily("Helvetica");
        list_font.setPointSize(14);
        item->setFont(list_font);

        //视频缩略图
        QString jpgPath;
        getJpgImageName(url.fileName(),jpgPath);
        if(getVideoFirstFrame(item->url.path().toStdString().c_str()+1, jpgPath.toStdString().c_str())<0)
        {
            emit emitGetFirstFrameImage(url.fileName());
            delete item;
            item = nullptr;
            return;
        }
        item->setIcon(QPixmap(jpgPath).scaled(imgWidth, imgHeight));
        playListWidget->insertItem(0, item);
        if (i == 0)
        {
            playListWidget->setCurrentItem(item);
            i = 1;
        }
    #endif
    }
    updateVideoNumber();
}

/*
 * function: 删除视频文件
 */
void  VideoWidget::on_deleteAction_triggered()
{
    if (playListWidget->selectedItems().count() > 0)
    {
        CustomListWidgetItem* item = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem());
        QMessageBox mess(QMessageBox::Information,tr("提示"),tr("是否要永久性删除此文件吗"));
        QPushButton *okbutton = (mess.addButton(tr("是"),QMessageBox::YesRole));
        QPushButton *nobutton = (mess.addButton(tr("否"),QMessageBox::NoRole));
        mess.setWindowIcon(QIcon(APP_ICON_PATH));
    #if OS_WINDOWS_QT
        mess.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
    #elif OS_MAC_QT
        mess.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
    #endif
        int ret = mess.exec();

        if(ret == 0)
        {
            if(QFile::remove(item->url.path().toStdString().c_str()+1))
            {
                //条件为真，返回true 成功删除文件
                listItemRow = listItemRow-1;

                //删除图片文件
                QString filePath;
                getJpgImageName(item->url.fileName(), filePath);
                QFile::remove(filePath);
            }
            else
            {
                QMessageBox messfail(QMessageBox::Information,tr("提示"),tr("文件正在使用中，删除失败"));
                QPushButton *okbutton = (messfail.addButton(tr("是"),QMessageBox::YesRole));
                messfail.setWindowIcon(QIcon(APP_ICON_PATH));
            #if OS_WINDOWS_QT
                messfail.setStyleSheet("*{font-family:Microsoft YaHei;font-size:15px;}");
            #elif OS_MAC_QT
                messfail.setStyleSheet("*{font-family:Helvetica;font-size:15px;}");
            #endif
                messfail.exec();
                return;
            }
            playListWidget->removeItemWidget(item);
            delete item;
            item = nullptr;
        }
        else if(ret == 1)
        {
            return;
        }
    }
    updateVideoNumber();
}

/*
 * function: 统计播放列表视频个数
 */
void VideoWidget::updateVideoNumber()
{
    QString str = QString(tr("  播放列表          共 %1 个视频")).arg(playListWidget->count());
    listTitleLabel->setText(str);
    this->update();
}



/*
 * function: 拍照截图，根据播放秒数寻找最近的I帧截图为jpg图片 time=2020/10/20
 * @param: filePath 视频文件路径名
 * @param: index  播放秒数
 */
int VideoWidget::playVideoPhoto(const char *filePath, int index)
{
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec     = nullptr;
    AVFrame *frameyuv   = nullptr;
    AVFrame *frame      = nullptr;
    AVPacket *pkt       = nullptr;
    struct SwsContext *swsContext = nullptr;
    int got_picture = 0;

    pFormatCtx = avformat_alloc_context();
    int res = avformat_open_input(&pFormatCtx, filePath, nullptr, nullptr);
    if(res)
    {
        emit emitGetVideoImage(0);
        return -1;
    }
    avformat_find_stream_info(pFormatCtx, nullptr);

    int videoStream = -1;
    for(int i=0; i < (int)pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if(videoStream == -1)
    {
        emit emitGetVideoImage(0);
        return -1;
    }

    //查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if(pCodec == nullptr)
    {
        emit emitGetVideoImage(0);
        return -1;
    }

    //打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, nullptr)<0)
    {
        emit emitGetVideoImage(0);
        return -1;
    }

    res = av_seek_frame(pFormatCtx, -1, index * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);//10(second)
    if (res<0)
    {
        return 0;
    }

    frame = av_frame_alloc();
    frameyuv =  av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)frameyuv, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    pkt = (AVPacket *)av_malloc(sizeof(AVPacket));

    swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height,pCodecCtx->pix_fmt,
                                            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC, nullptr, nullptr, nullptr);


    while(av_read_frame(pFormatCtx, pkt)>=0)
    {
        //读取一帧压缩数据
        if(pkt->stream_index == videoStream)
        {
            //解码一帧数据
            res = avcodec_decode_video2(pCodecCtx, frame, &got_picture, pkt);
            if(res < 0)
            {
                emit emitGetVideoImage(0);
                return -1;
            }
            if(got_picture)
            {
                sws_scale(swsContext, (const uint8_t *const*)frame->data,frame->linesize,0,pCodecCtx->height,frameyuv->data, frameyuv->linesize);
                playVideoSaveJpgImage(frameyuv, pCodecCtx->width, pCodecCtx->height, index);
                break;
            }
        }
    }

    //释放内存
    if(frameyuv!=nullptr)
    {
        av_frame_free(&frameyuv);
        frameyuv = nullptr;
    }

    av_free_packet(pkt);

    if(swsContext!=nullptr){
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }

    if(frame !=nullptr){
        av_frame_free(&frame);
        frame = nullptr;
    }
    avformat_close_input(&pFormatCtx);
    av_free(out_buffer);
    return 0;
}


/*
 * function:保存jpg图片
 * @param: frmae 一帧视频数据
 * @param: width 图片宽度
 * @param: height 图片高度
 * @param: index 第几分钟秒数
 */
int VideoWidget::playVideoSaveJpgImage(AVFrame *frame, int width, int height, int index)
{
    QDir dir;
    QString pathStr = sysPhotoPath;     //系统默认存储路径
    if(!dir.exists(pathStr))
    {
        dir.mkpath(pathStr);
    }
    QString strTime;
    APPCustom::getCurrentTime(strTime);
    QString str = pathStr +"/" +strTime + ".jpg";

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx   = nullptr;
    AVStream *pAVStream = nullptr;
    AVCodec *pCodec     = nullptr;
    AVPacket pkt;
    int y_size = 0;
    int got_picture = 0;
    int ret = 0;

    pFormatCtx = avformat_alloc_context();

    pFormatCtx->oformat = av_guess_format("mjpeg",nullptr,nullptr);

    if(avio_open(&pFormatCtx->pb, str.toUtf8(), AVIO_FLAG_READ_WRITE)<0)
    {
        emit emitGetVideoImage(0);
        return -1;
    }
    pAVStream = avformat_new_stream(pFormatCtx, 0);
    if(pAVStream == NULL)
    {
        emit emitGetVideoImage(0);
        return -1;
    }

    pCodecCtx = pAVStream->codec;
    pCodecCtx->codec_id     = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type   = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt      = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width        = width;
    pCodecCtx->height       = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;

    //查找编码器
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);    //mjpeg编码器
    if(!pCodec)
    {
        emit emitGetVideoImage(0);
        return -1;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    {
        emit emitGetVideoImage(0);
        return -1;
    }


    //写 header
    avformat_write_header(pFormatCtx, nullptr);
    y_size = pCodecCtx->width *pCodecCtx->height;
    av_new_packet(&pkt, y_size*3);


    ret = avcodec_encode_video2(pCodecCtx, &pkt, frame, &got_picture);
    if(ret < 0)
    {
        emit emitGetVideoImage(0);
        return -1;
    }
    if(got_picture == 1)
    {
        pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    //添加尾部
    av_write_trailer(pFormatCtx);

    av_free_packet(&pkt);
    if(pAVStream!=nullptr)
        avcodec_close(pAVStream->codec);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 0;
}


/*
 * function:ffmpeg截取视频第一帧，作为视频缩略图 time=2020/10/19
 * @param: filePath 视频文件路径名
 * @param: jpgPath 图片文件路径
 *
 * @return: 成功返回0
 */
int VideoWidget::getVideoFirstFrame(const char *filePath, const char *jpgPath)
{
    //检测jpg图片是否存在，存在不用截图，不存在截取
    QFileInfo fileInfo(jpgPath);
    if(fileInfo.isFile())
    {
        if(fileInfo.size())
            return 0;
    }
    else
        ;

    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx = nullptr;
    AVCodec *pCodec     = nullptr;
    AVFrame *frameyuv   = nullptr;
    AVFrame *frame      = nullptr;
    AVPacket *pkt       = nullptr;
    struct SwsContext *swsContext = nullptr;
    int got_picture = 0;

    pFormatCtx = avformat_alloc_context();

    int res = avformat_open_input(&pFormatCtx, filePath, nullptr, nullptr);
    if(res)
    {
        return -1;
    }
    avformat_find_stream_info(pFormatCtx, nullptr);

    int videoStream = -1;
    for(int i=0; i < (int)pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    //qDebug()<<"videoStream="<<videoStream<<"nb_streams"<<pFormatCtx->nb_streams;
    if(videoStream == -1)
    {
        return -1;
    }

    //查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if(pCodec == nullptr)
    {
        return -1;
    }

    //打开解码器
    if(avcodec_open2(pCodecCtx, pCodec, nullptr)<0)
    {
        return -1;
    }

    frame = av_frame_alloc();
    frameyuv =  av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    avpicture_fill((AVPicture *)frameyuv, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
    pkt = (AVPacket *)av_malloc(sizeof(AVPacket));

    swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height,pCodecCtx->pix_fmt,
                                            pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC, nullptr, nullptr, nullptr);


    while(av_read_frame(pFormatCtx, pkt)>=0)
    {
        //读取一帧压缩数据
        if(pkt->stream_index == videoStream)
        {
            //解码一帧数据
            res = avcodec_decode_video2(pCodecCtx, frame, &got_picture, pkt);
            if(res < 0)
            {
                return -1;
            }
            if(got_picture)
            {
                sws_scale(swsContext, (const uint8_t *const*)frame->data,frame->linesize,0,pCodecCtx->height,frameyuv->data, frameyuv->linesize);
                if(playVideoSaveJpgImage(frameyuv, pCodecCtx->width, pCodecCtx->height, jpgPath) < 0)
                    return -1;
                break;
            }
        }
    }
    avformat_close_input(&pFormatCtx);

    if(frameyuv)
    {
        av_frame_free(&frameyuv);
        frameyuv = nullptr;
    }

    av_free_packet(pkt);

    if(swsContext){
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }

    if(frame){
        av_frame_free(&frame);
        frame = nullptr;
    }
    av_free(out_buffer);
    return 0;
}

/*
 * function:保存jpg图片
 * @param: frmae 一帧视频数据
 * @param: width 图片宽度
 * @param: height 图片高度
 * @param: jpgPath 图片路径名
 */
int VideoWidget::playVideoSaveJpgImage(AVFrame *frame, int width, int height, const char *jpgPath)
{
    AVFormatContext *pFormatCtx = nullptr;
    AVCodecContext *pCodecCtx   = nullptr;
    AVStream *pAVStream = nullptr;
    AVCodec *pCodec     = nullptr;
    AVPacket pkt;

    int y_size = 0;
    int got_picture = 0;
    int ret = 0;

    pFormatCtx = avformat_alloc_context();

    pFormatCtx->oformat = av_guess_format("mjpeg",nullptr,nullptr);

    if(avio_open(&pFormatCtx->pb, jpgPath, AVIO_FLAG_READ_WRITE)<0)
    {
        qDebug()<<"avio_open";
        return -1;
    }
    pAVStream = avformat_new_stream(pFormatCtx, 0);
    if(pAVStream == NULL)
    {
        qDebug()<<"avformat_new_stream";
        return -1;
    }

    pCodecCtx = pAVStream->codec;
    pCodecCtx->codec_id     = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type   = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt      = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width        = width;
    pCodecCtx->height       = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 30;


    //查找编码器
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);    //mjpeg编码器
    if(!pCodec)
    {
        qDebug()<<"avcodec_find_encoder";
        return -1;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    {
        qDebug()<<"avcodec_open2";
        return -1;
    }


    //写 header
    avformat_write_header(pFormatCtx, nullptr);
    y_size = pCodecCtx->width *pCodecCtx->height;
    av_new_packet(&pkt, y_size*3);


    ret = avcodec_encode_video2(pCodecCtx, &pkt, frame, &got_picture);
    if(ret < 0)
    {
        qDebug()<<"avcodec_encode_video2"<<pCodecCtx->codec_id;
        return -1;
    }
    if(got_picture == 1)
    {
        pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    //添加尾部
    av_write_trailer(pFormatCtx);

    av_free_packet(&pkt);
    if(pAVStream!=nullptr)
        avcodec_close(pAVStream->codec);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 1;
}

/*
 * function:根据视频文件名获取jpg图片作为视频缩略图，如果文件存在，则不用
 * @param: fileName 图片文件名
 * @param: jpgPath  图片路径名
 */
int VideoWidget::getJpgImageName(QString fileName, QString &jpgPath)
{
    QStringList list;
    list = fileName.split('.');

    QDir dir;
    QString pathStr = sysPhotoPath + "/videoFrame";     //系统默认存储路径
    if (!dir.exists(pathStr))
    {
        dir.mkpath(pathStr);
    }

    jpgPath = pathStr + "/" + list.at(0) + ".jpg";
    return 0;
}

/*
 * function:视频文件播放，打开文件按钮隐藏 播放结束显示
 * @param  index   1 显示
 *         index   0 隐藏
 */
void VideoWidget::DealemitOpenFileButton(int index)
{
    if(index == 1)
    {
        openFileBtn->show();
        videoSlider->setEnabled(false);
    }
    else if(index == 0)
    {
        openFileBtn->hide();
        videoSlider->setEnabled(true);
    }

}

/*
 * function: 获取mp4文件总时间
 * @param : time mp4文件总秒数
 */

void VideoWidget::DealemitGetMp4FileTime(int times)
{
    mp4File_totalTimes = times;
    TimerMp4FilePlay->start(1000);
    if(times)
    {
        getPlayVideoTimes(mp4File_totalTimes, mp4File_timesStr);
    }
    videoSlider->setMaximum(times);
}

/*
 * function:处理mp4文件播放时间统计
 */
void VideoWidget::DealMp4FilePlayTimer()
{
    QString tStr, str1;
    photoNum = mp4File_playTimes++;

    getPlayVideoTimes(mp4File_playTimes, str1);
    tStr = str1 + " / " + mp4File_timesStr;
    durationLabel->setText(tStr);

    videoSlider->setValue(mp4File_playTimes);
    if(mp4File_playTimes > mp4File_totalTimes)
        pVideoThread->mp4FileIsStop = true;
}

/*
 * function: deal mp4 file decode for ffmpeg failed
 */

void VideoWidget::DealemitMp4FileError(int index)
{
    QString str;
    if(index == 0)
    {
        str = tr("视频文件不存在");
        APPCustom::customQMessage(str);
    }
    else if(index == 1)
    {
        str = tr("视频文件路径中含有中文,存储路径错误");
        APPCustom::customQMessage(str);
    }
    else if(index == 2)
    {
        str = tr("视频文件解码失败");
        APPCustom::customQMessage(str);
    }
    playListWidget->takeItem(playListWidget->currentRow());
    listItemRow = listItemRow-1;
}

/*
 * function:处理文件播放结束后停止定时器
 */
void VideoWidget::DealemitStopPlayTimes()
{
    durationLabel->setText("");
    TimerMp4FilePlay->stop();
    mp4File_playTimes = 0;
}


/*
 * function:处理视频播放完成后播放按钮状态
 */
void VideoWidget::DealemitGetPlayStatus(int index)
{
    if(index == 1)
    {
        playBtn->setSwitchIcon();
        CustomListWidgetItem *listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->currentItem());

        listItem->setTextColor(QColor(255,99,45));
        listItemRow = playListWidget->currentRow();

        //当声音按钮处于静音状态时
        if(voiceBtn->isChecked())
        {
            voiceBtn->setIcon(QIcon(this->strImg_voice +".png"));
            voiceBtn->click();
        }
    }
    else if(index == 0)
    {
        playBtn->setStopIcon();
        durationLabel->setText("00:00:00 / 00:00:00");
        if(playListWidget->count() == 1)
        {
            listItemRow = 0;
        }

        if(listItemRow<0)
            listItemRow = 0;
        if(playListWidget->count() ==0)
            return;
        CustomListWidgetItem *listItem = dynamic_cast<CustomListWidgetItem*>(playListWidget->item(listItemRow));
        listItem->setTextColor(QColor(255,255,255));
    }
}


/*
 * function: 获取停止播放的状态
 */
void VideoWidget::DealemitGetStopStatus(int index)
{
    if(index == 1)
    {
        photoBtn->setEnabled(true);
        stopBtn->setEnabled(true);
        speekQCombox->setEnabled(true);

    }
    else if(index == 0)
    {
        photoBtn->setEnabled(false);
        stopBtn->setEnabled(false);
        speekQCombox->setEnabled(false);
        speekQCombox->setCurrentIndex(0);
        videoSlider->setValue(0);
    }
    else
        ;
}

/*
 * function:释放视频内存
 */
void VideoWidget::freeVideoFile()
{
    while(playListWidget->count())
    {
        playListWidget->takeItem(playListWidget->currentRow());
    }
}

/*
 * function: 处理主窗口发送的信号 视频回放全屏
 */
void VideoWidget::DealVideoWidgetMaxMain(int index)
{
    if(index==0)
    {
        maxBtn->setToolTip(tr("向下还原"));
        maxBtn->setIcon(QIcon(":/new/prefix2/image/videoImage/title_max_checked.png"));
    }
    else if(index==1)
    {
        maxBtn->setToolTip(tr("最大化"));
        maxBtn->setIcon(QIcon(":/new/prefix2/image/videoImage/title_max.png"));
    }
    else
        ;
}

/*
 * function:处理视频播放控制菜单栏显示
 */
void VideoWidget::DealvideoMenu(bool flag)
{
    if(flag)
    {
        playControlWidget->show();
    }
    else
    {
        playControlWidget->hide();
    }
}




/*
 * function:初始化音频设备
 * @param: sampleRate   采样率     16000
 * @param: sampleSize   采样大小    32
 * @param: channelCount 单通道     1
 */
int VideoWidget::initAudioDevice(int sampleRate, int sampleSize, int channelCount)
{
    QAudioFormat format;
    format.setCodec("audio/pcm");
    format.setSampleRate(sampleRate);
    format.setSampleSize(4*sampleSize);
    format.setChannelCount(channelCount);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    audioDeviceOk = info.isFormatSupported(format);
    if(audioDeviceOk)
    {
        audioOutput = new QAudioOutput(format);
        audioDevice = audioOutput->start();
        connect(audioOutput,    &QAudioOutput::stateChanged,    this,   &VideoWidget::DealAudioStateChanged);
    }
    else
    {
       return -1;
    }
    return 0;
}

void VideoWidget::DealemitAudioFormat(int sampleRate, int sampleSize, int channelCount)
{
    mp4_sampleRate = sampleRate;
    mp4_sampleSize = sampleSize;
    mp4_channelCount = channelCount;
    initAudioDevice(mp4_sampleRate, mp4_sampleSize, mp4_channelCount);
}

/*
 * function:释放音频设备
 */
int VideoWidget::freeAudioDevice()
{
    audioOutput->stop();
    delete audioOutput;
    return 0;
}


/*
 * function:处理音频解码后数据
 * @param:data  pcm数据
 * @param:size  pcm大小
 */
void VideoWidget::DealemitAudioData(char *data, int size)
{
    if(!audioDeviceOk)
        return;
    audioDevice->write(data, size);
}

/*
 * function:处理音频设备状态变化
 */
void VideoWidget::DealAudioStateChanged(QAudio::State state)
{
    switch (state)
    {
        //for finished pcm data
        case QAudio::IdleState:
            break;
        //for other reasons
        case QAudio::StoppedState:
            if(audioOutput->error() != QAudio::NoError)
            {
                qDebug()<<"audio others error";
            }
            else
            {
                qDebug()<<"audio error";
            }
            break;
        default:
            break;
    }
}


/*
 * function:实现视频文件的倍速播放
 */
void VideoWidget::Dealmp4FilePlaySpeek(int index)
{
    if(!pVideoThread->mp4FileIsPlay)
        return;
    TimerMp4FilePlay->stop();
    switch(index)
    {
        case 0:
            pVideoThread->douSpeek = 1.0;         //30zhen
            mp4_playtimes = 1000;
            break;
        case 1:
            pVideoThread->douSpeek = 1.5;         //45zhen
            mp4_playtimes = 667;
            break;
        case 2:
            pVideoThread->douSpeek = 2;           //60zhen
            mp4_playtimes = 500;
            break;
    }
    TimerMp4FilePlay->start(mp4_playtimes);
    pVideoThread->mp4FileDouSpeek = true;
}


/*
 * function:点击视频回放模块，进入播放列表操作视频文件
 */
void VideoWidget::DealPlayListMouseEnter(QListWidgetItem *item)
{
    //qDebug()<<"hello";
}















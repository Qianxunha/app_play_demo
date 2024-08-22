#ifndef  VIDEOWIDGET_H
#define  VIDEOWIDGET_H



#include "videoThread.h"

#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QAudioOutput>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>


#define ATTRIB_VERTEX  0     //openGL
#define ATTRIB_TEXTURE 1

class VideoWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:

    VideoThread *pVideoThread;

    CustomPushButton*maxBtn;           // 最大化与最小化
    CustomPushButton*minBtn;
    CustomPushButton*closeBtn;         // 返回主窗口按钮
    QPushButton     *closeBtn1;        // 返回主窗口按钮

    QBoxLayout      *playLayout;
    QBoxLayout      *titleLayout;
    QBoxLayout      *rightBarLayout;
    QBoxLayout      *controlLayout;
    QBoxLayout      *playControlLayout;
    QBoxLayout      *playAndListLayout;
    QBoxLayout      *displayLayout;

    QWidget         *titleWidget;
    QWidget         *playControlWidget;
    QWidget         *rightBarWidget;

    void initAudioDeviceOk();                //初始化音频设备
    bool isAudioDeviceOk();                  //检查是否有音频设备
    bool audioDeviceHaveFlag = true;         //音频设备已插入
    bool audioDeviceFlag     = true;         //音频设备拔出

private:
    bool hwDecodeFlag;                  // 用于是否硬件解码
    bool loginSuccessFlag  = false;     // 登录成功标志位
    bool audioDeviceOk     = false;     // add audio

    int imgWidth;                       // 视频缩略图图片宽度
    int imgHeight;                      // 视频缩略图图片高度
    int photoNum     = 0;                    // 视频回放，视频播放截屏的秒数
    int audioCurValue= 0;                    // 获取音频当前的值
    QString urlVideo = nullptr;              // 视频文件路径名
    QString mp4File_timesStr = nullptr;      // mp4文件总时间字符串
    QString videoDirPath = nullptr;          // 视频文件夹路径

    int mp4File_totalTimes = 0;              // mp4文件总时间
    int mp4File_playTimes  = 0;              // mp4文件播放时间统计
    int mp4_playtimes      = 1000;           // 播放时间 定时器
    int mp4_sampleRate     = 48000;          // 音频采样率
    int mp4_sampleSize     = 8;              // 音频采样格式
    int mp4_channelCount   = 2;              // 音频采样通道

    int listItemRow = 0;                     // 记录当前播放视频文件的行号

    int videoCount = 0;                      // 视频数量
    int pageCount= 0;
    int pageNum  = 2;                        // 设置每页数量
    int pageIndex= 0;                        // 每页页数

    QWidget     *vPageWidget;                // 视频分页容器
    QHBoxLayout *vHBoxLayout;                // 水平分布
    QLabel      *vLabel;                     // 视频分页标签
    CustomPushButton *buttonPrev;            // 视频分页 上一页
    CustomPushButton *buttonNext;            // 视频分页 下一页

    QLabel* durationLabel;              // 播放时间显示标签 02:30/10:00
    QLabel* listTitleLabel;             // 播放列表头标签
    QLabel* labelVideoName;             // 窗口全屏时 显示文件播放名

    QListWidget     *playListWidget;    // 视频列表
    QMenu           *listMenu;          // 选中视频右击显示菜单栏
    QAction         *openAction;        // 添加视频文件
    QAction         *deleteAction;      // 删除视频文件

    CustomQSlider *videoSlider;         // 视频进度条
    CustomQSlider *audioSlider;         // 音量进度条

    CustomPushButton *openFileBtn;      // 打开文件
    CustomPushButton *previousBtn;      // 上一个
    CustomPushButton *nextBtn;          // 下一个
    CustomPushButton *stopBtn;          // 停止播放
    CustomPushButton *playBtn;          // 播放暂停
    CustomPushButton *voiceBtn;         // 音量
    CustomPushButton *photoBtn;         // 拍照
    CustomPushButton *videoMaxBtn;      // 全屏

    QComboBox *speekQCombox;            // 播放倍速框

    //QTimer *TimerMp4FileCheck;        // 进入视频回放模块 定时检测视频文件
    QTimer *TimerMp4FilePlay;           // 播放视频文件，定时器秒数

    QIODevice    *audioDevice;          // 音频设备
    QAudioOutput *audioOutput;          // 音频输出

    QString strImg_min;
    QString strImg_max;
    QString strImg_close;
    QString strImg_openFile;
    QString strImg_previous;
    QString strImg_next;
    QString strImg_stop;
    QString strImg_play;
    QString strImg_voice;
    QString strImg_photo;
    QString strImg_videoMax;

    int m_width;                        // 视频帧宽度
    int m_height;                       // 视频帧高度
    int m_type;                         // 类型

    uint8_t*m_py  = nullptr;            // nv12 y分量   yuv420p y分量
    uint8_t*m_puv = nullptr;            // nv12 uv分量
    uint8_t*m_pu = nullptr;             // yuv420p u分量
    uint8_t*m_pv = nullptr;             // yuv420p v分量

    //shader程序
    QOpenGLShaderProgram program, m_program;

    //nv12格式的
    GLuint idY,idUV;
    QOpenGLBuffer vbo;

    //yuv420p格式的
    GLuint m_textureUniformY, m_textureUniformU , m_textureUniformV;
    //创建纹理
    GLuint m_idy , m_idu , m_idv;

    //nv12显示
    void initialize_Nv12();
    void render(uchar* m_py,uchar* m_pu,int width,int height, int m_type);

    //yuv420p显示
    void initialize_yuv420p();
    void render(uchar* py,uchar* pu,uchar* pv,int width,int height,int type);

    //视频文件播放时间计算
    void getPlayVideoTimes(int num, QString &str);

public:
    VideoWidget(int mwidth, int mheight, VideoThread *videoThread, QWidget* parent = nullptr);
    ~VideoWidget();

    void retranslateUI();
    bool exists(QString path, CustomListWidgetItem* item);
    void updateFlagStatus(bool flag);
    void startCheckVideoFile(QString videoDirPath);
    void updateVideoNumber();
    void freeVideoFile();

    //ffmpeg photo 20201019
    int playVideoPhoto(const char *filePath,int index);
    int playVideoSaveJpgImage(AVFrame *frame, int width, int height, int index);

    //ffmpeg photo for first frame
    int getVideoFirstFrame(const char *filePath, const char *jpgPath);
    int playVideoSaveJpgImage(AVFrame *frame, int width, int height, const char *jpgPath);
    int getJpgImageName(QString fileName, QString &jpgPath);

    //audio device
    int initAudioDevice(int sampleRate, int sampleSize, int channelCount);
    int freeAudioDevice();

    //samba protocol play main borad's video
    QString getFileNameSamba(QString filePath);
    void playVideoUsingSamba(QStringList list);
    void deleteVideoUsingSamba();

protected:
    void mousePressEvent(QMouseEvent*event)override;
    void mouseMoveEvent(QMouseEvent*event)override;
    void mouseReleaseEvent(QMouseEvent*event)override;
    void mouseDoubleClickEvent(QMouseEvent *event)override;

    void changeEvent(QEvent* e)override;
    void closeEvent(QCloseEvent *event) override;

    //openGL
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;

signals:
    void emitOpenVideoSignals();
    void emitGetVideoImage(int index);
    void emitGetFirstFrameImage(const QString mp4FileName);
    void emitDoubleClick(QMouseEvent*event);
    void emitVideoWidgetMax();
    void emitVideoWidgetMaxMain(int index);
    void emitComputerSupportOpenGL(bool flag);

    /*
     * errorIndx = 0 yuv渲染错误
     * errorIndx = 1 nv12渲染
     */
    void emitOpenglError(int errorIndx);

    //mp4
    void emitGetMp4FilePath(QString mp4filePath);
    void emitYuv420pData(uint8_t *m_py, uint8_t*m_pu,uint8_t*m_pv, int width, int height, int hw_type);


public slots:

    void playVideo(QListWidgetItem *item);      // 播放视频文件

    //void on_buttonPrev_pressed();               // 视频列表分页
    //void on_buttonNext_pressed();               // 视频列表分页

    void listWidgetMenu();
    void on_addAction_triggered();
    void on_deleteAction_triggered();

    void on_openFileBtn_clicked();

    void on_stopBtn_clicked();

    void on_previousBtn_clicked();

    void on_playBtn_clicked();

    void on_nextBtn_clicked();

    void on_voiceBtn_clicked();

    void on_photoBtn_clicked();

    void on_videoMaxBtn_clicked();

    void on_minBtn_clicked();

    void on_maxBtn_clicked();

    void on_closeBtn_clicked();

    void on_closeBtn1_clicked();

    void setAudioValue(int value);              // 设置音量进度条

    void setVideoValue(int value);              // 设置视频进度条

    // ffmpeg decode mp4 file
    void recvNv12pDataShow(uint8_t* m_py, uint8_t* m_puv, int width, int height, int hw_type);

    void recvYuv420pDataShow(uint8_t* m_py, uint8_t *m_pu, uint8_t*m_pv, int width, int height, int hw_type);

    void DealemitOpenFileButton(int index);

    void DealemitGetMp4FileTime(int times);

    void DealMp4FilePlayTimer();

    void DealemitMp4FileError(int index);

    void DealemitStopPlayTimes();

    void DealemitGetPlayStatus(int index);

    void DealemitGetStopStatus(int index);

    void DealVideoWidgetMaxMain(int index);

    void DealvideoMenu(bool flag);              //处理视频播放控制菜单栏

    // aduio add
    void DealemitAudioData(char *data, int size);

    void DealemitAudioFormat(int sampleRate, int sampleSize, int channelCount);

    void DealAudioStateChanged(QAudio::State state);

    void Dealmp4FilePlaySpeek(int index);

    //play listwidget
    void DealPlayListMouseEnter(QListWidgetItem *item);

};

#endif

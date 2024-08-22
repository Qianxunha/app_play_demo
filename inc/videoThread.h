#ifndef  RECORDVIDEO_H
#define  RECORDVIDEO_H

#include "appconfig.h"

class VideoThread : public QObject
{
    Q_OBJECT
public:
    QThread workThread;

    //mp4
    bool    mp4FileIsPlay;          //mp4文件是否播放
    bool    mp4FileIsPause;         //mp4文件是否暂停
    bool    mp4FilePreviousPlay;    //mp4文件上一首
    bool    mp4FileNextPlay;        //mp4文件下一首
    bool    mp4FileIsStop;          //mp4文件暂停
    bool    mp4FileSeek;            //mp4文件跳转
    bool    mp4FileDouSpeek;        //mp4文件倍速播放
    bool    hwDecodeFlag;           //windows 硬解码标志位

    int     posSeek;                //mp4文件跳转位置
    double  douSpeek;               //倍速播放
    double  audioSeek;              //音频跳转


private:
    int mp4Width = 0;
    int mp4Height = 0;
    int videoIndex = -1;
    int audioIndex = -1;
    int video_dst_linesize[4];

    // 患者图片宽高数据
    int imgWidth  = 240;    // 图片缩略图宽
    int imgHeight = 135;    // 图片缩略图高
    int itemWidth = 260;    // item宽
    int itemHeight= 170;    // item高

    enum AVPixelFormat mp4PixFmt = AV_PIX_FMT_NONE;
    enum AVPixelFormat yuv_pixfmt;
    AVFormatContext*mp4FmtCtx = NULL;
    AVCodecContext *video_dec_ctx = NULL;
    AVCodecContext *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL;
    AVStream *audio_stream = NULL;

    AVFrame *audioframe = NULL;
    AVFrame *videoframe = NULL;
    AVFrame *tmpframe = NULL;
    AVFrame *swFrame  = NULL;



public:
    VideoThread(bool flag = false);
    ~VideoThread();

    // ffmpeg mp4 file decode
    int readMp4FileDecode (const char *mp4FilePath);
    int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt,AVFrame* frame);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame* yuvframe, SwsContext * swsContext);


signals:

    //ffmpeg mp4 file decode
    void emitGetMp4FilePath(QString mp4filePath);
    void emitGetMp4FileTime(int times);
    void emitMP4Yuv420pData(uint8_t *m_py, uint8_t*m_pu,uint8_t*m_pv, int width, int height, int hw_type);
    void emitMP4Nv12Data(uint8_t *m_py, uint8_t*mp_uv, int width, int height, int hw_type);

    void emitOpenFileButton(int index);
    void emitStopPlayTimes();
    void emitGetPlayStatus(int index);
    void emitGetStopStatus(int index);
    void emitGetFirstFrame(const QString mp4FileName);

    /*
     * index = 0  mp4 file is null
     * index = 1  mp4 file open failed
     * index = 2  mp4 file decode failed
     * index = 3  opengl not supprot
     */
    void emitMp4FileError(int index);

    //ffmpeg audio add
    void emitAudioData(char *data, int size);
    void emitGetAudioFormat(int sampleRate, int sampleSize, int channelCount);


public slots:

    //mp4
    void DealMp4FileDecode(QString mp4FilePath);

};

#endif


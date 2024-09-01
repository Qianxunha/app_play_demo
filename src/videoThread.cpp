#include "videoThread.h"

// ffmpeg + qsv hw decode
DecodeContext videoHwDevice = { NULL };
static AVPixelFormat get_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts);


// ffmpeg + dxva2 hw decode xx
static AVBufferRef *hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;


static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);

// FFmpeg + qsv hw decode
static AVPixelFormat get_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
    while (*pix_fmts != AV_PIX_FMT_NONE)
    {
        if (*pix_fmts == AV_PIX_FMT_QSV)
        {
            DecodeContext *decode = (DecodeContext *)avctx->opaque;
            AVHWFramesContext  *frames_ctx;
            AVQSVFramesContext *frames_hwctx;
            int ret;

            /* create a pool of surfaces to be used by the decoder */
            avctx->hw_frames_ctx = av_hwframe_ctx_alloc(decode->hw_device_ref);
            if(!avctx->hw_frames_ctx)
                return AV_PIX_FMT_NONE;
            frames_ctx   = (AVHWFramesContext*)avctx->hw_frames_ctx->data;
            frames_hwctx = (AVQSVFramesContext*)frames_ctx->hwctx;

            frames_ctx->format            = AV_PIX_FMT_QSV;
            frames_ctx->sw_format         = avctx->sw_pix_fmt;
            frames_ctx->width             = FFALIGN(avctx->coded_width,  32);
            frames_ctx->height            = FFALIGN(avctx->coded_height, 32);
            frames_ctx->initial_pool_size = 32;

            frames_hwctx->frame_type = MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET;

            ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
            if(ret < 0)
                return AV_PIX_FMT_NONE;

            return AV_PIX_FMT_QSV;
        }
        pix_fmts++;
    }
    fprintf(stderr, "The QSV pixel format not offered in get_format()\n");
    return AV_PIX_FMT_NONE;
}


// ffmpeg+ dxva2 hw decode
static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
        NULL, NULL, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
    const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++)
    {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}



/*
 * function: 构造函数
 */
VideoThread::VideoThread(bool flag)
{
    this->hwDecodeFlag = flag;
    this->moveToThread(&workThread);
    workThread.start();

    mp4FileIsPlay       = false;
    mp4FileIsPause      = true;
    mp4FilePreviousPlay = false;
    mp4FileNextPlay     = false;
    mp4FileIsStop       = false;
    mp4FileSeek         = false;
    mp4FileDouSpeek     = false;

    douSpeek            = 0;
    connect(this,   &VideoThread::emitGetMp4FilePath,       this,   &VideoThread::DealMp4FileDecode);
}

/*
 * function: 析构函数
 */
VideoThread::~VideoThread()
{
    workThread.quit();
    workThread.exit();
}




/*** ffmpeg mp4 decode  start*******************************************************/

/*
 * function:解码
 * @param: dec  编码信息上下文
 * @param: pkt  读取mp4文件视频流
 * @param: yuvframe 解码后数据 格式转换 yuvj420 ----> yuv420p
 * @param: swsContext 格式转换
 * @return: 正常执行返回0
 */
int VideoThread::decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame* yuvframe, SwsContext * swsContext)
{
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        return -1;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec, yuvframe);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;
            //qDebug()<<"record video avcodec_receive_frame failed";
            return -1;
        }

        AVFrame *tmpFrame = NULL;
        if(yuv_pixfmt == AV_PIX_FMT_NV12)
        {
            if(av_hwframe_transfer_data(swFrame, yuvframe, 0)<0)
                tmpFrame = yuvframe;
            else
                tmpFrame = swFrame;
            emit emitMP4Nv12Data(tmpFrame->data[0], tmpFrame->data[1], tmpFrame->width, tmpFrame->height, 1);
        }
        else
        {
            if(swsContext!=NULL)
                sws_scale(swsContext,(const uint8_t *const *)yuvframe->data, yuvframe->linesize, 0,yuvframe->height, tmpframe->data, tmpframe->linesize);
            emit emitMP4Yuv420pData(tmpframe->data[0], tmpframe->data[1], tmpframe->data[2], tmpframe->width, tmpframe->height, 2);
        }
    }
    return 0;
}

int VideoThread::decode_packet(AVCodecContext *dec, const AVPacket *pkt,AVFrame* frame)
{
    int got_frame;
    int ret = avcodec_decode_audio4(dec, frame, &got_frame, pkt);
    if (ret < 0) {

        return ret;
    }
    if(got_frame)
    {
        size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((enum AVSampleFormat)frame->format);

        emit emitAudioData((char *)frame->extended_data[0], (int)unpadded_linesize);
    }
    return 0;
}

/*
 * function: 打开解码器
 * @param stream_idx  是音频流/视频流
 * @param dec_ctx    编解码上下文
 * @param fmt_ctx    mp4文件信息
 * @param type       音频流/视频流
 *
 * @return:  正常执行返回0
 */
int VideoThread::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        //windows: qsv hw decode video
        if(hwDecodeFlag && type== AVMEDIA_TYPE_VIDEO)
        {
            HwDecodeVideo hwDecode;
            hwDecode.getHWdecode(hwDecode.hwList);

            for(int i=0; i<hwDecode.hwList.count(); i++)
            {
                if(hwDecode.hwList.at(i) == "qsv")
                {
                    if(av_hwdevice_ctx_create(&videoHwDevice.hw_device_ref, AV_HWDEVICE_TYPE_QSV, "auto", NULL, 0)<0)
                    {
                        continue;
                    }

                    if(st->codecpar->codec_id == AV_CODEC_ID_HEVC)
                    {
                        dec = avcodec_find_decoder_by_name("hevc_qsv");
                    }
                    else if(st->codecpar->codec_id == AV_CODEC_ID_H264)
                    {
                        dec = avcodec_find_decoder_by_name("h264_qsv");
                    }

                    *dec_ctx = avcodec_alloc_context3(dec);
                    avcodec_parameters_to_context(*dec_ctx, st->codecpar);

                    (*dec_ctx)->opaque      = &videoHwDevice;
                    (*dec_ctx)->get_format  = get_format;
                    (*dec_ctx)->thread_count = 4;
                    (*dec_ctx)->thread_safe_callbacks = 1;
                    break;
                }
                else if(hwDecode.hwList.at(i) == "d3d11va" || hwDecode.hwList.at(i) == "dxva2" || hwDecode.hwList.at(i) == "cuda")
                {
                    enum AVHWDeviceType type;
                    type = av_hwdevice_find_type_by_name(hwDecode.hwList.at(i).toStdString().c_str());
                    if (type == AV_HWDEVICE_TYPE_NONE)
                    {
                        continue;
                    }

                    dec = avcodec_find_decoder(st->codecpar->codec_id);
                    for (int i = 0;; i++)
                    {
                        const AVCodecHWConfig *config = avcodec_get_hw_config(dec, i);
                        if (!config)
                        {
                            fprintf(stderr, "Decoder %s does not support device type %s.\n",
                                    dec->name, av_hwdevice_get_type_name(type));
                            return -1;
                        }
                        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                            config->device_type == type) {
                            hw_pix_fmt = config->pix_fmt;
                            break;
                        }
                    }
                    *dec_ctx = avcodec_alloc_context3(dec);
                    avcodec_parameters_to_context(*dec_ctx, st->codecpar);

                    //回调硬解码函数
                    (*dec_ctx)->get_format = get_hw_format;
                    if(hw_decoder_init(*dec_ctx, type) < 0)
                        continue;
                    break;
                }
            }
            yuv_pixfmt  = AV_PIX_FMT_NV12;
            if(avcodec_open2(*dec_ctx, dec, NULL )<0)
            {
                qDebug()<<"nv12 avcodec open failed";
                return -1;
            }
        }
        else
        {
            /* find decoder for the stream */
            if(type == AVMEDIA_TYPE_VIDEO)
                yuv_pixfmt  = AV_PIX_FMT_YUV420P;
            dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec)
            {
                fprintf(stderr, "Failed to find %s codec\n",av_get_media_type_string(type));
                return AVERROR(EINVAL);
            }

            /* Allocate a codec context for the decoder */
            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx)
            {
                fprintf(stderr, "Failed to allocate the %s codec context\n",av_get_media_type_string(type));
                return AVERROR(ENOMEM);
            }

            //开启四线程解码
            (*dec_ctx)->thread_count = 4;
            (*dec_ctx)->thread_safe_callbacks = 1;


            /* Copy codec parameters from input stream to output codec context */
            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
            {
                fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                        av_get_media_type_string(type));
                return ret;
            }

            /* Init the decoders */
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
            {
                fprintf(stderr, "Failed to open %s codec\n",
                        av_get_media_type_string(type));
                return ret;
            }
        }
        *stream_idx = stream_index;
    }
    return 0;
}

/*
 * function: 音频流获取信息
 * @param fmt
 * @param sample_fmt
 * @return 正常执行返回0
 */
int VideoThread:: get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,"sample format %s is not supported as output format\n", av_get_sample_fmt_name(sample_fmt));
    return -1;
}

int align(int x, int a)
{
    return (x + a - 1) & ~(a - 1);
}

/*
 * function:读取mp4文件解码
 * @param: src_filename  mp4文件路径
 * @return :解码成功返回0
 */
int VideoThread:: readMp4FileDecode (const char *mp4FilePath)
{
    SwsContext * swsContext = NULL;
    uint8_t  *out_buffer = NULL;
    qint64 TimeStartDecode = 0;   //记录解码开始时间
    qint64 TimeEndDecode   = 0;   //记录解码完成时间
    qint64 TimeFrame = 0;         //解一帧需要多长时间
    double fps       = 0;         //帧率
    int ret = 0;

    AVPacket pkt;

    if(mp4FilePath== NULL)
    {
        emit emitMp4FileError(0);
        return -1;
    }

    /* open input file, and allocate format context */
    //"rtmp://192.168.1.126:1935/live/livestream"
    //"rtsp://192.168.1.126/live/livestream"
    //"http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8" HLS协议
    //"http://www.w3school.com.cn/i/movie.mp4" MP4文件公网
    qDebug()<<"mp4file="<<mp4FilePath;
    if (avformat_open_input(&mp4FmtCtx, mp4FilePath, NULL, NULL) < 0)
    {
        emit emitMp4FileError(0);
        return -1;
    }
    qDebug()<<"----------------------5";
    //获取mp4文件总秒数
    mp4FileIsPlay = true;
    emit emitOpenFileButton(0);
    //emit emitGetMp4FileTime(mp4FmtCtx->duration/1000000);
    emit emitGetPlayStatus(1);
    emit emitGetStopStatus(1);
    qDebug()<<"----------------------7";
    /* retrieve stream information */
    if (avformat_find_stream_info(mp4FmtCtx, NULL) < 0)
    {
        emit emitMp4FileError(2);
        return -1;
    }
    qDebug()<<"----------------------5";
    if(open_codec_context(&videoIndex, &video_dec_ctx, mp4FmtCtx, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        qDebug()<<"----------------------4";
        video_stream = mp4FmtCtx->streams[videoIndex];

        /* allocate image where the decoded image will be put */
        mp4Width   = video_dec_ctx->width;
        mp4Height  = video_dec_ctx->height;
        mp4PixFmt  = video_dec_ctx->pix_fmt;

        int align_width = 32; // 对齐宽度
        int align_height = 16; // 对齐高度

        int stride_y = align(mp4Width, align_width);
        int stride_uv= align(stride_y/2, align_width)*2;

        int size_y = align(mp4Height,   align_height);
        int size_uv= align(mp4Height/2, align_height);

        mp4Width = stride_y;
        mp4Height= size_y;
        qDebug()<<"----------------------3";
        /* 图像帧率 */
        AVRational frame_rate = av_guess_frame_rate(mp4FmtCtx, video_stream, NULL);
        fps = frame_rate.num/frame_rate.den;
        TimeFrame = 1000.0/fps;
    }
    else
    {
        goto finish;
    }

    // 如果mp4文件含有音频和视频两个通道 则需要加载音频解码器
    if(mp4FmtCtx->nb_streams >=2)
    {
        if (open_codec_context(&audioIndex, &audio_dec_ctx, mp4FmtCtx, AVMEDIA_TYPE_AUDIO) >= 0)
        {
            audio_stream = mp4FmtCtx->streams[audioIndex];
            /*
            qDebug()<<audio_stream->codec->sample_rate<<"音频采样率 16000";
            qDebug()<<audio_stream->codec->sample_fmt<<"采样格式 枚举变量 8";
            qDebug()<<audio_stream->codec->frame_size<<"音频帧编码采样个数 1024";
            qDebug()<<audio_stream->codec->channels<<"音频采样通道号 1";
            */
            emit emitGetAudioFormat(audio_stream->codec->sample_rate, audio_stream->codec->sample_fmt, audio_stream->codec->channels);
        }
        else
        {
            goto finish;
        }
    }
    qDebug()<<"----------------------1";
    audioframe = av_frame_alloc();
    videoframe = av_frame_alloc();
    swFrame    = av_frame_alloc();

    /* 用于缩放函数, 缩放分辨率比例不一样的情况*/
    tmpframe = av_frame_alloc();
    tmpframe->width = mp4Width;
    tmpframe->height= mp4Height;
    qDebug()<<"----------------------2";
    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(yuv_pixfmt, mp4Width, mp4Height));
    avpicture_fill((AVPicture *)tmpframe, out_buffer, yuv_pixfmt, tmpframe->width, tmpframe->height);
    swsContext = sws_getContext(video_dec_ctx->width, video_dec_ctx->height,  mp4PixFmt,
                                             tmpframe->width, tmpframe->height,  yuv_pixfmt, SWS_BILINEAR,NULL,NULL,NULL);

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    qDebug()<<"----------------------2";
    /* read frames from the file */
    while (1)
    {
        //暂停
        if(mp4FileIsPause)
        {
            TimeStartDecode = QDateTime::currentDateTime().toMSecsSinceEpoch();
            ret = av_read_frame(mp4FmtCtx, &pkt);

            /*
             * 1.视频回放播放界面进度条跳转
             * 2.int av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp, int flags);
             * 3.2021/01/19;
             */
            if(mp4FileSeek)
            {
                mp4FileSeek = !mp4FileSeek;
                if(pkt.stream_index == videoIndex)
                {
                    int64_t videoStamp = posSeek/ av_q2d(mp4FmtCtx->streams[videoIndex]->time_base);
                    if(av_seek_frame(mp4FmtCtx, videoIndex, videoStamp, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD )<0)
                    {
                        return -1;
                    }
                }

                if(pkt.stream_index == audioIndex)
                {
                    int64_t audioStamp = posSeek/ av_q2d(mp4FmtCtx->streams[audioIndex]->time_base);

                    if(av_seek_frame(mp4FmtCtx,audioIndex, audioStamp, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD)<0)
                    {
                        return -1;
                    }
                }
                av_packet_unref(&pkt);
                continue;
            }

            /*
             * 1. mp4文件实现倍速播放
             */
            if(mp4FileDouSpeek)
            {
                fps = video_stream->avg_frame_rate.num*douSpeek/video_stream->avg_frame_rate.den;
                mp4FileDouSpeek = !mp4FileDouSpeek;
            }

            if(pkt.stream_index == videoIndex)
            {
                if(decode_packet(video_dec_ctx, &pkt, videoframe, swsContext)<0)
                    continue;

                TimeEndDecode = QDateTime::currentDateTime().toMSecsSinceEpoch();
                if((TimeEndDecode - TimeStartDecode) < TimeFrame)
                {
                    av_usleep(1000*(TimeFrame-(TimeEndDecode - TimeStartDecode)));
                }
            }
            else if(pkt.stream_index == audioIndex)
            {
                if(decode_packet(audio_dec_ctx, &pkt, audioframe)<0)
                    continue;
            }

            av_packet_unref(&pkt);
            if (ret < 0)
            {
                break;
            }
        }

        //上一首
        if(mp4FilePreviousPlay)
            break;
        //下一首
        if(mp4FileNextPlay)
            break;
        //停止
        if(mp4FileIsStop)
            break;
    }

finish:
    if(hwDecodeFlag)
        emit emitMP4Nv12Data(nullptr, nullptr, 0, 0, 1);
    else
        emit emitMP4Yuv420pData(nullptr, nullptr, nullptr, 0, 0, 2);
    emit emitOpenFileButton(1);
    emit emitStopPlayTimes();
    emit emitGetPlayStatus(0);
    emit emitGetStopStatus(0);
    QThread::msleep(600);

    mp4FileIsPlay       = false;
    mp4FileIsPause      = true;
    mp4FilePreviousPlay = false;
    mp4FileNextPlay     = false;
    mp4FileIsStop       = false;

    if(video_dec_ctx)
        avcodec_free_context(&video_dec_ctx);
    if(audio_dec_ctx)
        avcodec_free_context(&audio_dec_ctx);
    if(mp4FmtCtx)
        avformat_close_input(&mp4FmtCtx);
    if(swsContext)
    {
        sws_freeContext(swsContext);
        swsContext = NULL;
    }

    if(out_buffer)
    {
        av_free(out_buffer);
        out_buffer = NULL;
    }

    if(audioframe)
        av_frame_free(&audioframe);
    if(videoframe)
        av_frame_free(&videoframe);
    if(tmpframe)
        av_frame_free(&tmpframe);
    if(swFrame)
        av_frame_free(&swFrame);
    return 0;
}


/*
 * function: 解码mp4文件解码
 * @param : mp4FilePath mp4文件的路径
 * @return:
 */
void VideoThread::DealMp4FileDecode(QString mp4FilePath)
{

#if OS_WINDOWS_QT
    QTextCodec *code = QTextCodec::codecForName("GB2312");
#elif OS_MAC_QT
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
#endif
    string fileNamePath = code->fromUnicode(mp4FilePath).data();
    readMp4FileDecode(fileNamePath.c_str());
}

/*** ffmpeg mp4 decode  end*******************************************************/

























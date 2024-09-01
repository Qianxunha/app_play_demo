QT       += core gui opengl multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG      += warn_off                 #关闭编译警告提示 眼不见为净
MOC_DIR     = temp/moc                  #指定编译生成的文件到temp目录 分门别类存储
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj
DEFINES     -= UNICODE

#DEFINES += QT_NO_DEBUG_OUTPUT          #禁用qdebug打印输出
DESTDIR     = bin                       #指定编译生成的可执行文件到bin目录
CONFIG      += c++11

#select windows/macos platform
DEFINES +=_QT_APP_PLATFORM
contains(DEFINES, _QT_APP_PLATFORM){
    message("Build windows platform");
}
else {
    message("Build macos platform");
}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/inc \
               $$PWD/inc/ffmpeg

LIBS += -L$$PWD/lib/ffmpeg \
        -lavformat \
        -lavcodec \
        -lavfilter \
        -lavutil \
        -lswresample \
        -lswscale \

SOURCES += \
    main.cpp \
    src/appconfig.cpp \
    src/dialogNetStream.cpp \
    src/videoThread.cpp \
    src/videoWidget.cpp \
    widget.cpp

HEADERS += \
    inc/appconfig.h \
    inc/dialogNetStream.h \
    inc/videoThread.h \
    inc/videoWidget.h \
    widget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    videoImage.qrc

FORMS += \
    widgetUI/DialogNetStream.ui

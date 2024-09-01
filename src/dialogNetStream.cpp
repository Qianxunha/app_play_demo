
#include "dialogNetStream.h"
#include "ui_DialogNetStream.h"

DialogNetStream::DialogNetStream(QWidget *parent): QDialog(parent), ui(new Ui::DialogNetStream)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(600, 400));
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    this->setWindowTitle("打开网络串流");

    connect(ui->pushBtnPlay,        &QPushButton::clicked,  this,   &DialogNetStream::on_pushBtnPlay_pressed1);
}

DialogNetStream::~DialogNetStream()
{

}



void DialogNetStream::on_pushBtnPlay_pressed1()
{
    emit emitPlayNetStream(ui->lineEdit->text());
    this->hide();
}

void DialogNetStream::on_pushBtnCancel_pressed()
{
    this->hide();
}

#ifndef DIALOGNETSTREAM_H
#define DIALOGNETSTREAM_H

#include <QDialog>

namespace  Ui
{
    class DialogNetStream;
}

class DialogNetStream: public QDialog
{
    Q_OBJECT
public:
    explicit DialogNetStream(QWidget *parent = nullptr);
    ~DialogNetStream();

signals:
    void emitPlayNetStream(QString netAddress);

private slots:
    void on_pushBtnPlay_pressed1();

    void on_pushBtnCancel_pressed();

private:
    Ui::DialogNetStream *ui;
};

#endif // DIALOGNETSTREAM_H

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QFileDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include "qdownloadhandler.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;

private slots:
    void btnFirmwareChoseClickedSlot();
    void btnFirmwareDownloadClickedSlot();
    void downlaodError(QString msg);
    void downloadProgress(int value);
    void downlaodInfo(QString msg);

private:

};

#endif // WIDGET_H

#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/logo.ico"));

    QFile file("config.ini");
    if(!file.exists())
    {
        QSettings config("config.ini", QSettings::IniFormat);
        config.setValue("portname", "");
        config.setValue("filepath", "");
    }

    QSettings config("config.ini", QSettings::IniFormat);

    QList<QSerialPortInfo> serials = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo portInfo, serials) {
        ui->cmbSerialChose->addItem(portInfo.portName() + " " + portInfo.description());
    }

    int index = ui->cmbSerialChose->findText(config.value("portname").toString());
    if(index < 0)
    {
        index = 0;
    }
    ui->cmbSerialChose->setCurrentIndex(index);
    ui->lineEdit->setText(config.value("filepath").toString());

    QObject::connect(ui->btnFirmwareChose, SIGNAL(clicked(bool)), SLOT(btnFirmwareChoseClickedSlot()));
    QObject::connect(ui->btnFirmwareDownload, SIGNAL(clicked(bool)), SLOT(btnFirmwareDownloadClickedSlot()));
}

Widget::~Widget()
{
    QSettings config("config.ini", QSettings::IniFormat);
    config.setValue("portname", ui->cmbSerialChose->currentText());
    config.setValue("filepath", ui->lineEdit->text());
    delete ui;
}

void Widget::btnFirmwareChoseClickedSlot()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择固件"),"./", tr("二进制文件 (*.bin *.BIN)"));
    ui->lineEdit->setText(fileName);
}

void Widget::btnFirmwareDownloadClickedSlot()
{
    ui->textBrowser->clear();
    ui->progressBar->setValue(0);

    QThread* thread = new QThread();
    QDownloadHandler* downloader = new QDownloadHandler(ui->cmbSerialChose->currentText().split(" ")[0], ui->lineEdit->text());

    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(thread, SIGNAL(finished()), downloader, SLOT(deleteLater()));

    QObject::connect(thread, SIGNAL(started()), downloader, SLOT(download()));

    QObject::connect(downloader, SIGNAL(error(QString)), this, SLOT(downlaodError(QString)));
    QObject::connect(downloader, SIGNAL(information(QString)), this, SLOT(downlaodInfo(QString)));
    QObject::connect(downloader, SIGNAL(progress(int)), this, SLOT(downloadProgress(int)));

    downloader->moveToThread(thread);

    thread->start();
}

void Widget::downloadProgress(int value)
{
    ui->progressBar->setValue(value);
}

void Widget::downlaodError(QString msg)
{
    QMessageBox::critical(this, "错误提示", msg, "关闭");
}

void Widget::downlaodInfo(QString msg)
{
    ui->textBrowser->append(msg);
}

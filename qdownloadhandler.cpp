#include "qdownloadhandler.h"

QDownloadHandler::QDownloadHandler(QString portName, QString fileName, QObject *parent) : QObject(parent), portName(portName), fileName(fileName)
{
    serial = new QSerialPort(this);
}
QDownloadHandler::~QDownloadHandler()
{
    delete serial;
}

void QDownloadHandler::download()
{
    int ret = 0;
    quint32 address = 0X08000000;
    quint32 nsend = 0;
    QFile file(fileName);

    if(!file.exists())
    {
        emit this->error("固件文件不存在，请重新选择！");
        return;
    }

    // 初始化串口
    serial->setPortName(portName);
    serial->setBaudRate(115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setParity(QSerialPort::EvenParity);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if(!serial->open(QSerialPort::ReadWrite))
    {
        emit this->error("串口打开失败！\r\n请检查该端口是否存在或被占用！");
        return;
    }

    // 设置STM32进入下载模式
    serial->setDataTerminalReady(false);  // 拉低DTR复位
    serial->setRequestToSend(true);       // 拉高RTS选择BOOT0=1
    QThread::usleep(100000);              // 延时等待
    serial->setDataTerminalReady(true);   // 拉高DTR释放复位
    QThread::usleep(100000);              // 延时等待

    // 连接设备
    ret = connectToDevice();
    emit this->information(QString("* 连接设备...%1！").arg(ret == 0 ? "成功" : "失败，请重试"));
    if(ret < 0)
    {
        serial->close();
        return;
    }

    // 擦除全片
    ret = eraseFull();
    emit this->information(QString("* 擦除全片...%1！").arg(ret == 0 ? "成功" : "失败，请重试"));
    if(ret < 0)
    {
        serial->close();
        return;
    }

    // 打开固件文件
    if(!file.open(QFile::ReadOnly))
    {
        emit this->information(QString("* 固件打开失败，请重试！"));
        serial->close();
        return;
    }

    // 发送固件
    emit this->information(QString("* 开始刷写..."));
    while(true)
    {
        QByteArray data = file.read(256);
        if(data.size() == 0)
        {
            break;
        }

        while(true)
        {
            if(writeBlock(address, data.data(), data.size()) == 0)
            {
                address += data.size();
                nsend += data.size();
                emit this->progress((float)nsend / (float)file.size() * 100);
                break;
            }else
            {
                emit this->information(QString("* [0X%1] 写入失败，重试...").arg(address, 8, 16, QChar('0')));
            }
        }
    }

    serial->setDataTerminalReady(false);  // 拉低DTR复位
    serial->setRequestToSend(false);      // 拉低RTS选择BOOT0=0
    QThread::usleep(100000);              // 延时等待
    serial->setDataTerminalReady(true);   // 拉高DTR释放复位
    QThread::usleep(100000);

    file.close();
    serial->close();

    emit this->information(QString("* 刷写完成！"));
}

int QDownloadHandler::write(const void *buf, quint32 len)
{
    int ret = -1;
    serial->clear();
    serial->write((const char *)buf, len);
    if(serial->waitForBytesWritten(1000))
    {
        ret = 0;
    }
    return ret;
}

int QDownloadHandler::read(void *buf, quint32 len)
{
    if(serial->waitForReadyRead(1000))
    {
        return serial->read((char *)buf, len);
    }
    return -1;
}

int QDownloadHandler::gotoRun()
{
    quint32 addr = 0X08000000;
    char buff[16];

    buff[0] = 0X21;
    buff[1] = 0XDE;
    write(buff, 2);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -1;
    }

    buff[0] = (addr >> 24) & 0XFF;
    buff[1] = (addr >> 16) & 0XFF;
    buff[2] = (addr >> 8) & 0XFF;
    buff[3] = (addr >> 0) & 0XFF;
    buff[4] = checksum(buff, 4);
    write(buff, 5);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -2;
    }
    return 0;
}

int QDownloadHandler::connectToDevice()
{
    char chr = 0X7F;
    write(&chr, 1);
    read(&chr, 1);
    if(chr != 0X79)
    {
        return -1;
    }
    return 0;
}

int QDownloadHandler::eraseFull()
{
    char buff[2];

    buff[0] = 0X43;
    buff[1] = 0XBC;
    write(buff, 2);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -1;
    }

    buff[0] = 0XFF;
    buff[1] = 0X00;
    write(buff, 2);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -2;
    }

    return 0;
}

int QDownloadHandler::writeBlock(quint32 addr, const void *buf, quint32 len)
{
    char buff[300];

    // 发送协议头
    buff[0] = 0X31;
    buff[1] = 0XCE;
    write(buff, 2);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -1;
    }

    // 发送地址
    buff[0] = (addr >> 24) & 0XFF;
    buff[1] = (addr >> 16) & 0XFF;
    buff[2] = (addr >> 8) & 0XFF;
    buff[3] = (addr >> 0) & 0XFF;
    buff[4] = checksum(buff, 4);
    write(buff, 5);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -2;
    }

    // 发送数据
    buff[0] = len - 1;
    memcpy(buff + 1, buf, len);
    buff[len + 1] = checksum(buff, len + 1);
    write(buff, len + 2);
    read(buff, 1);
    if(buff[0] != 0X79)
    {
        return -3;
    }

    return 0;
}

quint8 QDownloadHandler::checksum(const void *buf, quint32 len)
{
    const quint8* p = (const quint8 *)buf;
    quint8 checkval = 0;
    while(len--)
    {
        checkval ^= *p++;
    }
    return checkval;
}

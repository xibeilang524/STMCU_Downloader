#ifndef QDOWNLOADHANDLER_H
#define QDOWNLOADHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QFile>
#include <QDebug>

class QDownloadHandler : public QObject
{
    Q_OBJECT
public:
    explicit QDownloadHandler(QString portName, QString fileName, QObject *parent = nullptr);
    virtual ~QDownloadHandler();

signals:
    void error(QString msg);
    void information(QString msg);
    void progress(int value);

public slots:
    void download();

private:
    QSerialPort* serial;
    QString portName;
    QString fileName;

private:
    int write(const void* buf, quint32 len);
    int read(void* buf, quint32 len);
    bool waitResponse(void);
    quint8 checksum(const void* buf, quint32 len);
    int writeBlock(quint32 addr, const void* buf, quint32 len);
    int eraseFull(void);
    int connectToDevice(void);
    int gotoRun(void);
};

#endif // QDOWNLOADHANDLER_H

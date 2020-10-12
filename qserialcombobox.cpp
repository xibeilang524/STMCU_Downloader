#include "qserialcombobox.h"
#include <QSerialPortInfo>

QSerialComboBox::QSerialComboBox(QWidget* parent) : QComboBox(parent)
{

}

QSerialComboBox::~QSerialComboBox()
{

}

void QSerialComboBox::showPopup()
{
    int index = 0;
    QString text = currentText();

    clear();

    QList<QSerialPortInfo> serials = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo port, serials) {
        addItem(port.portName() + " " + port.description());
    }

    index = findText(text);
    if(index < 0)
    {
        index = 0;
    }

    setCurrentIndex(index);
    QComboBox::showPopup();
}

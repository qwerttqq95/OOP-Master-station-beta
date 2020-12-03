#include <WinSock2.h>
#include <QDialog>
#include <QSerialPort>
#include <process.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include "tinyxml2.h"
#include <stdlib.h>
#include <stdio.h>
#include <QString>

namespace Ui
{
    class Dialog;
}

class Serial : public QDialog
{
    Q_OBJECT
public:
    bool run_flag;
    bool internet_or_serial;
    SOCKET sockConn;
    SOCKET sockSrv;
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *root;
    QString temp;

    explicit Serial(QWidget *parent = nullptr);

    ~Serial() override;

    bool open_serial(const QString);

    BOOL CloseSerial();

    bool build_net(int com);

    QSerialPort *m_serialPort;

private:
    Ui::Dialog *ui;
    QSerialPort *m_serial = nullptr;
    QByteArray data = "";

public slots:

    bool write(const QList<QString>);

    void readData();

    void creat_process();

    void warming();

signals:

    void send_message(QList<QString>);

    void receive_message(QList<QString>);

    void open_fail_message();

    void send_write(QList<QString>);
};

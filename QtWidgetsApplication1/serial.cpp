#include "serial.h"
#include "ui_serial.h"
#include <thread>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QSerialPortInfo>

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

using namespace std;
#pragma comment(lib, "Ws2_32.lib")

extern int Stringlist2Hex_char(QString &str, char *pOut);

extern int String2Hex(string &str, BYTE *pOut, DWORD *pLenOut, DWORD nMaxLen);

extern string StringAddSpace(string &input);

extern QString StringAddSpace(QString &input);

extern int check(QString);
char convertCharToHex(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - 0x30;
    else if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;
    else if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    else
        return (-1);
}
void convertStringToHex(const QString &str, QByteArray &byteData)
{
    int hexdata, lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    byteData.resize(len / 2);
    char lstr, hstr;
    for (int i = 0; i < len;)
    {
        //char lstr,
        hstr = str[i].toLatin1();
        if (hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if (i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = convertCharToHex(hstr);
        lowhexdata = convertCharToHex(lstr);
        if ((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata * 16 + lowhexdata;
        i++;
        byteData[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    byteData.resize(hexdatalen);
}

Serial::Serial(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog), m_serial(new QSerialPort(this))
{
    ui->setupUi(this);
    setWindowTitle("通讯配置");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::MSWindowsFixedSizeDialogHint);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(creat_process()));
    connect(this, SIGNAL(open_fail_message()), this, SLOT(warming()));
    connect(this, SIGNAL(send_write(QList<QString>)), this, SLOT(write(QList<QString>)));
    connect(m_serial, SIGNAL(readyRead()), this, SLOT(readData()));
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        ui->comboBox->addItem(serial.portName());
    }
    if (doc.LoadFile("./config.xml") != 0)
    {
        qDebug() << "Load config failed";
        return;
    }

    root = doc.RootElement();
    tinyxml2::XMLElement *first_child1 = root->FirstChildElement("port");
    const char *port;
    port = first_child1->GetText();
    QString COM = (QString)port;
    ui->lineEdit->setText(COM);
}

void Serial::creat_process()
{
    if (ui->radioButton->isChecked()) //串口
    {
        if (ui->pushButton->text() == "打开")
        {
            QString str = ui->comboBox->currentText();
            if (!open_serial(str))
            {
                emit open_fail_message();
                return;
            }
            ui->pushButton->setText("关闭");
            ui->comboBox->setDisabled(true);
            ui->comboBox_2->setDisabled(true);
            ui->lineEdit->setDisabled(true);
            ui->radioButton_2->setDisabled(true);
            close();
            internet_or_serial = true;
        }
        else
        {
            ui->pushButton->setText("打开");
            ui->comboBox->setDisabled(false);
            ui->comboBox_2->setDisabled(false);

            ui->lineEdit->setDisabled(false);
            ui->radioButton_2->setDisabled(false);
            CloseSerial();
        }
    }
    else //以太网
    {
        tinyxml2::XMLElement *first_child1 = root->FirstChildElement("port");
        int com = ui->lineEdit->text().toInt();
        first_child1->SetText(com);
        doc.SaveFile("config.xml");
        if (ui->pushButton->text() == "打开")
        {
            qDebug() << "以太网状态开启";
            run_flag = true;
            ui->pushButton->setText("关闭");
            std::thread t2(&Serial::build_net, this, ui->lineEdit->text().toInt());
            t2.detach();
            close();
            internet_or_serial = false;
            ui->comboBox->setDisabled(true);
            ui->comboBox_2->setDisabled(true);
            ui->lineEdit->setDisabled(true);
            ui->radioButton->setDisabled(true);
        }
        else
        {
            qDebug() << "以太网状态关闭";
            ui->pushButton->setText("打开");
            ui->comboBox->setDisabled(false);
            ui->comboBox_2->setDisabled(false);
            ui->lineEdit->setDisabled(false);
            ui->radioButton_2->setDisabled(false);
            ui->radioButton->setDisabled(false);
            CloseSerial();
        }
    }
}

bool Serial::open_serial(const QString com)
{
    m_serial->setPortName(com);
    m_serial->setBaudRate(ui->comboBox_2->currentText().toLong(nullptr));
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::EvenParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite))
    {
        qDebug() << "open serial ok\n";
        write({"6817004345AAAAAAAAAAAA10DA5F0501034001020000900f16", "读地址","send" });
        return true;
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        return false;
    }
}

bool Serial::build_net(int com)
{
    WSADATA wsaData;
    // 加载套接字
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        qDebug() << ("加载套接字失败：%d......\n", WSAGetLastError());
        return 1;
    }
    sockSrv = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化IP和端口信息
    SOCKADDR_IN addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(com); // 1024以上的端口号
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    if (::bind(sockSrv, (LPSOCKADDR)&addrSrv, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        qDebug() << ("套接字绑定失败：%d......\n", WSAGetLastError());
        return 1;
    }
    if (listen(sockSrv, 10) == SOCKET_ERROR)
    {
        qDebug() << ("套接字监听失败：%d......\n", WSAGetLastError());
        return 1;
    }
    SOCKADDR_IN addrClient;
    int len = sizeof(SOCKADDR);
    while (run_flag)
    {
        qDebug() << ("服务端启动成功......开始监听...\n");
        sockConn = ::accept(sockSrv, (SOCKADDR *)&addrClient, &len);
        if (sockConn == SOCKET_ERROR)
        {
            qDebug() << ("建立连接失败：%d......\n", WSAGetLastError());
        }
        qDebug() << ("与客户端建立连接......");
        while (1)
        {
            int re;
            char recvBuf[4000];
            memset(recvBuf, 0, sizeof(recvBuf));
            re = recv(sockConn, recvBuf, sizeof(recvBuf), 0);
            if (!run_flag)
                    break;
            if (re == -1 || re == 0)
            {
                qDebug() << "连接丢失";
                break;
            }
            char a[4] = {0};
            string output = string("");
            for (int x = 0; x < re; x++)
            {
                sprintf(a, "%02X ", (BYTE)recvBuf[x]);
                output = output + a;
            }
            QStringList list = QString::fromStdString(output).split(' ', QString::SkipEmptyParts);
            while (list.begin() != list.end())
            {
                if (list[0] == "68")
                    break;
                else
                    list.removeAt(0);
            }
            int message_len = (list[2] + list[1]).toInt(nullptr, 16);
            qDebug() << "message_len " << message_len;
            qDebug() << "list.size() " << list.size();
            if (list[0] == "68" && list.endsWith("16") && message_len + 2 == list.size())
            {
                auto full = list.join(" ");
                qDebug() << "Receive from client(net) :" << full << endl;
                QList<QString> ss = { (QString)(full) ,0,0 };
                receive_message(ss);
            }
            else if (list[0] == "68")
            {
                qDebug() << "need more" << list;
                list.clear();
            }
        }
    }
    qDebug() << "close net";
    WSACleanup();
}

BOOL Serial::CloseSerial()
{
    qDebug() << "runFlag is False";
    run_flag = false;
    qDebug() << "closesocket: "<<closesocket(sockSrv);
    m_serial->close();
    return true;
}

Serial::~Serial() = default;

void Serial::warming()
{
    QMessageBox::warning(this, "ERROR", "串口打开失败!", QMessageBox::Ok);
    ui->pushButton->setText("打开");
    ui->comboBox->setDisabled(false);
    ui->comboBox_2->setDisabled(false);
}

void Serial::readData()
{
    const QByteArray dataRec = m_serial->readAll();
    data += dataRec;
loop:
    if (data.length() > 20 && data.size() > 0)
    {
        // qDebug() << "data:" << data.toHex(' ');
        if (data.contains(0x68))
        {
            if (data.at(0) != 0x68)
            {
                auto position = data.indexOf(0x68);
                data = data.mid(position);
            }
            if ((data.at(3) & 0x3) == 3)
            {
                auto dataSplit = ((QString)(data.mid(1, 2).toHex(' '))).split(' ');
                // qDebug() << "1:" << dataSplit[0].toInt(nullptr, 16);
                int len = (dataSplit[1].toInt(nullptr, 16) << 8) + dataSplit[0].toInt(nullptr, 16) + 2;
                // qDebug() << "len:" << len;
                if (len <= data.length())
                {
                    if (data.at(len - 1) == 0x16)
                    {
                        qDebug() << "Receive: " << data.left(len).toHex(' ') << endl;
                        QList<QString> ss = { (QString)(data.left(len).toHex(' ')) ,0,0 };
                        emit receive_message(ss); //接收显示
                        //emit receive_message((QString)(data.left(len).toHex(' '))); //接收显示
                        data = data.mid(len);
                        goto loop;
                    }
                    else
                    {
                        data = data.mid(1);
                        goto loop;
                    }
                }
                return;
            }
            else
            {
                data = data.mid(1);
                goto loop;
            }
        }
    }
}

bool Serial::write(QList<QString> add)
{
    if (internet_or_serial)
    {
        QByteArray sendBuf;
        convertStringToHex(add[0], sendBuf);
        qDebug() << "write serial" << add;
        m_serial->write(sendBuf);
        emit send_message(add);
    }
    else
    {
        qDebug() << "write eth" << add;
        int len = add[0].length() / 2 + 1;
        char Apdu[1500] = {0};
        Stringlist2Hex_char(add[0], Apdu);
        string new_add = add[0].toStdString();
        qDebug() << "Eth Send: " << add[0];
        qDebug() << "len: " << len;
        send(sockConn, Apdu, len, 0);
        emit send_message(add);
    }

    return true;
}
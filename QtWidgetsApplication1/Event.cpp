#include "Event.h"
#include "ui_Event.h"


Event::Event(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::EventDialog)
{
    ui->setupUi(this);
    setWindowTitle("批量配置事件");
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(PressOk()));
    setAttribute(Qt::WA_QuitOnClose, false);
}

void Event::PressOk() {
    QString text = ui->textEdit->toPlainText();
    auto text_list = text.split("\n",QString::SplitBehavior::SkipEmptyParts);
    QString output("");
    for each (auto var in text_list)
    {

        QString temp1,temp2;
        auto sp = var.split(",");
        QString name = sp[0];
        name.remove(' ');
        if (sp[1] == "上报")
        {
            temp1 = "01";
        }
        else
        {
            temp1 = "00";
        }
        if (sp[2] == "开启")
        {
            temp2 = "01";
        }
        else
        {
            temp2 = "00";
        }
        output += "<SET" + name + "\tAPDU=\"06 02 06 02 " + sp[0] + " 08 00 16 " + temp1 + " " + sp[0] + " 09 00 03 " + temp2 + " 00\"/>\n";
        
    }
    QTextEdit *op = new QTextEdit();
    op->setWindowTitle("输出");
    op->setPlainText(output);
    op->setMinimumSize(QSize(450,350));
    op->show();
    //qDebug() << output;
}

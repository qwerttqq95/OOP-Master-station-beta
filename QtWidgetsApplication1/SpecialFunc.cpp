//
// Created by admin on 2020/5/28.
//
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "SpecialFunc.h"
#include "string"
#include "QMessageBox"

extern auto BuildMessage(const QString &apdu, const QString &SA, const QString &ctrl_zone) -> QString;

extern QString re_rever_add();

extern std::string ReTime();

extern QString StringAddSpace(QString &);

SpecialFuncs::SpecialFuncs(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::SpecialFunc_ui) {
    ui->setupUi(this);
    setWindowTitle("特殊功能");
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(timing()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(timing_with_local()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(trans()));
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

void SpecialFuncs::timing() {
    auto get_t = ui->dateTimeEdit->dateTime().toString().split(" ");
    auto year = get_t.takeLast(), mouth = get_t.at(1), day = get_t.at(2);
    auto time_ = get_t.takeLast().split(":");
    mouth.remove("月");
    mouth = ReMouth(mouth);
    char temp_year[5] = {0};
    char temp_mouth[3] = {0};
    char temp_day[3] = {0};
    sprintf(temp_year, "%04X", year.toInt());
    sprintf(temp_mouth, "%02X", mouth.toInt());
    sprintf(temp_day, "%02X", day.toInt());
    auto datetime_ = QString(temp_year) + QString(temp_mouth) + QString(temp_day);
    char temp[3] = {0};
    for (int i = 0; i < 3; ++i) {
        sprintf(temp, "%02X", time_.takeFirst().toInt());
        datetime_.append(temp);
    }
    auto apdu = "06 01 05 40 00 02 00 1c" + datetime_ + "00";
    emit send_write(
            {BuildMessage(apdu.remove(" "), re_rever_add(), "43"), "对时","send" });
}

void SpecialFuncs::timing_with_local() {
    std::string a;
    a = ReTime();
    auto now_date = QString::fromStdString(a).split("-");
    auto now_time = QTime::currentTime().toString().split(":");
    char temp_year[5] = {0};
    char temp_mouth[3] = {0};
    char temp_day[3] = {0};

    sprintf(temp_year, "%04X", now_date[0].toInt());
    sprintf(temp_mouth, "%02X", now_date[1].toInt());
    sprintf(temp_day, "%02X", now_date[2].toInt());
    auto datetime_ = QString(temp_year) + QString(temp_mouth) + QString(temp_day);
    char temp[3] = {0};
    for (int i = 0; i < 3; ++i) {
        sprintf(temp, "%02X", now_time.takeFirst().toInt());
        datetime_.append(temp);
    }
    auto apdu = "06 01 05 40 00 02 00 1c" + datetime_ + "00";
    emit send_write(
            {BuildMessage(apdu.remove(" "), re_rever_add(), "43"), "对时","send" });
}

void SpecialFuncs::trans() {
    if (ui->textEdit->toPlainText() != "") {
        auto left_text = StringAddSpace(ui->textEdit->toPlainText().remove(" ")).split(" ", QString::SkipEmptyParts);
        if (left_text[0] == "68" || left_text[0] != "85") {
            QMessageBox::warning(nullptr, "警告", "不是85开头的APDU");
            return;
        }
        if (left_text[1] == "01") {//8501
            left_text[0] = "07";
            left_text[5] = "7f";
            left_text.removeAt(7);
        } else {
            if (left_text[1] == "05") {//8505
                for (int i = 0; i < 5; ++i) {
                    left_text.removeAt(3);
                }
                left_text[0] = "07";
                left_text[1] = "01";
                left_text[5] = "7f";
                left_text.removeAt(7);
            }
        }
        int len = left_text.length();
        QString right("");
        for (int i = 0; i < len; ++i) {
            right.append(left_text[i]);
        }
        ui->textEdit_2->setText(StringAddSpace(right));
        return;
    }

}

QString SpecialFuncs::ReMouth(QString mouth) {
    const QString a[12] = {"一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "十一", "十二"};
    for (int i = 0; i < 12; i++) {
        if (a[i] == mouth){
            return QString::number(i+1);
        }
    }
    return mouth;
}

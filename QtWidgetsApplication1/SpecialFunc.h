//
// Created by admin on 2020/5/28.
//

#ifndef INC_698SP_CL_SPECIALFUNC_H
#define INC_698SP_CL_SPECIALFUNC_H

#include "ui_Special.h"
#include "QDebug"
#include "QDialog"
#pragma execution_character_set("utf-8")
namespace Ui {
    class SpecialFunc_ui;
}

class SpecialFuncs : public QDialog {
Q_OBJECT
public:
    explicit SpecialFuncs(QWidget *parent = nullptr);

public slots:

    void timing();

    void timing_with_local();

    void trans();

    static QString ReMouth(QString);


private:
    Ui::SpecialFunc_ui *ui;

signals:

    void send_write(QList<QString>);

};

#endif //INC_698SP_CL_SPECIALFUNC_H

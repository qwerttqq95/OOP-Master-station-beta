#include "Qt698sp.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
int main(int argc, char *argv[])
{
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	qRegisterMetaType<QList<QString> >("QList<QString>");
    QApplication a(argc, argv);
    Qt698sp w;
    w.setAttribute(Qt::WA_QuitOnClose,true);
    w.show();
    return a.exec();
}

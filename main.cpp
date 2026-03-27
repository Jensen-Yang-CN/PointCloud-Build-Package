#include <QMetaType>
#include "mainwindow.h"


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QHostAddress>("QHostAddress"); // ✅ 必须
    MainWindow w;
    w.show();
    return a.exec();
}

#include "Medusa.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Medusa w;
    w.show();
    w.setFixedSize(w.width(),w.height());
    return a.exec();
}

#include <QApplication>
#include <QtGui/QImage>

#include <QtDebug>

#include "window.h"
#include "adviewer.h"

int main(int argc,  char **argv)
{
    QApplication app(argc, argv);
    // parse arguments
    QStringList args = app.arguments();
    int image_rate = -1;
    QString prefix;
    bool simple = false;

    QRegExp argRate("--rate=([0-9]{1,})");
    QRegExp argPrefix("--prefix=(.*)");
    QRegExp argSimple("--simple");

    foreach(QString arg, args){
        if (argRate.indexIn(arg) != -1) {
            image_rate = argRate.cap(1).toInt();
        } else if (argPrefix.indexIn(arg) != -1) {
            prefix = argPrefix.cap(1);
        } else if (argSimple.indexIn(arg) != -1) {
            simple = true;
        }
    }

    if (simple && !prefix.isEmpty()) {
        ADViewer *viewer = new ADViewer();
        viewer->setPrefix(prefix);
        viewer->connectChannels();
        if (image_rate == -1)
            viewer->setMonitor(true);
        else
            viewer->setPeriod(1000/image_rate);
        viewer->start();
        viewer->resize(600, 600);
        viewer->show();
    } else {
        MainWindow *win = new MainWindow();
        win->resize(600,600);
        win->show();
    }
    return app.exec();
}

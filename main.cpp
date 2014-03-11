#include <QApplication>
#include <QtGui/QImage>
#include <QtGui/QDesktopWidget>
#include <QtGui/QStyle>

#include <QtDebug>

#include "window.h"
#include "adviewer.h"

/*
 *  Parse X11 geometry argument with the form [<width>x<height>][{+-}<xoffset>{+-}<yoffset>]
 *  The result is returned in a string list <<left, right, width, height>>.
 *  Empty string means the value is not found.
 */
QStringList parseGeometry(QString string)
{
    QStringList geom;
    geom << "" << "" << "" << "";
    QString::const_iterator iter = string.constBegin();
    for(int i=0; i<4; i++) {
        QChar op(0);
        if (*iter == 'x' || *iter == '-' || *iter == '+') {
            op = *iter;
            iter ++;
        }
        // extract the integer number
        QString number;
        while(iter != string.constEnd() && iter->isDigit()) {
            number.append(*iter);
            iter ++;
        }
        if (number.isEmpty())
            continue;
        // depending on op code meaning set to geometry list
        if (op.isNull()) {
            geom[2] = number;
        } else if (op == 'x') {
            geom[3] = number;
        } else {
            // negation
            if (op == '-')
                number.prepend('-');
            if (geom[0].isEmpty())
                geom[0] = number;
             else
                geom[1] = number;
        }
    }
    return geom;
}

int main(int argc,  char **argv)
{
    QApplication app(argc, argv);
    // parse arguments
    QStringList args = app.arguments();
    int image_rate = 0;
    QString prefix;
    bool simple = false;
    bool fullscreen = false;
    // default geometry
    QRect geometry = QStyle::alignedRect(
           Qt::LeftToRight,
           Qt::AlignCenter,
           QSize(600, 600),
           qApp->desktop()->availableGeometry());

    QRegExp argRate("--rate=([0-9]{1,})");
    QRegExp argPrefix("--prefix=(.*)");
    QRegExp argSimple("--simple");
    QRegExp argFullscreen("--fullscreen");
    QRegExp argGeometry("--dg=(.*)");
    QRegExp argHelp("--help");

    foreach(QString arg, args) {
        if (argRate.indexIn(arg) != -1) {
            image_rate = argRate.cap(1).toInt();
        } else if (argPrefix.indexIn(arg) != -1) {
            prefix = argPrefix.cap(1);
        } else if (argSimple.indexIn(arg) != -1) {
            simple = true;
        } else if (argFullscreen.indexIn(arg) != -1) {
            fullscreen = true;
        } else if (argGeometry.indexIn(arg) != -1) {
            QStringList geom = parseGeometry(argGeometry.cap(1));
            if (!geom[2].isEmpty())
                geometry.setWidth(geom[2].toInt());
            if (!geom[3].isEmpty())
                geometry.setHeight(geom[3].toInt());

            if (!geom[0].isEmpty()) {
                int x = geom[0].toInt();
                // negative x means distance from the right border
                if (x < 0)
                    x = qApp->desktop()->screenGeometry().width() + x - geometry.width();
                geometry.moveLeft(x);
            }
            if (!geom[1].isEmpty()) {
                int y = geom[1].toInt();
                // negative y means distance from the bottom border
                if (y < 0)
                    y = qApp->desktop()->screenGeometry().height() + y - geometry.height();
                geometry.moveTop(y);
            }
        } else if (argHelp.indexIn(arg) != -1) {
            qDebug() << "Usage: QtAD [options]\n"
                << "  --prefix=<areaDetector NDArray>: channel prefix of NDStdArray\n"
                << "  --rate=<frame rate>: refresh with fixed frame rate\n"
                << "  --simple: show only the viewer window\n"
                << "  --fullscreen: fullscreen mode\n"
                << "  --dg=<[width][xheight][xoffset][yoffset]>: window geometry\n";
            exit(0);
        }
    }


    if (simple && !prefix.isEmpty()) {
        ADViewer *viewer = new ADViewer();
        viewer->setPrefix(prefix);
        viewer->connectChannels();
        if (image_rate <= 0)
            viewer->setMonitor(true);
        else
            viewer->setPeriod(1000/image_rate);
        viewer->start();
        viewer->setGeometry(geometry);
        if (fullscreen)
            viewer->showFullScreen();
        else
            viewer->showNormal();
    } else {
        MainWindow *win = new MainWindow();
        win->setGeometry(geometry);
        if (fullscreen) 
            win->showFullScreen();
        else
            win->showNormal();
    }
    return app.exec();
}

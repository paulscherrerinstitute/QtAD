#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QtDebug>
#include <QThread>
#include <QtOpenGL>
#include <QWheelEvent>

#include "adviewer.h"

ADViewer :: ADViewer(QWidget *parent)
    : QGLWidget(parent)
{
    _rect = QRectF(0.0, 0.0, 1.0, 1.0);
    _period = 1000;
    _imagerate = 0;
    _timerUpdate = 0;
    setMouseTracking(true);
}

ADViewer :: ~ADViewer ()
{
    glDeleteTextures(1, &_tex);
}

void ADViewer :: setPeriod(int period)
{
    _period = period;
    if(_timerUpdate)
        start();
}

void ADViewer :: connectChannels()
{
    pvData.setChannel(_prefix + "ArrayData"); pvData.init(false);
    connect(&pvData, SIGNAL(valueChanged()), this, SLOT(updateImage()));

    pvDims.setChannel(_prefix + "NDimensions_RBV"); pvDims.init();
    pvSize0.setChannel(_prefix + "ArraySize0_RBV"); pvSize0.init();
    pvSize1.setChannel(_prefix + "ArraySize1_RBV"); pvSize1.init();
    pvSize2.setChannel(_prefix + "ArraySize2_RBV"); pvSize2.init();
    pvColor.setChannel(_prefix + "ColorMode_RBV");  pvColor.init();
    pvUniqueId.setChannel(_prefix + "UniqueId_RBV");  pvUniqueId.init();
    if (pvData.ensureConnection() != ECA_NORMAL)
        qCritical("EPICS channels are not connected");
}

void ADViewer :: start()
{
    if(_timerUpdate)
        killTimer(_timerUpdate);
    _timerUpdate = startTimer(_period);
}

void ADViewer :: stop()
{
    killTimer(_timerUpdate);
    _timerUpdate = 0;
}

ImgInfo ADViewer :: getImageInfo()
{
    // get image info
    int width=0, height=0, depth=0;
    GLenum type,  format;
    switch (pvData.dataType()) {
        case DBF_CHAR:
            type = GL_UNSIGNED_BYTE;
            break; 
        case DBF_SHORT:
            type = GL_UNSIGNED_SHORT; 
            break; 
        default:
            qDebug() << "Unsupported data type!"; 
            break;
    }
    if (pvDims.value() == 2) {
        depth = 1;
        width = pvSize0.value().toInt(); 
        height= pvSize1.value().toInt(); 
        format= GL_LUMINANCE;
    } else {
        depth = 3;
        width = pvSize1.value().toInt(); 
        height= pvSize2.value().toInt(); 
        format= GL_RGB;
    }
    // store image info
    ImgInfo imginfo;
    imginfo.width  = width; 
    imginfo.height = height; 
    imginfo.type   = type; 
    imginfo.format = format; 
    imginfo.size   = width * height * depth;

    return imginfo;
}

void ADViewer :: setMonitor(bool monitor)
{
    if (monitor) {
        ImgInfo imginfo = getImageInfo();
        pvData.monitor(imginfo.size);
    } else
        pvData.unmonitor();
}

void ADViewer :: updateImage()
{
    static int prev_uid = 0;
    static QTime prev = QTime::currentTime();
    static int frame_counter = 0;

    int uid = pvUniqueId.value().toInt(); 
    if (uid == prev_uid)
        return;
    prev_uid = uid;
    _imginfo = getImageInfo();

    // update if aspect ratio has been changed
    if (_imginfo.width *  _rectView.height() != _rectView.width() * _imginfo.height)
        resizeGL(this->width(), this->height());

    if (pvData.monitored()) {
        // Update monitor if image size changed and wait for next update
        if (_imginfo.size != pvData.arraySize()) {
            qDebug() << "update image monitor size " << pvData.arraySize() << "->" <<  _imginfo.size;
            setMonitor(false);
            sleep(0.5);
            setMonitor(true);
            return;
        }
    } else
        pvData.get(_imginfo.size);
    // update frame rate every 5 frames
    frame_counter += 1;
    if (frame_counter == 5) {
        QTime now = QTime::currentTime();
        int msecs  = prev.msecsTo(now);
        _imagerate = frame_counter / (msecs / 1000.) + 0.5;
        prev = now;
        frame_counter = 0;
    }

    // bind image texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _imginfo.width, _imginfo.height,
                 0, _imginfo.format, _imginfo.type, pvData.arrayData());
    updateGL();
}


void ADViewer :: initializeGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth (1.0f);
    glClear (GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // select replace to just draw the texture
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    // create texture
    glGenTextures(1, &_tex);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void ADViewer :: resizeGL(int w, int h)
{ 
    // adjust view port size so that it keeps the image aspect ratio
    int x=0, y=0, width=w, height=h;
    _imginfo = getImageInfo();
    if (_imginfo.size > 0) {
        float aspect = 1.0 * _imginfo.width / _imginfo.height;
        if (h * aspect < w) {
            width = height * aspect;
            x = (w - width) / 2;
        } else {
            height = width / aspect;
            y = (h - height) / 2;
        }
    }
    _rectView.setLeft(x); _rectView.setWidth(width);
    _rectView.setTop(y); _rectView.setHeight(height);

    glViewport(x, y, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void ADViewer :: paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    // compile single textured quad
    glBegin(GL_QUADS);
        glTexCoord2f(_rect.left(), _rect.bottom());    glVertex2f(-1.0, -1.0);
        glTexCoord2f(_rect.right(), _rect.bottom());    glVertex2f( 1.0, -1.0);
        glTexCoord2f(_rect.right(), _rect.top());    glVertex2f( 1.0,  1.0);
        glTexCoord2f(_rect.left(), _rect.top());    glVertex2f(-1.0,  1.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    renderText(-0.95, -0.95, 0, QString("Frame Rate: %1").arg(_imagerate));
    renderText(0.55, -0.95, 0, QString("x:%1 y:%2").arg(_xp).arg(_yp));
}

void ADViewer :: keyReleaseEvent(QKeyEvent *ke)
{
    switch(ke->key())
    {
    case Qt::Key_Left:
        _rect.translate(0.1,0);
        break;
    case Qt::Key_Right:
        _rect.translate(-0.1,0);
        break;
    case Qt::Key_Up:
        _rect.translate(0, 0.1);
        break;
    case Qt::Key_Down:
        _rect.translate(0, -0.1);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
    {
        QPointF pt = _rect.center();
        _rect.setSize(QSizeF(_rect.width()*0.9, _rect.height()*0.9));
        _rect.translate(pt-_rect.center());
    }
        break;
    case Qt::Key_Minus:
    {
        QPointF pt = _rect.center();
        _rect.setSize(QSizeF(_rect.width()*1.1, _rect.height()*1.1));
        _rect.translate(pt-_rect.center());
    }
        break;
    case Qt::Key_Z:
        _rect.setRect(0, 0, 1.0, 1.0);
    }
    updateGL();
}

void ADViewer :: wheelEvent(QWheelEvent *we)
{
    if (we->delta()>0) {
        QPointF pt = _rect.center();
        _rect.setSize(QSizeF(_rect.width()*0.9, _rect.height()*0.9));
        _rect.translate(pt-_rect.center());
    } else {
        QPointF pt = _rect.center();
        _rect.setSize(QSizeF(_rect.width()*1.1, _rect.height()*1.1));
        _rect.translate(pt-_rect.center());
    }
    updateGL();
    we->accept();
}

void ADViewer ::mouseDoubleClickEvent(QMouseEvent *)
{
    _rect.setRect(0, 0, 1.0, 1.0);
    updateGL();
}

void ADViewer :: mousePressEvent(QMouseEvent *me)
{
    _ptLast = me->pos();
    setCursor(Qt::ClosedHandCursor);
}
void ADViewer :: mouseMoveEvent(QMouseEvent* me)
{
    if (!_ptLast.isNull())
    {
        QPointF delta = _ptLast -  me->pos();
        _rect.translate(delta.x()/width(),delta.y()/height());
        _ptLast = me->pos();
    } else {
        _xp =  _imginfo.width * (_rect.left() / _rect.width()  +  1.0 * (me->pos().x() - _rectView.x()) / _rectView.width()) * _rect.width();
        _yp =  _imginfo.height* (_rect.top()  / _rect.height() +  1.0 * (me->pos().y() - _rectView.y()) / _rectView.height())* _rect.height();
    }
    updateGL();
}

void ADViewer :: mouseReleaseEvent(QMouseEvent* me)
{
    setCursor(Qt::ArrowCursor);
    _ptLast = QPoint();
}

void ADViewer :: timerEvent(QTimerEvent *te)
{
    if (te->timerId() == _timerUpdate) {
        updateImage();
    }
}

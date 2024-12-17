#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QtDebug>
#include <QThread>
#include <QWheelEvent>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif

extern "C" {
#include "bayer.h"
}
#include "adviewer.h"

ADViewer :: ADViewer(QWidget *parent)
    : QOpenGLWidget(parent)
{
    _rect = QRectF(0.0, 0.0, 1.0, 1.0);
    _xp = _yp = 0;
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
    pvBayer.setChannel(_prefix + "BayerPattern_RBV");  pvBayer.init();
    pvUniqueId.setChannel(_prefix + "UniqueId_RBV");  pvUniqueId.init();
    pvArrayRate.setChannel(_prefix + "ArrayRate_RBV");  pvArrayRate.init();
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
    GLenum type = GL_UNSIGNED_BYTE,  format=GL_LUMINANCE;
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
    imginfo.color  = pvColor.value().toInt();
    imginfo.bayer  = pvBayer.value().toInt();

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
            Sleep(500);
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

    // decode bayer image
    if (_imginfo.color == 1) {
        if (_imginfo.type == GL_UNSIGNED_BYTE) {
            _img.resize(_imginfo.width * _imginfo.height * 3);
            dc1394error_t err = dc1394_bayer_decoding_8bit((uint8_t*)pvData.arrayData(), (uint8_t*)_img.data(),
                    _imginfo.width, _imginfo.height, (dc1394color_filter_t)(_imginfo.bayer + DC1394_COLOR_FILTER_MIN),
                    DC1394_BAYER_METHOD_NEAREST);
            if (err == DC1394_SUCCESS)
                _imginfo.format = GL_RGB;
        } else if (_imginfo.type == GL_UNSIGNED_SHORT) {
            _img.resize(_imginfo.width * _imginfo.height * sizeof(uint16_t) * 3);
            dc1394error_t err = dc1394_bayer_decoding_16bit((uint16_t*)pvData.arrayData(), (uint16_t*)_img.data(),
                    _imginfo.width, _imginfo.height, (dc1394color_filter_t)(_imginfo.bayer + DC1394_COLOR_FILTER_MIN),
                    DC1394_BAYER_METHOD_NEAREST, 12);
            if (err == DC1394_SUCCESS)
                _imginfo.format = GL_RGB;
        }
    } else {
        _img.resize(pvData.arrayBytes());
        memcpy(_img.data(), pvData.arrayData(), pvData.arrayBytes());
    }
    _imginfo.updated.storeRelaxed(true);
    update();
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

}

void ADViewer :: paintGL()
{
    glClear (GL_COLOR_BUFFER_BIT);
    glViewport(_rectView.left() * devicePixelRatio(), _rectView.top() * devicePixelRatio(),
               _rectView.width() * devicePixelRatio(), _rectView.height() * devicePixelRatio() );

    // compile single textured quad
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _tex);
    if (_imginfo.updated.testAndSetRelaxed(true, false)) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _imginfo.width, _imginfo.height, 0, _imginfo.format, _imginfo.type, _img.data());
    }
    glBegin(GL_QUADS);
        glTexCoord2f(_rect.left(), _rect.bottom());    glVertex2f(-1.0, -1.0);
        glTexCoord2f(_rect.right(), _rect.bottom());    glVertex2f( 1.0, -1.0);
        glTexCoord2f(_rect.right(), _rect.top());    glVertex2f( 1.0,  1.0);
        glTexCoord2f(_rect.left(), _rect.top());    glVertex2f(-1.0,  1.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Render text
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.drawText(_rectView.left() + 5, _rectView.bottom() - 20, QString("Frame Rate: %1").arg(pvArrayRate.value().toInt() ? _imagerate : 0));
    painter.drawText(_rectView.right() - 100, _rectView.bottom() - 20, QString("x:%1 y:%2").arg(_xp).arg(_yp));
    painter.end();
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
        zoom(0.9);
        break;
    case Qt::Key_Minus:
        zoom(1.1);
        break;
    case Qt::Key_Z:
        _rect.setRect(0, 0, 1.0, 1.0);
    }
    update();
}

void  ADViewer :: zoom(double factor, const QPointF point)
{
    QPointF pointOrigin = point;
    if (pointOrigin.isNull()) {
        pointOrigin = _rect.center();
    }

    QPointF pointResized = (pointOrigin - _rect.topLeft()) * factor + _rect.topLeft();

    _rect.setSize(_rect.size() * factor);
    _rect.translate(pointOrigin - pointResized);
}

QPointF ADViewer :: mapToGL(const QPointF point) const
{
    qreal xp = _rect.left() + 1.0 * (point.x() - _rectView.x()) / _rectView.width() * _rect.width();
    qreal yp = _rect.top()  + 1.0 * (point.y() - _rectView.y()) / _rectView.height()* _rect.height();
    return QPointF(xp, yp);
}

void ADViewer :: wheelEvent(QWheelEvent *we)
{
    if (we->angleDelta().y() > 0)
        zoom(0.9, mapToGL(we->position()));
    else
        zoom(1.1, mapToGL(we->position()));

    update();
    we->accept();
}

void ADViewer ::mouseDoubleClickEvent(QMouseEvent *)
{
    _rect.setRect(0, 0, 1.0, 1.0);
    update();
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
        QPointF delta = mapToGL(_ptLast) -  mapToGL(me->pos());
        _rect.translate(delta);
        _ptLast = me->pos();
    } else {
        QPointF pt = mapToGL(me->pos());
        _xp =  _imginfo.width * pt.x();
        _yp =  _imginfo.height * pt.y();
    }
    update();
}

void ADViewer :: mouseReleaseEvent(QMouseEvent*)
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

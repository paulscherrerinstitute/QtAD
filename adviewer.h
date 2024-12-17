#ifndef ADVIEWER_H
#define ADVIEWER_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <pvobject.h>
#include <QAtomicInteger>

typedef struct {
    int width; 
    int height; 
    int color;
    int bayer;
    GLenum type; 
    GLenum format; 
    unsigned int size;
    QAtomicInteger<bool> updated;
} ImgInfo; 

class ADViewer : public QOpenGLWidget
{
    Q_OBJECT
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
    Q_PROPERTY(int period READ period WRITE setPeriod)
    Q_PROPERTY(int imageRate READ imageRate)

public:
    ADViewer(QWidget *parent = 0); 
    ~ADViewer(); 

    QString prefix() {return _prefix;}
    void setPrefix(QString prefix) {_prefix = prefix;}

    int period() {return _period;}
    void setPeriod(int);

    int imageRate() { return _imagerate;}

    void setMonitor(bool);

    void connectChannels();
    void start();
    void stop();

public slots:
    void updateImage(); 

protected:
    void initializeGL(); 
    void resizeGL(int, int); 
    void paintGL(); 
    void keyReleaseEvent(QKeyEvent *);
    void wheelEvent(QWheelEvent*);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);

    void zoom(double factor, const QPointF=QPointF());
    QPointF mapToGL(const QPointF) const;

    ImgInfo getImageInfo();

private:
    QString _prefix; 
    int _period;

    PvObject pvData; 
    PvObject pvDims; 
    PvObject pvSize0; 
    PvObject pvSize1;
    PvObject pvSize2; 
    PvObject pvColor; 
    PvObject pvBayer;
    PvObject pvUniqueId; 
    PvObject pvArrayRate;

    int _imagerate;
    int _imagecounter;
    GLuint  _tex;
    int _timerUpdate;
    int _timerCounter;
    QRectF _rect;
    QRect _rectView;
    QPointF _ptLast;
    ImgInfo _imginfo;
    std::vector<uint8_t> _img;
    int _xp;
    int _yp;
}; 

#endif // ADVIEWER_H


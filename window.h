#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class ADViewer;
class QLineEdit;
class QLabel;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    
signals:
    
public slots:
    void changePrefix();
    void changePeriod(int);
    void updateImage(int);

protected:
    void timerEvent(QTimerEvent *);

private:
     ADViewer *viewer;
     QLineEdit *editPrefix;
     QLabel *labelImageRate;
};

#endif // WINDOW_H

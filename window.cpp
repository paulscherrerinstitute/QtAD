#include <QBoxLayout>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>

#include "adviewer.h"
#include "window.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel * label = new QLabel("EPICS Prefix:");
    editPrefix = new QLineEdit();
    connect(editPrefix, SIGNAL(editingFinished()), this, SLOT(changePrefix()));
    hLayout->addWidget(label);
    hLayout->addWidget(editPrefix, 2);

    label = new QLabel("Frame Rate:");
    QSpinBox *spin = new QSpinBox();
    spin->setRange(1,50);
    connect(spin, SIGNAL(valueChanged(int)), this, SLOT(changePeriod(int)));
    labelImageRate = new QLabel("0");
    hLayout->addWidget(label);
    hLayout->addWidget(spin);
    hLayout->addWidget(labelImageRate);

    QButtonGroup *buttonGroup = new QButtonGroup();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(updateImage(int)));
#else
    connect(buttonGroup, SIGNAL(idClicked(int)), this, SLOT(updateImage(int)));
#endif
    hLayout->addStretch(1);

    QRadioButton *radio = new QRadioButton("Monitor");
    buttonGroup->addButton(radio, 0);
    hLayout->addWidget(radio);

    radio = new QRadioButton("Update");
    buttonGroup->addButton(radio, 1);
    hLayout->addWidget(radio);

    viewer = new ADViewer();
    vLayout->addLayout(hLayout);
    vLayout->addWidget(viewer);

    // initialize 
    changePeriod(1);
    radio->setChecked(true);
    updateImage(1);

    startTimer(2000);
}

void MainWindow :: changePrefix()
{
    if (editPrefix->text() != viewer->prefix()) {
        viewer->setPrefix(editPrefix->text());
        viewer->connectChannels();
        setWindowTitle(QString("%1 - QtAD").arg(viewer->prefix()));
    }
}

void MainWindow :: changePeriod(int framerate)
{
    viewer->setPeriod(1000/framerate);
}

void MainWindow :: updateImage(int id)
{
    if (id == 0) {
        viewer->stop();
        viewer->setMonitor(true);
    } else {
        viewer->setMonitor(false);
        viewer->start();
    }
}

void MainWindow :: timerEvent(QTimerEvent *)
{
    labelImageRate->setText(QString::number(viewer->imageRate()));
}

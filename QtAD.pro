TEMPLATE = app
QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
MOC_DIR = .moc
OBJECTS_DIR = .obj
VERSION = 1.0.0

macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Darwin
    LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a
}

unix:!macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Linux
    LIBS += -lreadline $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a
}


win32{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/WIN32
    LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom -lws2_32
}

# Input
HEADERS += pvobject.h \
            adviewer.h \
            window.h

SOURCES += pvobject.cpp \
            adviewer.cpp \
            main.cpp \
            window.cpp

# Install
isEmpty(PREFIX) {
    unix {
        PREFIX=/usr/local/bin
    }
}
target.path = $$PREFIX
INSTALLS += target

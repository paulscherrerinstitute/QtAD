TEMPLATE = app
QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
MOC_DIR = .moc
OBJECTS_DIR = .obj
VERSION = 1.0.0

QMAKE_CFLAGS += -std=c99

macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Darwin $$(EPICS_BASE)/include/compiler/clang
    LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a
}

unix:!macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Linux $$(EPICS_BASE)/include/compiler/gcc
    LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a -lreadline -ldl -lrt
}


win32{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/WIN32 $$(EPICS_BASE)/include/compiler/msvc
    LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom -lws2_32 -lopengl32
}

# Input
HEADERS += pvobject.h \
            adviewer.h \
            bayer.h \
            window.h

SOURCES += pvobject.cpp \
            adviewer.cpp \
            main.cpp \
            bayer.c \
            window.cpp

# Install
isEmpty(PREFIX) {
    unix {
        PREFIX=/usr/local/bin
    }
}
target.path = $$PREFIX
INSTALLS += target

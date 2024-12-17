TEMPLATE = app
QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
MOC_DIR = .moc
OBJECTS_DIR = .obj
VERSION = 2.0.0

QMAKE_CFLAGS += -std=c99

INCLUDEPATH += $$(EPICS_BASE)/include
LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)

macx{
    INCLUDEPATH += $$(EPICS_BASE)/include/os/Darwin $$(EPICS_BASE)/include/compiler/clang
    exists($$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a) {
        LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a
    } else {
        LIBS += -Wl,-rpath,$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom
    }
}

unix:!macx{
    INCLUDEPATH += $$(EPICS_BASE)/include/os/Linux $$(EPICS_BASE)/include/compiler/gcc
    exists($$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a) {
        LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libca.a $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/libCom.a -lreadline -ldl -lrt
    } else {
        LIBS += -Wl,-rpath,$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom
    }
}


win32{
    INCLUDEPATH += $$(EPICS_BASE)/include/os/WIN32 $$(EPICS_BASE)/include/compiler/msvc
    LIBS += -lca -lCom -lws2_32 -lopengl32
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

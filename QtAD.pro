TEMPLATE = app
QT += core gui opengl
MOC_DIR = .moc
OBJECTS_DIR = .obj

macx{
    INCLUDEPATH += /Users/wang/epics/base/include  /Users/wang/epics/base/include/os/Darwin
    LIBS += -L/Users/wang/epics/base/lib/darwin-x86 -lca -lCom
}

unix:!macx{
    INCLUDEPATH += /usr/local/epics/base/include  /usr/local/epics/base/include/os/Linux
    LIBS += -L/usr/local/epics/base/lib/$$(EPICS_HOST_ARCH) -Wl,-R/usr/local/epics/base/lib/$$(EPICS_HOST_ARCH) -lca -lCom
}


win32{
    INCLUDEPATH += . C:/epics/base-3.14.12.2-VC10/include  C:/epics/base-3.14.12.2-VC10/include/os/WIN32
    LIBS += -LC:/epics/base-3.14.12.2-VC10/lib/win32-x86-mingw -lca -lCom -lws2_32
}

# Input
HEADERS += pvobject.h \
            adviewer.h \
            window.h

SOURCES += pvobject.cpp \
            adviewer.cpp \
            main.cpp \
            window.cpp

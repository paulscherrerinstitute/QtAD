TEMPLATE = app
QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
MOC_DIR = .moc
OBJECTS_DIR = .obj

macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Darwin
    LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom
}

unix:!macx{
    INCLUDEPATH += $$(EPICS_BASE)/include  $$(EPICS_BASE)/include/os/Linux
    LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -Wl,-R$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) -lca -lCom
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

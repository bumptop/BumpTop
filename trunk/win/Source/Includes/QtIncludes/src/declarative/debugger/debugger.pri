INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/qdeclarativedebuggerstatus.cpp \
    $$PWD/qpacketprotocol.cpp \
    $$PWD/qdeclarativedebugservice.cpp \
    $$PWD/qdeclarativedebugclient.cpp \
    $$PWD/qdeclarativedebug.cpp \
    $$PWD/qdeclarativedebugtrace.cpp \
    $$PWD/qdeclarativedebughelper.cpp \
    $$PWD/qdeclarativedebugserver.cpp

HEADERS += \
    $$PWD/qdeclarativedebuggerstatus_p.h \
    $$PWD/qpacketprotocol_p.h \
    $$PWD/qdeclarativedebugservice_p.h \
    $$PWD/qdeclarativedebugservice_p_p.h \
    $$PWD/qdeclarativedebugclient_p.h \
    $$PWD/qdeclarativedebug_p.h \
    $$PWD/qdeclarativedebugtrace_p.h \
    $$PWD/qdeclarativedebughelper_p.h \
    $$PWD/qdeclarativedebugserver_p.h \
    debugger/qdeclarativedebugserverconnection_p.h

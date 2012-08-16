TARGET  = qmlgesturesplugin
TARGETPATH = Qt/labs/gestures
include(../qimportbase.pri)

QT += declarative

SOURCES += qdeclarativegesturearea.cpp plugin.cpp
HEADERS += qdeclarativegesturearea_p.h

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/imports/$$TARGETPATH
target.path = $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

qmldir.files += $$PWD/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$TARGETPATH

symbian:{
    TARGET.UID3 = 0x2002131F
    
    isEmpty(DESTDIR):importFiles.sources = qmlgesturesplugin$${QT_LIBINFIX}.dll qmldir
    else:importFiles.sources = $$DESTDIR/qmlgesturesplugin$${QT_LIBINFIX}.dll qmldir
    importFiles.path = $$QT_IMPORTS_BASE_DIR/$$TARGETPATH
    
    DEPLOYMENT = importFiles
}

INSTALLS += target qmldir

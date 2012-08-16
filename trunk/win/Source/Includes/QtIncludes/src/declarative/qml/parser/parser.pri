INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/qdeclarativejsast_p.h \
    $$PWD/qdeclarativejsastfwd_p.h \
    $$PWD/qdeclarativejsastvisitor_p.h \
    $$PWD/qdeclarativejsengine_p.h \
    $$PWD/qdeclarativejsgrammar_p.h \
    $$PWD/qdeclarativejslexer_p.h \
    $$PWD/qdeclarativejsmemorypool_p.h \
    $$PWD/qdeclarativejsnodepool_p.h \
    $$PWD/qdeclarativejsparser_p.h \
    $$PWD/qdeclarativejsglobal_p.h

SOURCES += \
    $$PWD/qdeclarativejsast.cpp \
    $$PWD/qdeclarativejsastvisitor.cpp \
    $$PWD/qdeclarativejsengine_p.cpp \
    $$PWD/qdeclarativejsgrammar.cpp \
    $$PWD/qdeclarativejslexer.cpp \
    $$PWD/qdeclarativejsparser.cpp

# Qt network access module

HEADERS += \
    access/qftp.h \
    access/qhttp.h \
    access/qhttpnetworkheader_p.h \
    access/qhttpnetworkrequest_p.h \
    access/qhttpnetworkreply_p.h \
    access/qhttpnetworkconnection_p.h \
    access/qhttpnetworkconnectionchannel_p.h \
    access/qfilenetworkreply_p.h \
    access/qnetworkaccessmanager.h \
    access/qnetworkaccessmanager_p.h \
    access/qnetworkaccesscache_p.h \
    access/qnetworkaccessbackend_p.h \
    access/qnetworkaccessdatabackend_p.h \
    access/qnetworkaccessdebugpipebackend_p.h \
    access/qnetworkaccesshttpbackend_p.h \
    access/qnetworkaccessfilebackend_p.h \
    access/qnetworkaccesscachebackend_p.h \
    access/qnetworkaccessftpbackend_p.h \
    access/qnetworkcookie.h \
    access/qnetworkcookie_p.h \
    access/qnetworkcookiejar.h \
    access/qnetworkcookiejar_p.h \
    access/qnetworkcookiejartlds_p.h \
    access/qnetworkrequest.h \
    access/qnetworkrequest_p.h \
    access/qnetworkreply.h \
    access/qnetworkreply_p.h \
    access/qnetworkreplyimpl_p.h \
    access/qabstractnetworkcache_p.h \
    access/qabstractnetworkcache.h \
    access/qnetworkdiskcache_p.h \
    access/qnetworkdiskcache.h

SOURCES += \
    access/qftp.cpp \
    access/qhttp.cpp \
    access/qhttpnetworkheader.cpp \
    access/qhttpnetworkrequest.cpp \
    access/qhttpnetworkreply.cpp \
    access/qhttpnetworkconnection.cpp \
    access/qhttpnetworkconnectionchannel.cpp \
    access/qfilenetworkreply.cpp \
    access/qnetworkaccessmanager.cpp \
    access/qnetworkaccesscache.cpp \
    access/qnetworkaccessbackend.cpp \
    access/qnetworkaccessdatabackend.cpp \
    access/qnetworkaccessdebugpipebackend.cpp \
    access/qnetworkaccessfilebackend.cpp \
    access/qnetworkaccesscachebackend.cpp \
    access/qnetworkaccessftpbackend.cpp \
    access/qnetworkaccesshttpbackend.cpp \
    access/qnetworkcookie.cpp \
    access/qnetworkcookiejar.cpp \
    access/qnetworkrequest.cpp \
    access/qnetworkreply.cpp \
    access/qnetworkreplyimpl.cpp \
    access/qabstractnetworkcache.cpp \
    access/qnetworkdiskcache.cpp

include($$PWD/../../3rdparty/zlib_dependency.pri)

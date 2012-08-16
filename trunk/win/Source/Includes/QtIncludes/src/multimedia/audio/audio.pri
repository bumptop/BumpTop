HEADERS += $$PWD/qaudio.h \
           $$PWD/qaudioformat.h \
           $$PWD/qaudioinput.h \
           $$PWD/qaudiooutput.h \
           $$PWD/qaudiodeviceinfo.h \
           $$PWD/qaudioengineplugin.h \
           $$PWD/qaudioengine.h \
           $$PWD/qaudiodevicefactory_p.h


SOURCES += $$PWD/qaudio.cpp \
           $$PWD/qaudioformat.cpp  \
           $$PWD/qaudiodeviceinfo.cpp \
           $$PWD/qaudiooutput.cpp \
           $$PWD/qaudioinput.cpp \
           $$PWD/qaudioengineplugin.cpp \
           $$PWD/qaudioengine.cpp \
           $$PWD/qaudiodevicefactory.cpp

contains(QT_CONFIG, audio-backend) {

mac {
    HEADERS +=  $$PWD/qaudioinput_mac_p.h \
                $$PWD/qaudiooutput_mac_p.h \
                $$PWD/qaudiodeviceinfo_mac_p.h \
                $$PWD/qaudio_mac_p.h

    SOURCES += $$PWD/qaudiodeviceinfo_mac_p.cpp \
               $$PWD/qaudiooutput_mac_p.cpp \
               $$PWD/qaudioinput_mac_p.cpp \
               $$PWD/qaudio_mac.cpp

    LIBS += -framework ApplicationServices -framework CoreAudio -framework AudioUnit -framework AudioToolbox

} else:win32 {

    HEADERS += $$PWD/qaudioinput_win32_p.h $$PWD/qaudiooutput_win32_p.h $$PWD/qaudiodeviceinfo_win32_p.h
    SOURCES += $$PWD/qaudiodeviceinfo_win32_p.cpp \
               $$PWD/qaudiooutput_win32_p.cpp \
               $$PWD/qaudioinput_win32_p.cpp
    !wince*:LIBS += -lwinmm
    wince*:LIBS += -lcoredll

} else:symbian {
    INCLUDEPATH += $${EPOCROOT}epoc32/include/mmf/common
    INCLUDEPATH += $${EPOCROOT}epoc32/include/mmf/server

    HEADERS += $$PWD/qaudio_symbian_p.h \
               $$PWD/qaudiodeviceinfo_symbian_p.h \
               $$PWD/qaudioinput_symbian_p.h \
               $$PWD/qaudiooutput_symbian_p.h

    SOURCES += $$PWD/qaudio_symbian_p.cpp \
               $$PWD/qaudiodeviceinfo_symbian_p.cpp \
               $$PWD/qaudioinput_symbian_p.cpp \
               $$PWD/qaudiooutput_symbian_p.cpp

    LIBS += -lmmfdevsound
} else:unix {
    unix:contains(QT_CONFIG, alsa) {
        linux-*|freebsd-*|openbsd-*:{
            DEFINES += HAS_ALSA
            HEADERS += $$PWD/qaudiooutput_alsa_p.h $$PWD/qaudioinput_alsa_p.h $$PWD/qaudiodeviceinfo_alsa_p.h
            SOURCES += $$PWD/qaudiodeviceinfo_alsa_p.cpp \
                   $$PWD/qaudiooutput_alsa_p.cpp \
                   $$PWD/qaudioinput_alsa_p.cpp
            LIBS_PRIVATE += -lasound
        }
    }
}
} else {
    DEFINES += QT_NO_AUDIO_BACKEND
}

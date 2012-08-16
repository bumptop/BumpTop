contains(QT_CONFIG, egl): {
	CONFIG += egl

	HEADERS += \
	    egl/qegl_p.h \
	    egl/qeglcontext_p.h \
	    egl/qeglproperties_p.h

	SOURCES += \
	    egl/qegl.cpp \
	    egl/qeglproperties.cpp

	wince*: SOURCES += egl/qegl_wince.cpp

	unix {
	    embedded {
	        SOURCES += egl/qegl_qws.cpp
	    } else {
	        symbian {
	            SOURCES += egl/qegl_symbian.cpp
	        } else {
	            SOURCES += egl/qegl_x11.cpp
	        }
	    }
	}
} else:symbian {
	DEFINES += QT_NO_EGL
	SOURCES += egl/qegl_stub.cpp
	SOURCES += egl/qeglproperties_stub.cpp
}

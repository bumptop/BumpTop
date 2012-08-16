# Qt core library plugin module

HEADERS += \
	plugin/qfactoryinterface.h \
	plugin/qpluginloader.h \
	plugin/qlibrary.h \
	plugin/qlibrary_p.h \
	plugin/qplugin.h \
	plugin/quuid.h \
	plugin/qfactoryloader_p.h \
	plugin/qsystemlibrary_p.h

SOURCES += \
	plugin/qpluginloader.cpp \
	plugin/qfactoryloader.cpp \
	plugin/quuid.cpp \
	plugin/qlibrary.cpp

win32 {
	SOURCES += \
		plugin/qlibrary_win.cpp \
		plugin/qsystemlibrary.cpp
}

unix {
	SOURCES += plugin/qlibrary_unix.cpp
}

LIBS_PRIVATE += $$QMAKE_LIBS_DYNLOAD

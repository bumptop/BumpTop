if (PHONON_BUILD_TESTS)
    add_subdirectory(tests)
endif (PHONON_BUILD_TESTS)

if (PHONON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif (PHONON_BUILD_EXAMPLES)

add_subdirectory(experimental)

set(PULSEAUDIO_MINIMUM_VERSION "0.9.15")
macro_optional_find_package(PulseAudio)
if (PULSEAUDIO_FOUND)
    # PULSEAUDIO_DEVICE_MANAGER feature check could be moved to FindPulseAudio.cmake, hint hint. -- Rex
    macro_ensure_version("0.9.21" "${PULSEAUDIO_VERSION}" PULSEAUDIO_DEVICE_MANAGER)
endif (PULSEAUDIO_FOUND)
macro_log_feature(PULSEAUDIO_FOUND "PulseAudio" "A cross-platform, networked sound server." "http://www.pulseaudio.org" FALSE "" "Allows audio playback via the PulseAudio soundserver when it is running")
macro_optional_find_package(GLIB2)
macro_log_feature(GLIB2_FOUND "GLib2" "GLib 2 is required to compile the pulseaudio for Phonon" "http://www.gtk.org/download/" FALSE)


if (GLIB2_FOUND AND PULSEAUDIO_FOUND)
   add_definitions(-DHAVE_PULSEAUDIO)
   include_directories(${GLIB2_INCLUDE_DIR} ${PULSEAUDIO_INCLUDE_DIR})
   if (PULSEAUDIO_DEVICE_MANAGER)
      add_definitions(-DHAVE_PULSEAUDIO_DEVICE_MANAGER)
   endif(PULSEAUDIO_DEVICE_MANAGER)
else(GLIB2_FOUND AND PULSEAUDIO_FOUND)
   set(PULSEAUDIO_INCLUDE_DIR "")
   set(PULSEAUDIO_LIBRARY "")
   set(PULSEAUDIO_MAINLOOP_LIBRARY "")
endif(GLIB2_FOUND AND PULSEAUDIO_FOUND)


set(phonon_LIB_SRCS
   objectdescription.cpp
   objectdescriptionmodel.cpp
   phononnamespace.cpp
   mediasource.cpp
   abstractmediastream.cpp
   streaminterface.cpp
   mediaobject.cpp
   medianode.cpp
   path.cpp
   effectparameter.cpp
   effect.cpp
   volumefadereffect.cpp
   abstractaudiooutput.cpp
   abstractaudiooutput_p.cpp
   audiooutput.cpp
   audiooutputinterface.cpp
   abstractvideooutput.cpp
   abstractvideooutput_p.cpp
   backendcapabilities.cpp
   globalconfig.cpp
   factory.cpp
   platform.cpp
   mediacontroller.cpp
   videowidget.cpp
   videoplayer.cpp
   seekslider.cpp
   swiftslider.cpp
   volumeslider.cpp
   effectwidget.cpp
   iodevicestream.cpp
   audiodataoutput.cpp
   pulsesupport.cpp
   )

if (QT_QTDBUS_FOUND)
   list(APPEND phonon_LIB_SRCS
      audiooutputadaptor.cpp
   )
endif (QT_QTDBUS_FOUND)


add_definitions(-DPHONON_LIBRARY_PATH="${PLUGIN_INSTALL_DIR}/plugins")
automoc4_add_library(phonon SHARED ${phonon_LIB_SRCS})
target_link_libraries(phonon ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
if (GLIB2_FOUND AND PULSEAUDIO_FOUND)
target_link_libraries(phonon ${GLIB2_LIBRARIES} ${GOBJECT_LIBRARIES} ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})
endif (GLIB2_FOUND AND PULSEAUDIO_FOUND)

if (QT_QTDBUS_FOUND)
  target_link_libraries(phonon ${QT_QTDBUS_LIBRARY})
endif (QT_QTDBUS_FOUND)
if (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  # We need to explicitly link libm to phonon in Solaris
  target_link_libraries(phonon m)
endif (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")

set_target_properties(phonon PROPERTIES
                             VERSION ${PHONON_LIB_VERSION}
                             SOVERSION ${PHONON_LIB_SOVERSION}
                             DEFINE_SYMBOL MAKE_PHONON_LIB
                             )
install(TARGETS phonon ${INSTALL_TARGETS_DEFAULT_ARGS})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/phononnamespace.h.in ${CMAKE_CURRENT_BINARY_DIR}/phononnamespace.h)

install(FILES
   phonon_export.h
   objectdescription.h
   objectdescriptionmodel.h
   ${CMAKE_CURRENT_BINARY_DIR}/phononnamespace.h
   mediasource.h
   abstractmediastream.h
   streaminterface.h
   mediaobject.h
   audiooutput.h
   medianode.h
   path.h
   effectparameter.h
   effect.h
   effectinterface.h
   volumefadereffect.h
   volumefaderinterface.h
   abstractaudiooutput.h
   abstractvideooutput.h
   backendcapabilities.h
   phonondefs.h
   backendinterface.h
   mediaobjectinterface.h
   audiooutputinterface.h
   addoninterface.h
   mediacontroller.h
   videowidget.h
   videowidgetinterface.h
   videoplayer.h
   seekslider.h
   volumeslider.h
   effectwidget.h
   platformplugin.h
   audiodataoutput.h
   audiodataoutputinterface.h
   globalconfig.h
   pulsesupport.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/phonon COMPONENT Devel)

install(FILES org.kde.Phonon.AudioOutput.xml  DESTINATION  ${DBUS_INTERFACES_INSTALL_DIR})

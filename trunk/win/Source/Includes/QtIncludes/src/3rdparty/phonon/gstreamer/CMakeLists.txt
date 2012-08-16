# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
# Copyright (C) 2008 Matthias Kretz <kretz@kde.org>
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 or 3 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.

project(phonon-gstreamer)
include(ConfigureChecks.cmake)

if (BUILD_PHONON_GSTREAMER)
   include_directories(
	  ${CMAKE_CURRENT_BINARY_DIR}
	  ${GSTREAMER_INCLUDE_DIR}
	  ${GLIB2_INCLUDE_DIR}
      ${LIBXML2_INCLUDE_DIR}
	  ${X11_X11_INCLUDE_PATH})
   add_definitions(-DPHONON_BACKEND_VERSION_4_2)

   # configure plugin api
   if(USE_INSTALL_PLUGIN)
      set(PLUGIN_INSTALL_API TRUE)
   endif(USE_INSTALL_PLUGIN)

   configure_file(phonon-config-gstreamer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/phonon-config-gstreamer.h )

   set(phonon_gstreamer_SRCS
      audiooutput.cpp
      backend.cpp
      devicemanager.cpp
      effectmanager.cpp
      gsthelper.cpp
      mediaobject.cpp
      medianode.cpp
      effect.cpp
      medianodeevent.cpp
      videowidget.cpp
      qwidgetvideosink.cpp
      streamreader.cpp
      phononsrc.cpp
      message.cpp
      audioeffect.cpp
      abstractrenderer.cpp
      widgetrenderer.cpp
      glrenderer.cpp
      volumefadereffect.cpp
      audiodataoutput.cpp
      )

   if(NOT WIN32)
      set(phonon_gstreamer_SRCS
          ${phonon_gstreamer_SRCS}
          artssink.cpp
          x11renderer.cpp)
      macro_optional_find_package(Alsa)
      macro_ensure_version("0.10.22" ${GSTREAMER_VERSION} GSTREAMER_HAS_NONBLOCKING_ALSASINK)
   endif(NOT WIN32)
   if(ALSA_FOUND AND NOT GSTREAMER_HAS_NONBLOCKING_ALSASINK)
      add_definitions(-DUSE_ALSASINK2)
      include_directories(${ALSA_INCLUDES})
      set(phonon_gstreamer_SRCS
         ${phonon_gstreamer_SRCS}
         alsasink2.c
         )
   endif(ALSA_FOUND AND NOT GSTREAMER_HAS_NONBLOCKING_ALSASINK)

   automoc4_add_library(phonon_gstreamer MODULE ${phonon_gstreamer_SRCS})
   set_target_properties(phonon_gstreamer PROPERTIES PREFIX "")
   target_link_libraries(phonon_gstreamer
      ${QT_QTOPENGL_LIBRARY}
      ${PHONON_LIBS} ${OPENGL_gl_LIBRARY}
      ${GSTREAMER_LIBRARIES} ${GSTREAMER_BASE_LIBRARY} ${GSTREAMER_INTERFACE_LIBRARY}
      ${GSTREAMER_PLUGIN_VIDEO_LIBRARIES} ${GSTREAMER_PLUGIN_AUDIO_LIBRARIES}
      ${GLIB2_LIBRARIES} ${GOBJECT_LIBRARIES})
   if(ALSA_FOUND)
      target_link_libraries(phonon_gstreamer ${ASOUND_LIBRARY})
   endif(ALSA_FOUND)
   if(USE_INSTALL_PLUGIN)
       target_link_libraries(phonon_gstreamer ${GSTREAMER_PLUGIN_PBUTILS_LIBRARIES})
   endif(USE_INSTALL_PLUGIN)

   install(TARGETS phonon_gstreamer DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/phonon_backend)
   install(FILES gstreamer.desktop DESTINATION ${SERVICES_INSTALL_DIR}/phononbackends)
endif (BUILD_PHONON_GSTREAMER)

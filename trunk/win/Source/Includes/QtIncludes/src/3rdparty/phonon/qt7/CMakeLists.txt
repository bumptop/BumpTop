# Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
project(phonon-qt7)
include(ConfigureChecks.cmake)

include_directories(${OPENGL_INCLUDE_DIR})

if (BUILD_PHONON_QT7)
    set(phonon_qt7_SRCS
        quicktimevideoplayer.mm
        backendheader.mm 
        medianodevideopart.mm 
        medianodeevent.mm 
        audiooutput.mm 
        backendinfo.mm 
        audiosplitter.mm 
        audioeffects.mm 
        quicktimestreamreader.mm 
        medianode.mm 
        backend.mm 
        mediaobject.mm 
        mediaobjectaudionode.mm 
        audiomixer.mm  
        quicktimeaudioplayer.mm 
        videoframe.mm 
        quicktimemetadata.mm 
        audiodevice.mm 
        audioconnection.mm 
        audiograph.mm 
        audionode.mm 
        videowidget.mm
       )

   automoc4_add_library(phonon_qt7 MODULE ${phonon_qt7_SRCS})
   target_link_libraries(phonon_qt7
       ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY}
       ${PHONON_LIBS}
       "-framework QuickTime"
       "-framework AudioUnit"
       "-framework AudioToolbox"
       "-framework CoreAudio"
       "-framework QuartzCore"
       "-framework QTKit"
       )
    install(TARGETS phonon_qt7 DESTINATION ${PLUGIN_INSTALL_DIR})

endif (BUILD_PHONON_QT7)

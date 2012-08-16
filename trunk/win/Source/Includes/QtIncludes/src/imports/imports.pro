TEMPLATE = subdirs

SUBDIRS += folderlistmodel particles gestures
contains(QT_CONFIG, opengl): SUBDIRS += shaders


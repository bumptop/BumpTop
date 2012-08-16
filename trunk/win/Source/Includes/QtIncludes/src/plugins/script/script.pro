TEMPLATE = subdirs
contains(QT_CONFIG, script):contains(QT_CONFIG, dbus):SUBDIRS += qtdbus

TEMPLATE = subdirs

SUBDIRS =

unix:contains(QT_CONFIG, gstreamer): SUBDIRS *= gstreamer
mac:contains(QT_CONFIG, phonon-backend): SUBDIRS *= qt7
win32:!wince*:contains(QT_CONFIG, phonon-backend): SUBDIRS *= ds9
wince*:contains(QT_CONFIG, phonon-backend): SUBDIRS *= waveout
wince*:contains(QT_CONFIG, directshow): SUBDIRS *= ds9

# Note that the MMF backend is in some scenarios an important complement to the
# Helix backend: the latter requires Symbian signed capabilities, hence MMF
# provides multimedia for self signed scenarios.
symbian:contains(QT_CONFIG, phonon-backend): SUBDIRS *= mmf

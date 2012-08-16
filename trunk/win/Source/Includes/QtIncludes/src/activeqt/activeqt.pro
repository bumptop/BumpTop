TEMPLATE = subdirs

CONFIG	+= ordered
SUBDIRS	= container
!wince*: SUBDIRS += control

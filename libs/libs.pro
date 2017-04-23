TEMPLATE = subdirs

SUBDIRS = \
    hoedown \
    googletest \
    librepcb \
    pugixml \
    quazip

librepcb.depends = hoedown pugixml quazip

include(../plugin_pro.pri)
DEPENDPATH += . \
    GeneratedFiles
QT += gui

# Input
HEADERS += contacttree.h \
    clistoptions.h \
    contactlist.h \
    clistwin.h \
    contacttreemodel.h \
    ../../include/clist_i.h \
    ../../include/options_i.h \
    ../../include/menus_i.h
FORMS += clistoptions.ui \
    clistwin.ui \
    contacttree.ui
SOURCES += clistoptions.cpp \
    contactlist.cpp \
    clistwin.cpp \
    contacttree.cpp \
    contacttreemodel.cpp

include(../plugin_pro.pri)

DEPENDPATH += . GeneratedFiles

QT += gui

# Input
HEADERS += autoreconnect.h autoreconnectoptions.h \
	../../include/options_i.h
SOURCES += autoreconnect.cpp autoreconnectoptions.cpp
FORMS += autoreconnectoptions.ui

include(../plugin_pro.pri)
DEPENDPATH += . \
    GeneratedFiles
QT += gui

# Input
HEADERS += styles.h \
    stylesoptions.h \
    ..\..\include\options_i.h
SOURCES += styles.cpp \
    stylesoptions.cpp
FORMS += stylesoptions.ui
RESOURCES += res.qrc
OTHER_FILES += Resources/default.ss

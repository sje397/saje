include(../plugin_pro.pri)
DEPENDPATH += . \
    GeneratedFiles
QT += gui

# Input
HEADERS += mainwin.h \
    main_window.h \
    mainwinoptions.h \
    ../../include/main_window_i.h \
    ../../include/options_i.h \
    ../../include/menus_i.h
FORMS += mainwin.ui \
    mainwinoptions.ui
SOURCES += mainwin.cpp \
    main_window.cpp \
    mainwinoptions.cpp

######################################################################
# Automatically generated by qmake (2.00a) ??? ??? 4 18:00:38 2006
######################################################################

TEMPLATE = app
TARGET = leechcraft
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = .
win32:DESTDIR = ./

# Input
TRANSLATIONS = leechcraft_ru.ts
FORMS += changelogdialog.ui
HEADERS += mainwindow.h \
	   core.h \
	   pluginmanager.h \
	   interfaces/interfaces.h \
	   interfaces/structures.h \
	   plugininfo.h \
	   pluginlisttablewidgeticon.h \
	   changelogdialog.h \
	   logshower.h
SOURCES += mainwindow.cpp \
	   core.cpp \
	   main.cpp \
	   pluginmanager.cpp \
	   plugininfo.cpp \
	   pluginlisttablewidgeticon.cpp \
	   changelogdialog.cpp \
	   logshower.cpp
RESOURCES = resources.qrc
CONFIG  += release threads warn_on
LIBS    += -lexceptions -lplugininterface -lsettingsdialog
win32:LIBS += -L.

IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_vgrabber)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	vgrabber.cpp
	findproxy.cpp
	categoriesselector.cpp
	categorymodifier.cpp
	xmlsettingsmanager.cpp
	audiofindproxy.cpp
	videofindproxy.cpp
	)
SET (HEADERS
	vgrabber.h
	findproxy.h
	categoriesselector.h
	categorymodifier.h
	xmlsettingsmanager.h
	audiofindproxy.h
	videofindproxy.h
	)
SET (FORMS
	categoriesselector.ui
	categorymodifier.ui
	)
SET (RESOURCES vgrabberresources.qrc)
CreateTrs ("vgrabber" "ar;de;en;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_vgrabber SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_vgrabber
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_vgrabber DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES vgrabbersettings.xml DESTINATION ${LC_SETTINGS_DEST})

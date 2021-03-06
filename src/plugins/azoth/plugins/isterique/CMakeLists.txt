CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_isterique)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
SET (ISTERIQUE_SRCS
	isterique.cpp
	xmlsettingsmanager.cpp
	)
SET (ISTERIQUE_HEADERS
	isterique.h
	xmlsettingsmanager.h
	)
SET (ISTERIQUE_RESOURCES
	isterique.qrc
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (ISTERIQUE_MOC_SRCS ${ISTERIQUE_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (ISTERIQUE_RCCS ${ISTERIQUE_RESOURCES})
CreateTrs ("azoth_isterique" "en;ru_RU;uk_UA" ISTERIQUE_COMPILED_TRANSLATIONS)

ADD_LIBRARY (leechcraft_azoth_isterique SHARED
	${ISTERIQUE_COMPILED_TRANSLATIONS}
	${ISTERIQUE_SRCS}
	${ISTERIQUE_MOC_SRCS}
	${ISTERIQUE_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_isterique
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_isterique DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${ISTERIQUE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES azothisteriquesettings.xml DESTINATION ${LC_SETTINGS_DEST})

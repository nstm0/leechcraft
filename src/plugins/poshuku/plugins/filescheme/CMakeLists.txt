INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FILESCHEME_SRCS
	filescheme.cpp
	schemereply.cpp
	)
SET (FILESCHEME_HEADERS
	filescheme.h
	schemereply.h
	)
SET (FILESCHEME_RESOURCES
	poshukufileschemeresources.qrc
	)
CreateTrs ("poshuku_filescheme" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" FILESCHEME_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (FILESCHEME_MOC_SRCS ${FILESCHEME_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (FILESCHEME_RCCS ${FILESCHEME_RESOURCES})
ADD_LIBRARY (leechcraft_poshuku_filescheme SHARED
	${FILESCHEME_COMPILED_TRANSLATIONS}
	${FILESCHEME_SRCS}
	${FILESCHEME_MOC_SRCS}
	${FILESCHEME_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_filescheme
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poshuku_filescheme DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${FILESCHEME_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

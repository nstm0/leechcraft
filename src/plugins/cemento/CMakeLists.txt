CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_cemento)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	cemento.cpp
	)
SET (HEADERS
	cemento.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (leechcraft_cemento SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_cemento
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_cemento DESTINATION ${LC_PLUGINS_DEST})

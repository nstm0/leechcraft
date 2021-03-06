CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_sb2)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (QJSON REQUIRED)

SET (QT_USE_QTDECLARATIVE TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
SET (SRCS
	sb2.cpp
	sbview.cpp
	viewmanager.cpp
	widthiconprovider.cpp
	desaturateeffect.cpp
	quarkproxy.cpp
	quarkmanager.cpp
	quarksettingsmanager.cpp
	tablistview.cpp
	tabunhidelistview.cpp
	themeimageprovider.cpp
	launcherdroparea.cpp
	launchercomponent.cpp
	lcmenucomponent.cpp
	traycomponent.cpp
	)

ADD_LIBRARY (leechcraft_sb2 SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_sb2
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_sb2 DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY share/qml/sb2 DESTINATION ${LC_QML_DEST})
INSTALL (DIRECTORY share/qml/common DESTINATION ${LC_QML_DEST})

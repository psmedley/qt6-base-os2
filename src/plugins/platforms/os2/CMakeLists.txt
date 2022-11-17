# Generated from os2.pro.

#####################################################################
## QOS2IntegrationPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QOS2IntegrationPlugin
    OUTPUT_NAME qos2
    PLUGIN_TYPE platforms
    DEFAULT_IF ${QT_QPA_DEFAULT_PLATFORM} MATCHES os2 # special case
    SOURCES
        main.cpp
        qos2backingstore.cpp
        qos2clipboard.cpp
        qos2context.cpp
        qos2cursor.cpp
        qos2integration.cpp
        qos2keymapper.cpp
        qos2mime.cpp
        qos2screen.cpp
        qos2theme.cpp
        qos2window.cpp

    DEFINES
        QT_NO_CAST_FROM_ASCII
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)


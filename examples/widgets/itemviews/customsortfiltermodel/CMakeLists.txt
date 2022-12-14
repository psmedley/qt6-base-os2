# Generated from customsortfiltermodel.pro.

cmake_minimum_required(VERSION 3.16)
project(customsortfiltermodel LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/itemviews/customsortfiltermodel")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(customsortfiltermodel
    filterwidget.cpp filterwidget.h
    main.cpp
    mysortfilterproxymodel.cpp mysortfilterproxymodel.h
    window.cpp window.h
)
set_target_properties(customsortfiltermodel PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(customsortfiltermodel PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)


# Resources:
set(customsortfiltermodel_resource_files
    "images/find.png"
)

qt6_add_resources(customsortfiltermodel "customsortfiltermodel"
    PREFIX
        "/"
    FILES
        ${customsortfiltermodel_resource_files}
)

install(TARGETS customsortfiltermodel
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

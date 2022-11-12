# Generated from printsupport.pro.

if(QT_FEATURE_cups AND (UNIX OR OS2) AND NOT APPLE)
    add_subdirectory(cups)
endif()

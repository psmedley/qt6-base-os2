# Generated from qplugin.pro.

add_subdirectory(invalidplugin)
add_subdirectory(debugplugin)
add_subdirectory(releaseplugin)

qt_internal_add_test(tst_qplugin
    SOURCES
        tst_qplugin.cpp
    LIBRARIES
        Qt::CorePrivate
)

if(NOT ANDROID)
    # Collect test data
    file(GLOB_RECURSE test_data_glob
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        plugins/*)
    list(APPEND test_data ${test_data_glob})
    set_target_properties(tst_qplugin PROPERTIES TESTDATA "${test_data}")
else()
    # On Android the plugins must be located in the libs subdir of the APK.
    # Use QT_ANDROID_EXTRA_LIBS to achieve that.
    set(plugins
        invalidplugin
        debugplugin
        releaseplugin
    )
    set(extra_libs)
    foreach(plugin IN LISTS plugins)
        list(APPEND extra_libs
            "${CMAKE_CURRENT_BINARY_DIR}/plugins/lib${plugin}_${CMAKE_ANDROID_ARCH_ABI}.so")
    endforeach()
    set_target_properties(tst_qplugin PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${extra_libs}"
    )
endif()

target_compile_definitions(tst_qplugin PRIVATE CMAKE_BUILD=1)

add_dependencies(tst_qplugin invalidplugin debugplugin releaseplugin)

if(ANDROID)
    qt6_android_generate_deployment_settings(tst_qplugin)
endif()

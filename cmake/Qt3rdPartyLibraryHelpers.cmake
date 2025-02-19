macro(qt_internal_get_add_library_option_args option_args)
    set(${option_args}
        SHARED
        STATIC
        MODULE
        INTERFACE
        )
endmacro()

# Helper to create a library using the public _qt_internal_add_library function.
#
# The difference to _qt_internal_add_library is that MODULE is replaced with STATIC in a static
# Qt build.
# Everything else is just prepation for option validating.
function(qt_internal_add_common_qt_library_helper target)
    qt_internal_get_add_library_option_args(option_args)
    qt_parse_all_arguments(arg "qt_internal_add_common_qt_library_helper"
        "${option_args}"
        ""
        ""
        ${ARGN}
    )

    if(arg_SHARED)
        set(arg_SHARED SHARED)
    else()
        set(arg_SHARED "")
    endif()

    if(arg_MODULE)
        set(arg_MODULE MODULE)
    else()
        set(arg_MODULE "")
    endif()

    if(arg_STATIC)
        set(arg_STATIC STATIC)
    else()
        set(arg_STATIC "")
    endif()

    if(arg_INTERFACE)
        set(arg_INTERFACE INTERFACE)
    else()
        set(arg_INTERFACE "")
    endif()

    if(arg_MODULE AND NOT BUILD_SHARED_LIBS)
        set(arg_MODULE STATIC)
    endif()

    _qt_internal_add_library(${target} ${arg_STATIC} ${arg_SHARED} ${arg_MODULE} ${arg_INTERFACE})
    qt_internal_mark_as_internal_library(${target})
endfunction()

# Wrapper function to create a regular cmake target and forward all the
# arguments collected by the conversion script.
function(qt_internal_add_cmake_library target)
    qt_internal_get_add_library_option_args(option_args)
    set(single_args
        OUTPUT_DIRECTORY
        ARCHIVE_INSTALL_DIRECTORY
        INSTALL_DIRECTORY
        OS2_SHORT_NAME
    )
    set(multi_args
        ${__default_private_args}
        ${__default_public_args}
    )

    qt_parse_all_arguments(arg "qt_add_cmake_library"
        "${option_args}"
        "${single_args}"
        "${multi_args}"
        ${ARGN}
    )

    qt_remove_args(library_helper_args
        ARGS_TO_REMOVE
            ${single_args}
            ${multi_args}
        ALL_ARGS
            ${option_args}
            ${single_args}
            ${multi_args}
        ARGS
            ${ARGN}
    )

    qt_internal_add_common_qt_library_helper(${target} ${library_helper_args})

    qt_skip_warnings_are_errors_when_repo_unclean("${target}")

    if (arg_OUTPUT_DIRECTORY)
        set_target_properties(${target} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${arg_OUTPUT_DIRECTORY}
            RUNTIME_OUTPUT_DIRECTORY ${arg_OUTPUT_DIRECTORY}
            LIBRARY_OUTPUT_DIRECTORY ${arg_OUTPUT_DIRECTORY}
        )
    endif()

if (OS2)
    if(arg_OS2_SHORT_NAME)
       set_target_properties(${target} PROPERTIES
            TARGET_SHORT ${arg_OS2_SHORT_NAME}
       )
    endif()
endif()

    qt_internal_extend_target("${target}"
        SOURCES ${arg_SOURCES}
        INCLUDE_DIRECTORIES
            ${arg_INCLUDE_DIRECTORIES}
        PUBLIC_INCLUDE_DIRECTORIES
            ${arg_PUBLIC_INCLUDE_DIRECTORIES}
        PUBLIC_DEFINES
            ${arg_PUBLIC_DEFINES}
        DEFINES
            ${arg_DEFINES}
        PUBLIC_LIBRARIES ${arg_PUBLIC_LIBRARIES}
        LIBRARIES ${arg_LIBRARIES} Qt::PlatformCommonInternal
        COMPILE_OPTIONS ${arg_COMPILE_OPTIONS}
        PUBLIC_COMPILE_OPTIONS ${arg_PUBLIC_COMPILE_OPTIONS}
        LINK_OPTIONS ${arg_LINK_OPTIONS}
        PUBLIC_LINK_OPTIONS ${arg_PUBLIC_LINK_OPTIONS}
        MOC_OPTIONS ${arg_MOC_OPTIONS}
        ENABLE_AUTOGEN_TOOLS ${arg_ENABLE_AUTOGEN_TOOLS}
        DISABLE_AUTOGEN_TOOLS ${arg_DISABLE_AUTOGEN_TOOLS}
        OS2_SHORT_NAME ${arg_OS2_SHORT_NAME}
    )
endfunction()

# This function replaces qmake's qt_helper_lib feature. It is intended to
# compile 3rdparty libraries as part of the build.
#
function(qt_internal_add_3rdparty_library target)
    qt_internal_get_add_library_option_args(library_option_args)
    set(option_args
        EXCEPTIONS
        INSTALL
        SKIP_AUTOMOC
        )
    set(single_args
        OUTPUT_DIRECTORY
        QMAKE_LIB_NAME
    )
    set(multi_args
        ${__default_private_args}
        ${__default_public_args}
    )

    qt_parse_all_arguments(arg "qt_internal_add_3rdparty_library"
        "${library_option_args};${option_args}"
        "${single_args}"
        "${multi_args}"
        ${ARGN}
    )

    qt_remove_args(library_helper_args
        ARGS_TO_REMOVE
            ${option_args}
            ${single_args}
            ${multi_args}
        ALL_ARGS
            ${library_option_args}
            ${option_args}
            ${single_args}
            ${multi_args}
        ARGS
            ${ARGN}
    )

    qt_internal_add_common_qt_library_helper(${target} ${library_helper_args})

    if(NOT arg_INTERFACE)
        qt_set_common_target_properties(${target})
    endif()

    set_target_properties(${target} PROPERTIES
        _qt_module_interface_name "${target}"
    )
    qt_internal_add_qt_repo_known_module(${target})
    qt_internal_add_target_aliases(${target})
    _qt_internal_apply_strict_cpp(${target})

    qt_skip_warnings_are_errors_when_repo_unclean("${target}")

    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}"
        RUNTIME_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_BINDIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${QT_BUILD_DIR}/${INSTALL_LIBDIR}"
        VERSION ${PROJECT_VERSION}
if (NOT OS2)
        SOVERSION ${PROJECT_VERSION_MAJOR}
endif()
        _qt_module_is_3rdparty_library TRUE
        _qt_module_skip_depends_include TRUE
    )
    set_property(TARGET "${target}"
                 APPEND PROPERTY EXPORT_PROPERTIES _qt_module_is_3rdparty_library)
    set_property(TARGET "${target}"
                 APPEND PROPERTY EXPORT_PROPERTIES _qt_module_skip_depends_include)

    qt_handle_multi_config_output_dirs("${target}")

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${INSTALL_CMAKE_NAMESPACE}${target}"
    )

    if(NOT arg_SKIP_AUTOMOC)
        qt_autogen_tools_initial_setup(${target})
    endif()

    if(NOT arg_EXCEPTIONS AND NOT arg_INTERFACE)
        qt_internal_set_exceptions_flags("${target}" FALSE)
    elseif(arg_EXCEPTIONS)
        qt_internal_set_exceptions_flags("${target}" TRUE)
    endif()

    qt_internal_extend_target("${target}"
        SOURCES ${arg_SOURCES}
        INCLUDE_DIRECTORIES
            ${arg_INCLUDE_DIRECTORIES}
        PUBLIC_INCLUDE_DIRECTORIES
            ${arg_PUBLIC_INCLUDE_DIRECTORIES}
        PUBLIC_DEFINES
            ${arg_PUBLIC_DEFINES}
        DEFINES
            ${arg_DEFINES}
        PUBLIC_LIBRARIES ${arg_PUBLIC_LIBRARIES}
        LIBRARIES ${arg_LIBRARIES} Qt::PlatformModuleInternal
        COMPILE_OPTIONS ${arg_COMPILE_OPTIONS}
        PUBLIC_COMPILE_OPTIONS ${arg_PUBLIC_COMPILE_OPTIONS}
        LINK_OPTIONS ${arg_LINK_OPTIONS}
        PUBLIC_LINK_OPTIONS ${arg_PUBLIC_LINK_OPTIONS}
        MOC_OPTIONS ${arg_MOC_OPTIONS}
        ENABLE_AUTOGEN_TOOLS ${arg_ENABLE_AUTOGEN_TOOLS}
        DISABLE_AUTOGEN_TOOLS ${arg_DISABLE_AUTOGEN_TOOLS}
    )

    if(NOT BUILD_SHARED_LIBS OR arg_INSTALL)
        qt_generate_3rdparty_lib_pri_file("${target}" "${arg_QMAKE_LIB_NAME}" pri_file)
        if(pri_file)
            qt_install(FILES "${pri_file}" DESTINATION "${INSTALL_MKSPECSDIR}/modules")
        endif()

        set(path_suffix "${INSTALL_CMAKE_NAMESPACE}${target}")
        qt_path_join(config_build_dir ${QT_CONFIG_BUILD_DIR} ${path_suffix})
        qt_path_join(config_install_dir ${QT_CONFIG_INSTALL_DIR} ${path_suffix})
        set(export_name "${INSTALL_CMAKE_NAMESPACE}${target}Targets")

        qt_internal_get_min_new_policy_cmake_version(min_new_policy_version)
        qt_internal_get_max_new_policy_cmake_version(max_new_policy_version)
        configure_package_config_file(
            "${QT_CMAKE_DIR}/Qt3rdPartyLibraryConfig.cmake.in"
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}Config.cmake"
            INSTALL_DESTINATION "${config_install_dir}"
        )

        write_basic_package_version_file(
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}ConfigVersionImpl.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
        )
        qt_internal_write_qt_package_version_file(
            "${INSTALL_CMAKE_NAMESPACE}${target}"
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}ConfigVersion.cmake"
        )

        qt_install(FILES
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}Config.cmake"
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}ConfigVersion.cmake"
            "${config_build_dir}/${INSTALL_CMAKE_NAMESPACE}${target}ConfigVersionImpl.cmake"
            DESTINATION "${config_install_dir}"
            COMPONENT Devel
        )

        qt_install(TARGETS ${target}
            EXPORT "${export_name}"
            RUNTIME DESTINATION ${INSTALL_BINDIR}
            LIBRARY DESTINATION ${INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${INSTALL_LIBDIR}
        )

        qt_install(EXPORT ${export_name}
            NAMESPACE "${QT_CMAKE_EXPORT_NAMESPACE}::"
            DESTINATION "${config_install_dir}"
        )

        qt_internal_export_additional_targets_file(
            TARGETS ${target}
            EXPORT_NAME_PREFIX ${INSTALL_CMAKE_NAMESPACE}${target}
            CONFIG_INSTALL_DIR "${config_install_dir}"
        )

        qt_internal_export_modern_cmake_config_targets_file(
            TARGETS ${target}
            EXPORT_NAME_PREFIX ${INSTALL_CMAKE_NAMESPACE}${target}
            CONFIG_INSTALL_DIR "${config_install_dir}"
        )

        set(debug_install_dir "${INSTALL_LIBDIR}")
        if (MINGW)
            set(debug_install_dir "${INSTALL_BINDIR}")
        endif()
        qt_enable_separate_debug_info(${target} "${debug_install_dir}")
        qt_internal_install_pdb_files(${target} "${INSTALL_LIBDIR}")
    endif()
endfunction()

function(qt_install_3rdparty_library_wrap_config_extra_file target)
    if(TARGET "${target}")
        set(use_bundled "ON")
    else()
        set(use_bundled "OFF")
    endif()

    set(QT_USE_BUNDLED_${target} "${use_bundled}" CACHE INTERNAL "")
    set(extra_cmake_code "set(QT_USE_BUNDLED_${target} ${use_bundled} CACHE INTERNAL \"\")")
    configure_file(
        "${QT_CMAKE_DIR}/QtFindWrapConfigExtra.cmake.in"
        "${QT_CONFIG_BUILD_DIR}/${INSTALL_CMAKE_NAMESPACE}/FindWrap${target}ConfigExtra.cmake"
         @ONLY
    )

    qt_install(FILES
        "${QT_CONFIG_BUILD_DIR}/${INSTALL_CMAKE_NAMESPACE}/FindWrap${target}ConfigExtra.cmake"
        DESTINATION "${QT_CONFIG_INSTALL_DIR}/${INSTALL_CMAKE_NAMESPACE}"
        COMPONENT Devel
    )
endfunction()

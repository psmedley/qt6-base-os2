# Install script for directory: /home/psmedley/qt6-base-os2/src/opengl

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local/Qt-6.4.2")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtOpenGL" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/qtopenglexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/opengl/meta_types/qt6opengl_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6OpenGL/Qt6OpenGLConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6OpenGL/Qt6OpenGLConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6OpenGL/Qt6OpenGLConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6OpenGL.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6OpenGL.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
           NEW_RPATH "\$ORIGIN")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6OpenGL.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6OpenGL.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtOpenGL/6.4.2/QtOpenGL/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/opengl/qopengl2pexvertexarray_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglcompositor_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglcompositorbackingstore_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglcustomshaderstage_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglengineshadermanager_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglengineshadersource_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglframebufferobject_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglgradientcache_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglpaintdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglpaintengine_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglqueryhelper_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglshadercache_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltexture_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltexturecache_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltextureglyphcache_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltexturehelper_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltextureuploader_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglversionfunctions_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglvertexarrayobject_p.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qvkconvenience_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtOpenGL" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/qtopenglexports.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglbuffer.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengldebug.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglframebufferobject.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_0.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_1.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_2.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_3.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_4.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_1_5.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_2_0.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_2_1.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_0.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_1.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_2_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_2_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_3_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_3_3_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_0_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_0_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_1_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_1_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_2_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_2_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_3_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_3_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_4_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_4_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_5_compatibility.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_4_5_core.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglfunctions_es2.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglpaintdevice.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglpixeltransferoptions.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglshaderprogram.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltexture.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltextureblitter.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopengltimerquery.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglversionfunctions.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglversionfunctionsfactory.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglversionprofile.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglvertexarrayobject.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qopenglwindow.h"
    "/home/psmedley/qt6-base-os2/src/opengl/qtopenglglobal.h"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLBuffer"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLDebugMessage"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLDebugLogger"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFramebufferObject"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFramebufferObjectFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_0"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_1"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_2"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_3"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_4"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_1_5"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_2_0"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_2_1"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_0"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_1"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_2_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_2_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_3_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_3_3_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_0_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_0_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_1_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_1_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_2_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_2_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_3_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_3_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_4_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_4_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_5_Compatibility"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_4_5_Core"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLFunctions_ES2"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLPaintDevice"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLPixelTransferOptions"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLShader"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLShaderProgram"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLTexture"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLTextureBlitter"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLTimerQuery"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLTimeMonitor"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLVersionFunctions"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLVersionFunctionsFactory"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLVersionProfile"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLVertexArrayObject"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QOpenGLWindow"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/qtopenglversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QtOpenGLVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QtOpenGL"
    "/home/psmedley/qt6-base-os2/build/include/QtOpenGL/QtOpenGLDepends"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/opengl/CMakeFiles/Export/8430990577e61c35e6352904ac17f5ae/Qt6OpenGLTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/opengl/CMakeFiles/Export/8430990577e61c35e6352904ac17f5ae/Qt6OpenGLTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/opengl/CMakeFiles/Export/8430990577e61c35e6352904ac17f5ae/Qt6OpenGLTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/opengl/CMakeFiles/Export/8430990577e61c35e6352904ac17f5ae/Qt6OpenGLVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL/Qt6OpenGLVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/opengl/CMakeFiles/Export/8430990577e61c35e6352904ac17f5ae/Qt6OpenGLVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/OpenGL.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_OpenGL")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtopengl" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtopengl/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_OpenGL")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtopengl.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6OpenGL" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6OpenGL/Qt6OpenGLAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_opengl.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_opengl_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6OpenGL.pc")
endif()


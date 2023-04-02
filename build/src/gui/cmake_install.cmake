# Install script for directory: /home/psmedley/qt6-base-os2/src/gui

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/include/QtGui/qtguiexports.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui/6.4.2/QtGui/qpa" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qplatformaccessibility.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qplatformpixmap.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformclipboard.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformcursor.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformdialoghelper.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformdrag.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformgraphicsbuffer.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformgraphicsbufferhelper.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatforminputcontext.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatforminputcontext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatforminputcontextfactory_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatforminputcontextplugin_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformintegration.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformintegrationfactory_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformintegrationplugin.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformmenu.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformmenu_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformnativeinterface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformoffscreensurface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformopenglcontext.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformscreen.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformscreen_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformservices.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformsessionmanager.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformsharedgraphicscache.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformsurface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformsystemtrayicon.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformtheme.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformtheme_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformthemefactory_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformthemeplugin.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformwindow.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qplatformwindow_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindowsysteminterface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindowsysteminterface_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qplatformbackingstore.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qplatformfontdatabase.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qplatformvulkaninstance.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE OPTIONAL RENAME "qvulkanfunctions.h" FILES "/home/psmedley/qt6-base-os2/build/src/gui/vulkan/qvulkanfunctions.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE OPTIONAL FILES "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanFunctions")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE OPTIONAL FILES "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanDeviceFunctions")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui/6.4.2/QtGui/private" TYPE FILE OPTIONAL RENAME "qvulkanfunctions_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/gui/vulkan/qvulkanfunctions_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE OPTIONAL RENAME "qtgui-config.h" FILES "/home/psmedley/qt6-base-os2/build/src/gui/qtgui-config.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui/6.4.2/QtGui/private" TYPE FILE OPTIONAL RENAME "qtgui-config_p.h" FILES "/home/psmedley/qt6-base-os2/build/src/gui/qtgui-config_p.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/metatypes" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/gui/meta_types/qt6gui_release_metatypes.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6GuiConfig.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6GuiConfigVersion.cmake"
    "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6GuiConfigVersionImpl.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so.6"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "\$ORIGIN")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Gui.so.6.4.2"
    "/home/psmedley/qt6-base-os2/build/lib/libQt6Gui.so.6"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so.6.4.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so.6"
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so"
         RPATH "\$ORIGIN")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/psmedley/qt6-base-os2/build/lib/libQt6Gui.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so"
         OLD_RPATH "/home/psmedley/qt6-base-os2/build/lib:"
         NEW_RPATH "\$ORIGIN")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/psmedley/qt6-base-os2/build/./libexec/qt-internal-strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libQt6Gui.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui/6.4.2/QtGui/private" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessiblebridgeutils_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessiblecache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qabstractfileiconengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qabstractfileiconprovider_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qbmphandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qicon_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qiconloader_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimage_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimagepixmapcleanuphooks_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimagereaderwriterhelpers_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpaintengine_pic_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpicture_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpixmap_blitter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpixmap_raster_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpixmapcache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpnghandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qppmhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qxbmhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qxpmhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/itemmodels/qfileinfogatherer_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/itemmodels/qfilesystemmodel_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/itemmodels/qstandarditemmodel_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qaction_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qactiongroup_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qcursor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qdnd_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qevent_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qeventpoint_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qguiapplication_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qhighdpiscaling_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputdevicemanager_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputdevicemanager_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputmethod_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinternalmimedata_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qkeymapper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qkeysequence_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qoffscreensurface_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qopenglcontext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpaintdevicewindow_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpointingdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qscreen_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qsessionmanager_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qshapedpixmapdndwindow_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qshortcut_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qshortcutmap_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qsimpledrag_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qt_gui_pch.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qtguiglobal_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindow_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopengl_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopenglextensions_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopenglprogrambinarycache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qbackingstoredefaultcompositor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qbackingstorerhisupport_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qbezier_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qblendfunctions_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qblittable_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolormatrix_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolorspace_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortransferfunction_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortransfertable_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortransform_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortrc_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortrclut_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcoregraphics_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcosmeticstroker_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcssutil_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdatabuffer_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdrawhelper_mips_dsp_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdrawhelper_neon_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdrawhelper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdrawhelper_x86_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qdrawingprimitive_sse2_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qemulationpaintengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qfixed_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qgrayraster_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qicc_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qimagescale_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qmath_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qmemrotate_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qoutlinemapper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpagedpaintdevice_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpageranges_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintengine_blitter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintengine_raster_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintengineex_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpainter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpainterpath_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpathclipper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpathsimplifier_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpdf_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpen_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpixellayout_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrasterbackingstore_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrasterdefs_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrasterizer_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrbtree_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrgba64_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrhibackingstore_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qstroker_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qt_mips_asm_dsp_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qtextureglyphcache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qtriangulatingstroker_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qtriangulator_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qvectorpath_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/cs_tdr_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhi_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhi_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhid3d11_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhid3d11_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhigles2_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhigles2_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhimetal_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhimetal_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhinull_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhinull_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhivulkan_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhivulkan_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qrhivulkanext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qshader_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qshader_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qshaderdescription_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/qshaderdescription_p_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/rhi/vs_test_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qabstracttextdocumentlayout_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qcssparser_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qdistancefield_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfont_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontengineglyphcache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontsubset_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfragmentmap_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qglyphrun_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qharfbuzzng_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qinputcontrol_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qrawfont_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qstatictext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextcursor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocument_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocumentfragment_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocumentlayout_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextformat_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtexthtmlparser_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextimagehandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextmarkdownimporter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextmarkdownwriter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextobject_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextodfwriter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtexttable_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qzipreader_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qzipwriter_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qabstractlayoutstyleinfo_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qastchandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qedidparser_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qedidvendortable_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qgridlayoutengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qhexstring_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qktxhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qlayoutpolicy_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qpkmhandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qtexturefiledata_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qtexturefilehandler_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qtexturefilereader_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qundostack_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qbasicvulkanplatforminstance_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qvulkandefaultinstance_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qvulkaninstance_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qvulkanwindow_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/atspiadaptor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/dbusconnection_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/qspi_constant_mappings_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/qspi_struct_marshallers_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/qspiaccessiblebridge_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/qspiapplicationadaptor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/linux/qspidbuscache_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/darwin/qapplekeymapper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/darwin/qmacmime_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/os2/qos2guieventdispatcher_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qeventdispatcher_glib_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qgenericunixeventdispatcher_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qgenericunixservices_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qgenericunixthemes_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qtx11extras_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qunixeventdispatcher_qpa_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/qxkbcommon_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/wasm/qwasmlocalfileaccess_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/windows/qwindowsguieventdispatcher_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/windows/qwindowsmime_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/coretext/qcoretextfontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/coretext/qfontengine_coretext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/freetype/qfontengine_ft_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/freetype/qfreetypefontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/unix/qfontconfigdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/unix/qfontenginemultifontconfig_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/unix/qgenericunixfontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsdirectwritefontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsfontdatabase_ft_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsfontdatabase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsfontdatabasebase_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsfontengine_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsfontenginedirectwrite_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/windows/qwindowsnativeimage_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/qwindowsuiawrapper_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiaattributeids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiaclientinterfaces_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiacontroltypeids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiaerrorids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiaeventids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiageneralids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiapatternids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiapropertyids_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiaserverinterfaces_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/windows/apisupport/uiatypes_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/egl/qeglconvenience_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/egl/qeglpbuffer_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/egl/qeglplatformcontext_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/egl/qeglstreamconvenience_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/egl/qt_egl_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/platform/unix/qglxconvenience_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusmenuadaptor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusmenubar_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusmenuconnection_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusmenuregistrarproxy_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusmenutypes_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbusmenu/qdbusplatformmenu_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbustray/qdbustrayicon_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbustray/qdbustraytypes_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbustray/qstatusnotifieritemadaptor_p.h"
    "/home/psmedley/qt6-base-os2/src/gui/platform/unix/dbustray/qxdgnotificationproxy_p.h"
    "/home/psmedley/qt6-base-os2/build/src/gui/qtgui-config_p.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtGui" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/include/QtGui/qtguiexports.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessible.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessible_base.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessiblebridge.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessibleobject.h"
    "/home/psmedley/qt6-base-os2/src/gui/accessible/qaccessibleplugin.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qabstractfileiconprovider.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qbitmap.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qicon.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qiconengine.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qiconengineplugin.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimage.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimageiohandler.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimagereader.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qimagewriter.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qmovie.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpicture.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpixmap.h"
    "/home/psmedley/qt6-base-os2/src/gui/image/qpixmapcache.h"
    "/home/psmedley/qt6-base-os2/src/gui/itemmodels/qfilesystemmodel.h"
    "/home/psmedley/qt6-base-os2/src/gui/itemmodels/qstandarditemmodel.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qaction.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qactiongroup.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qclipboard.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qcursor.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qdrag.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qevent.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qeventpoint.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qgenericplugin.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qgenericpluginfactory.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qguiapplication.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qguiapplication_platform.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputdevice.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qinputmethod.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qkeysequence.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qoffscreensurface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qoffscreensurface_platform.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qopenglcontext.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qopenglcontext_platform.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpaintdevicewindow.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpalette.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpixelformat.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qpointingdevice.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qrasterwindow.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qscreen.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qsessionmanager.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qshortcut.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qstylehints.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qsurface.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qsurfaceformat.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qtestsupport_gui.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qtguiglobal.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindow.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindowdefs.h"
    "/home/psmedley/qt6-base-os2/src/gui/kernel/qwindowdefs_win.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qgenericmatrix.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qmatrix4x4.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qquaternion.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qvector2d.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qvector3d.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qvector4d.h"
    "/home/psmedley/qt6-base-os2/src/gui/math3d/qvectornd.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopengl.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopengles2ext.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopenglext.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopenglextrafunctions.h"
    "/home/psmedley/qt6-base-os2/src/gui/opengl/qopenglfunctions.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qbackingstore.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qbrush.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolor.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolorspace.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qcolortransform.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpagedpaintdevice.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpagelayout.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpageranges.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpagesize.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintdevice.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpaintengine.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpainter.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpainterpath.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpdfwriter.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpen.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qpolygon.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qregion.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrgb.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrgba64.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qrgbafloat.h"
    "/home/psmedley/qt6-base-os2/src/gui/painting/qtransform.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qabstracttextdocumentlayout.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfont.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontdatabase.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontinfo.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qfontmetrics.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qglyphrun.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qrawfont.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qstatictext.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qsyntaxhighlighter.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextcursor.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocument.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocumentfragment.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextdocumentwriter.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextformat.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextlayout.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextlist.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextobject.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtextoption.h"
    "/home/psmedley/qt6-base-os2/src/gui/text/qtexttable.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qdesktopservices.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qundogroup.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qundostack.h"
    "/home/psmedley/qt6-base-os2/src/gui/util/qvalidator.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qvulkaninstance.h"
    "/home/psmedley/qt6-base-os2/src/gui/vulkan/qvulkanwindow.h"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessible"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleEditableTextInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleValueInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTableCellInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTableInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleActionInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleImageInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleHyperlinkInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleStateChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextCursorEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextSelectionEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextInsertEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextRemoveEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTextUpdateEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleValueChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleTableModelChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleBridge"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleBridgePlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleObject"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessibleApplication"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAccessiblePlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAbstractFileIconProvider"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QBitmap"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QIcon"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QIconEngine"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QIconEnginePlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImageCleanupFunction"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImage"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImageIOHandler"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImageIOPlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImageReader"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QImageWriter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMovie"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPicture"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPixmap"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPixmapCache"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFileSystemModel"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QStandardItem"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QStandardItemModel"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAction"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QActionGroup"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QClipboard"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QCursor"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDrag"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QInputEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPointerEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QSinglePointEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QEnterEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMouseEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QHoverEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWheelEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTabletEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QNativeGestureEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QKeyEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFocusEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPaintEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMoveEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QExposeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPlatformSurfaceEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QResizeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QCloseEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QIconDragEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QShowEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QHideEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QContextMenuEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QInputMethodEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QInputMethodQueryEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDropEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDragMoveEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDragEnterEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDragLeaveEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QHelpEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QStatusTipEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWhatsThisClickedEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QActionEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFileOpenEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QToolBarChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QShortcutEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWindowStateChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTouchEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QScrollPrepareEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QScrollEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QScreenOrientationChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QApplicationStateChangeEvent"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QtEvents"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QEventPoint"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGenericPlugin"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGenericPluginFactory"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGuiApplication"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QInputDevice"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QInputMethod"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QKeySequence"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOffscreenSurface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLContextGroup"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLContext"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPaintDeviceWindow"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPalette"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPixelFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPointingDeviceUniqueId"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPointingDevice"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRasterWindow"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QScreen"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QSessionManager"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QShortcut"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QStyleHints"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QSurface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QSurfaceFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWindow"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWidgetList"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWindowList"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWidgetMapper"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QWidgetSet"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGenericMatrix"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix2x2"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix2x3"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix2x4"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix3x2"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix3x3"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix3x4"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix4x2"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix4x3"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QMatrix4x4"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QQuaternion"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVector2D"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVector3D"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVector4D"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLExtraFunctions"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLExtraFunctionsPrivate"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLFunctions"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QOpenGLFunctionsPrivate"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QBackingStore"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QBrushDataPointerDeleter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QBrush"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QBrushData"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGradientStop"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGradientStops"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGradient"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QLinearGradient"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRadialGradient"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QConicalGradient"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QColor"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QColorSpace"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRgbaFloat16"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRgbaFloat32"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QColorTransform"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPagedPaintDevice"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPageLayout"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPageRanges"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPageSize"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPaintDevice"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextItem"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPaintEngine"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPaintEngineState"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPainter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPainterPath"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPainterPathStroker"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPdfWriter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPen"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPolygon"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QPolygonF"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRegion"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRgb"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRgba64"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTransform"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAbstractTextDocumentLayout"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextObjectInterface"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFont"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFontDatabase"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFontInfo"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFontMetrics"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QFontMetricsF"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QGlyphRun"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRawFont"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QStaticText"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QSyntaxHighlighter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextCursor"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QAbstractUndoItem"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextDocument"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextDocumentFragment"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextDocumentWriter"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextLength"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextCharFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextBlockFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextListFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextImageFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextFrameFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextTableFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextTableCellFormat"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextInlineObject"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextLayout"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextLine"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextList"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextObject"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextBlockGroup"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextFrameLayoutData"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextFrame"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextBlockUserData"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextBlock"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextFragment"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextOption"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextTableCell"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QTextTable"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDesktopServices"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QUndoGroup"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QUndoCommand"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QUndoStack"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QValidator"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QIntValidator"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QDoubleValidator"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QRegularExpressionValidator"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanLayer"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanExtension"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanInfoVector"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanInstance"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanWindowRenderer"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QVulkanWindow"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/qtguiversion.h"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QtGuiVersion"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QtGui"
    "/home/psmedley/qt6-base-os2/build/include/QtGui/QtGuiDepends"
    "/home/psmedley/qt6-base-os2/build/src/gui/qtgui-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/gui/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6GuiTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/gui/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6GuiTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/gui/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6GuiTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiVersionlessTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiVersionlessTargets.cmake"
         "/home/psmedley/qt6-base-os2/build/src/gui/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6GuiVersionlessTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiVersionlessTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui/Qt6GuiVersionlessTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/src/gui/CMakeFiles/Export/186cdab8ebd5e47f6ef8450c9fc81ba1/Qt6GuiVersionlessTargets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./modules" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./modules/Gui.json")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_html_docs_Gui")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc/qtgui" TYPE DIRECTORY FILES "/home/psmedley/qt6-base-os2/build/./doc/qtgui/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "_install_qch_docs_Gui")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./doc" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/./doc/qtgui.qch")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Qt6Gui" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/cmake/Qt6Gui/Qt6GuiAdditionalTargetInfo.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mkspecs/modules" TYPE FILE FILES
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_gui.pri"
    "/home/psmedley/qt6-base-os2/build/mkspecs/modules/qt_lib_gui_private.pri"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/psmedley/qt6-base-os2/build/lib/pkgconfig/Qt6Gui.pc")
endif()


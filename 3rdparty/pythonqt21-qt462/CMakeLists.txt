cmake_minimum_required(VERSION 2.8)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

#-----------------------------------------------------------------------------
project(PythonQt)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Python libraries

find_package(PythonLibs REQUIRED)
include_directories("${PYTHON_INCLUDE_DIR}")
add_definitions(-DPYTHONQT_USE_RELEASE_PYTHON_FALLBACK)

#-----------------------------------------------------------------------------
# Build options

foreach(qtlib core gui svg uitools xml)
  OPTION(PythonQt_Wrap_Qt${qtlib} "Make all of Qt${qtlib} available in python" ON)
endforeach()

option(PythonQt_DEBUG "Enable/Disable PythonQt debug output" OFF)
if(PythonQt_DEBUG)
  add_definitions(-DPYTHONQT_DEBUG)
else()
  remove_definitions(-DPYTHONQT_DEBUG)
endif()

#-----------------------------------------------------------------------------
# Setup Qt

set(minimum_required_qt_version "4.6.2")

find_package(Qt4)

if(QT4_FOUND)

  set(found_qt_version ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH})

  if(${found_qt_version} VERSION_LESS ${minimum_required_qt_version})
      message(FATAL_ERROR "error: PythonQt requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${found_qt_version}.")
  endif()
  
  # Enable required qt module
  foreach(qtlib sql uitools svg xml)
    string(TOUPPER ${qtlib} qtlib_uppercase)
    if (NOT ${QT_QT${qtlib_uppercase}_FOUND})
      message(FATAL_ERROR "QT_QT${${qtlib_uppercase} *not* FOUND - Try to disable PythonQt_Wrap_Qt${qtlib}")
    endif()
    set(QT_USE_QT${qtlib_uppercase} ${PythonQt_Wrap_Qt${qtlib}})
  endforeach()
  
  include(${QT_USE_FILE})
else()
  message(FATAL_ERROR "error: Qt4 was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable")
endif()

#-----------------------------------------------------------------------------
# Sources

set(sources
    src/PythonQtClassInfo.cpp
    src/PythonQtClassWrapper.cpp
    src/PythonQtConversion.cpp
    src/PythonQt.cpp
    src/PythonQtImporter.cpp
    src/PythonQtInstanceWrapper.cpp
    src/PythonQtMethodInfo.cpp
    src/PythonQtMisc.cpp
    src/PythonQtObjectPtr.cpp
    src/PythonQtQFileImporter.cpp
    src/PythonQtSignalReceiver.cpp
    src/PythonQtSlot.cpp
    src/PythonQtSignal.cpp
    src/PythonQtStdDecorators.cpp
    src/PythonQtStdIn.cpp
    src/PythonQtStdOut.cpp
    src/gui/PythonQtScriptingConsole.cpp
    
    generated_cpp/PythonQt_QtBindings.cpp

	generated_cpp/com_trolltech_qt_xml/com_trolltech_qt_xml_init.cpp
	generated_cpp/com_trolltech_qt_xml/com_trolltech_qt_xml0.cpp
	generated_cpp/com_trolltech_qt_xml/com_trolltech_qt_xml1.cpp
# 	generated_cpp/com_trolltech_qt_sql/com_trolltech_qt_sql0.cpp
# 	generated_cpp/com_trolltech_qt_sql/com_trolltech_qt_sql_init.cpp
# 	generated_cpp/com_trolltech_qt_webkit/com_trolltech_qt_webkit_init.cpp
# 	generated_cpp/com_trolltech_qt_webkit/com_trolltech_qt_webkit0.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui11.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui1.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui3.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui8.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui0.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui10.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui_init.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui5.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui4.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui6.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui7.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui2.cpp
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui9.cpp
	generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin0.cpp
	generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin_init.cpp
	generated_cpp/PythonQt_QtBindings.cpp
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core1.cpp
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.cpp
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core2.cpp
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core_init.cpp
# 	generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network_init.cpp
# 	generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network1.cpp
# 	generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network0.cpp
	generated_cpp/com_trolltech_qt_uitools/com_trolltech_qt_uitools0.cpp
	generated_cpp/com_trolltech_qt_uitools/com_trolltech_qt_uitools_init.cpp
# 	generated_cpp/com_trolltech_qt_opengl/com_trolltech_qt_opengl0.cpp
# 	generated_cpp/com_trolltech_qt_opengl/com_trolltech_qt_opengl_init.cpp
	generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin_init.cpp
	generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin0.cpp
# 	generated_cpp/com_trolltech_qt_xmlpatterns/com_trolltech_qt_xmlpatterns_init.cpp
# 	generated_cpp/com_trolltech_qt_xmlpatterns/com_trolltech_qt_xmlpatterns0.cpp
	generated_cpp/com_trolltech_qt_svg/com_trolltech_qt_svg0.cpp
	generated_cpp/com_trolltech_qt_svg/com_trolltech_qt_svg_init.cpp

    extensions/PythonQt_QtAll/PythonQt_QtAll.cpp
)

#-----------------------------------------------------------------------------
# List headers.  This is list is used for the install command.

set(headers
    src/PythonQtClassInfo.h
    src/PythonQtClassWrapper.h
    src/PythonQtConversion.h
    src/PythonQtCppWrapperFactory.h
    src/PythonQtDoc.h
    src/PythonQt.h
    src/PythonQtImporter.h
    src/PythonQtImportFileInterface.h
    src/PythonQtInstanceWrapper.h
    src/PythonQtMethodInfo.h
    src/PythonQtMisc.h
    src/PythonQtObjectPtr.h
    src/PythonQtQFileImporter.h
    src/PythonQtSignalReceiver.h
    src/PythonQtSlot.h
    src/PythonQtSignal.h
    src/PythonQtStdDecorators.h
    src/PythonQtStdIn.h
    src/PythonQtStdOut.h
    src/PythonQtSystem.h
    src/PythonQtVariants.h
    src/PythonQtPythonInclude.h
    generated_cpp/PythonQt_QtBindings.h
)

#-----------------------------------------------------------------------------
# Headers that should run through moc

set(moc_sources
    src/PythonQt.h
    src/PythonQtSignalReceiver.h
    src/PythonQtStdDecorators.h
    src/gui/PythonQtScriptingConsole.h

generated_cpp/com_trolltech_qt_xml/com_trolltech_qt_xml1.h
	generated_cpp/com_trolltech_qt_xml/com_trolltech_qt_xml0.h
# 	generated_cpp/com_trolltech_qt_sql/com_trolltech_qt_sql0.h
# 	generated_cpp/com_trolltech_qt_webkit/com_trolltech_qt_webkit0.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui1.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui2.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui10.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui4.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui8.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui11.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui5.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui3.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui7.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui9.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui6.h
	generated_cpp/com_trolltech_qt_gui/com_trolltech_qt_gui0.h
	generated_cpp/com_trolltech_qt_core_builtin/com_trolltech_qt_core_builtin0.h
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core1.h
	generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core2.h
# 	generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network0.h
# 	generated_cpp/com_trolltech_qt_network/com_trolltech_qt_network1.h
	generated_cpp/com_trolltech_qt_uitools/com_trolltech_qt_uitools0.h
# 	generated_cpp/com_trolltech_qt_opengl/com_trolltech_qt_opengl0.h
	generated_cpp/com_trolltech_qt_gui_builtin/com_trolltech_qt_gui_builtin0.h
# 	generated_cpp/com_trolltech_qt_xmlpatterns/com_trolltech_qt_xmlpatterns0.h
	generated_cpp/com_trolltech_qt_svg/com_trolltech_qt_svg0.h
    extensions/PythonQt_QtAll/PythonQt_QtAll.h
)

#-----------------------------------------------------------------------------
# Add extra sources
foreach(qtlib core gui svg uitools xml)
  
  if (${PythonQt_Wrap_Qt${qtlib}})
    
    ADD_DEFINITIONS(-DPYTHONQT_WRAP_Qt${qtlib})
    
    set(file_prefix generated_cpp/com_trolltech_qt_${qtlib}/com_trolltech_qt_${qtlib})
    
    foreach(index RANGE 0 10)
    
      # Source files
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file_prefix}${index}.cpp)
        list(APPEND sources ${file_prefix}${index}.cpp)
      endif()
      
      # Headers that should run through moc
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file_prefix}${index}.h)
        list(APPEND moc_sources ${file_prefix}${index}.h)
      endif()
      
    endforeach()
    
    list(APPEND sources ${file_prefix}_init.cpp)

  endif()
endforeach()

#-----------------------------------------------------------------------------
# UI files
set(ui_sources )

#-----------------------------------------------------------------------------
# Resources
set(qrc_sources )

#-----------------------------------------------------------------------------
# Do wrapping
qt4_wrap_cpp(gen_moc_sources ${moc_sources})
qt4_wrap_ui(gen_ui_sources ${ui_sources})
qt4_add_resources(gen_qrc_sources ${qrc_sources})

#-----------------------------------------------------------------------------
# Build the library

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  
add_library(PythonQt STATIC
            ${sources}
            ${gen_moc_sources}
            ${gen_ui_sources}
            ${gen_qrc_sources}
  )
set_target_properties(PythonQt PROPERTIES DEFINE_SYMBOL PYTHONQT_EXPORTS)

#
# That should solve linkage error on Mac when the project is used in a superbuild setup
# See http://blog.onesadcookie.com/2008/01/installname-magic.html
#
set_target_properties(PythonQt  PROPERTIES
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  )

target_link_libraries(PythonQt
              ${PYTHON_LIBRARY}
              ${QT_LIBRARIES}
  )

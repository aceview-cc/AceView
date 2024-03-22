# Install script for directory: D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/libheif")
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

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/Debug/heif.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/Release/heif.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/MinSizeRel/heif.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/RelWithDebInfo/heif.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/Debug/heif.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/Release/heif.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/MinSizeRel/heif.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/RelWithDebInfo/heif.dll")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libheif" TYPE FILE FILES
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/bitstream.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/box.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/error.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_api_structs.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_context.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_file.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_image.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_hevc.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_colorconversion.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_plugin_registry.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_limits.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/heif_plugin.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/libheif/logging.h"
    "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/heif_version.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif/libheif-config.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif/libheif-config.cmake"
         "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif/libheif-config-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif/libheif-config.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/CMakeFiles/Export/lib/cmake/libheif/libheif-config-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libheif" TYPE FILE FILES "D:/trunk/2345pic/vendor/depends/libheif/libheif/build/libheif/libheif-config-version.cmake")
endif()


# Install script for directory: /home/nins/Code/work/incubator-pegasus/src/rdsn/src/replica

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/nins/Code/work/incubator-pegasus/DSN_ROOT")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/libdsn_replica_server.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so"
         OLD_RPATH "/home/nins/Code/work/incubator-pegasus/thirdparty/output/lib:/home/nins/Code/work/incubator-pegasus/thirdparty/output/lib64:/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server:/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/utils:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libdsn_replica_server.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/duplication/test/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/backup/test/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/bulk_load/test/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/split/test/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/storage/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/test/cmake_install.cmake")

endif()


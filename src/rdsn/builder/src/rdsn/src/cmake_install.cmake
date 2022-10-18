# Install script for directory: /home/nins/Code/work/incubator-pegasus/src/rdsn/src

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/runtime/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/aio/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/zookeeper/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/perf_counter/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/failure_detector/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/remote_cmd/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/nfs/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/block_service/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/http/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/client/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/common/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/replica/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/meta/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/tools/cmake_install.cmake")
  include("/home/nins/Code/work/incubator-pegasus/src/rdsn/builder/src/rdsn/src/utils/cmake_install.cmake")

endif()


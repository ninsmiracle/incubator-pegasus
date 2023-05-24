# Install script for directory: /home/nins/code/incubator-pegasus/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/aio/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/base/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/base/test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/block_service/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/client/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/client_lib/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/common/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/failure_detector/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/geo/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/http/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/meta/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/nfs/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/perf_counter/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/redis_protocol/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/remote_cmd/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/replica/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/reporter/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/runtime/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/sample/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/server/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/server/test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/shell/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/test_util/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/test/bench_test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/test/function_test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/test/kill_test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/test/pressure_test/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/tools/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/utils/cmake_install.cmake")
  include("/home/nins/code/incubator-pegasus/cmake-build-debug-clang/src/zookeeper/cmake_install.cmake")

endif()


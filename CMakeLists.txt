project(UrbanCLI CXX)
cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

find_package(PkgConfig)
pkg_search_module(CURL REQUIRED libcurl)
pkg_search_module(Json REQUIRED jsoncpp)
include_directories(${CURL_INCLUDE_DIRS} ${Json_INCLUDE_DIRS})
link_libraries(${CURL_LIBRARIES} ${Json_LIBRARIES})

add_executable(urbancli urbancli.cpp)

install(
  TARGETS urbancli
  RUNTIME DESTINATION bin
)
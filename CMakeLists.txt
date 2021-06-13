cmake_minimum_required(VERSION 3.5)

project(bridge LANGUAGES CXX)

include(FetchContent)

message(STATUS "Standby, pulling yaml-cpp...")
FetchContent_Declare(yamlcpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_PROGRESS ON
	GIT_SHALLOW ON
	FETCHCONTENT_FULLY_DISCONNECTED ON
)
option(YAML_CPP_BUILD_TESTS OFF)
option(YAML_CPP_BUILD_TOOLS OFF)
option(YAML_CPP_BUILD_CONTRIB OFF)
option(YAML_CPP_INSTALL OFF)
FetchContent_MakeAvailable(yamlcpp)

message(STATUS "Standby, pulling LevelDB...")
FetchContent_Declare(leveldb_src
	GIT_REPOSITORY https://github.com/google/leveldb.git
	GIT_PROGRESS ON
	GIT_SHALLOW ON
	FETCHCONTENT_FULLY_DISCONNECTED ON
)
option(LEVELDB_BUILD_TESTS OFF)
option(LEVELDB_BUILD_BENCHMARKS OFF)
option(LEVELDB_INSTALL OFF)
FetchContent_MakeAvailable(leveldb_src)

add_subdirectory(table_1)
add_subdirectory(table_2)
add_subdirectory(table_3)

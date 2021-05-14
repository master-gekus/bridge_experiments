cmake_minimum_required(VERSION 3.5)

project(table_1 LANGUAGES CXX)

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

add_executable(${PROJECT_NAME}
	main.cpp
	table.h
	table.cpp
	)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
	)

target_link_libraries(${PROJECT_NAME}
	yaml-cpp
	)

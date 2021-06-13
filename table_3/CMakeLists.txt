cmake_minimum_required(VERSION 3.5)

project(table_3 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
	main.cpp
	enums.hpp
	enums.cpp
	table_first.h
	table_first.cpp
	table_processor.hpp
	table_processor.cpp
	)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE ON
	)

target_link_libraries(${PROJECT_NAME}
	yaml-cpp
	leveldb
	)

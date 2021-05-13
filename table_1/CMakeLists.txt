cmake_minimum_required(VERSION 3.5)

project(table_1 LANGUAGES CXX)

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

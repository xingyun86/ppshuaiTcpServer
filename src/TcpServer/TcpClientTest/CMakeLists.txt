# CMakeList.txt : CMake project for TcpServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set (project_name TcpClientTest)
project(${project_name})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/Vendor/include
)
link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

IF(MSVC)
set (MSVC_LIBS ws2_32)
ELSE()
set (MSVC_LIBS )
ENDIF(MSVC)

# Add source to this project's executable.
add_executable (${project_name} "TcpClientTest.cpp" "TcpClientTest.h")

target_link_libraries(${project_name} 
	 PRIVATE ${MSVC_LIBS}
)
# TODO: Add tests and install targets if needed.

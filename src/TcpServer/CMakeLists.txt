# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("TcpServer")

# Include sub-projects.
add_subdirectory ("TcpServer")
add_subdirectory ("TcpClient")
add_subdirectory ("UdpServer")
add_subdirectory ("UdpClient")

add_subdirectory ("TcpClientTest")
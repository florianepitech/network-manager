cmake_minimum_required(VERSION 3.21)
project(test)

set(CMAKE_CXX_STANDARD 20)

add_executable(test main_server.cpp NewNetworkManager.hpp TcpManager.hpp UdpManager.hpp uuid.hpp EventRegistry.hpp NetworkUtils.hpp)

# target_link_libraries(test PUBLIC pthread)

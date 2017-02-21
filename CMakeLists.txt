cmake_minimum_required(VERSION 3.6)
project(SimpleHttpServer)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp SimpleHttpServer.hpp HttpHandler.hpp SimpleHttpServer.cpp HttpHandler.cpp DatabaseHandler.cpp DatabaseHandler.hpp DynamicHtml.cpp DynamicHtml.hpp)
add_executable(SimpleHttpServer ${SOURCE_FILES})

include_directories(/usr/include/cppconn)
link_directories(/usr/lib)

find_package(Boost COMPONENTS REQUIRED system filesystem)
target_link_libraries (SimpleHttpServer ${Boost_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(SimpleHttpServer Threads::Threads mysqlcppconn)
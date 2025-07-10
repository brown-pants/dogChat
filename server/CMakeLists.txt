cmake_minimum_required(VERSION 3.10)

project(chat_server)

file(GLOB_RECURSE SRC "./src/*.cpp" "./src/*.h")

add_executable(chat_server ${SRC})

find_library(MYSQLCPPCONN_LIBRARY NAMES mysqlcppconn)

if (NOT MYSQLCPPCONN_LIBRARY)
    message(FATAL_ERROR "MySQL Connector/C++ not found!")
endif()

find_package(Boost REQUIRED COMPONENTS system)

target_link_libraries(chat_server PRIVATE ${MYSQLCPPCONN_LIBRARY} Boost::system)

target_include_directories(chat_server PRIVATE "3rdparty/json-develop/include")

file(COPY ${PROJECT_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_FLAGS "-std=c++11")

project(Rummy VERSION 1.0.0.1)

configure_file("${CMAKE_SOURCE_DIR}/src/RummyConfig.h.in" "${CMAKE_SOURCE_DIR}/src/RummyConfig.h")

include_directories(
    "${CMAKE_SOURCE_DIR}/inc"
)

link_directories(
    "${CMAKE_SOURCE_DIR}/lib"
)

aux_source_directory("${CMAKE_SOURCE_DIR}/src" DIR_SRCS) 

add_executable(${PROJECT_NAME} ${DIR_SRCS}) 

target_link_libraries(${PROJECT_NAME} jsoncpp_static)

add_custom_command( 
          TARGET ${PROJECT_NAME} 
          POST_BUILD 
          COMMAND echo "Attention ${CMAKE_SOURCE_DIR}/${PROJECT_NAME} is coming!"
)




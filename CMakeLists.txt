cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Release)

if (CMAKE_BUILD_TYPE STREQUAL Debug )
    add_definitions(
        -DDEBUG -DDEBUGMORE
    )
endif()

set(CXX_FLAGS
 -g
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 -std=c++11
 -rdynamic
 )


project(rummy)

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
          COMMAND echo "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}"
)




if(EMSCRIPTEN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
  set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
endif()

cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(MetroGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED

if(NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)

  if(NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

add_subdirectory(third_party/random)

include_directories(src third_party)

set(SOURCES
  "src/metro.hpp"
  "src/metro.cpp"
  "src/main.cpp"
  "src/raygui.h"
  "src/ui.hpp"
  "resources.o"
)

if(WIN32)
  add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
  )
else()
  add_executable(${PROJECT_NAME}
    ${SOURCES}
  )
endif()

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
  )
  set_source_files_properties(resources.rc PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} effolkronium_random)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

if(${PLATFORM} STREQUAL "Web")
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

if(APPLE)
  target_link_libraries(${PROJECT_NAME} "-framework IOKit")
  target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
  target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

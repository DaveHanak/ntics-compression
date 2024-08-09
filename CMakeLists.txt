cmake_minimum_required(VERSION 3.5.0)
project(NTComp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17) # std::filesystem

set(SOURCE_FILES main.cpp)
add_executable(NTComp ${SOURCE_FILES})

# You can alter these according to your needs, e.g if you don't need to display images - set(YOU_NEED_X11 0)
set(YOU_NEED_X11 0)
set(YOU_NEED_JPG 1)
set(YOU_NEED_PNG 0)

# Display images
if(${YOU_NEED_X11} EQUAL 1)
    message(STATUS "Looking for X11...")
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    target_link_libraries(NTComp ${X11_LIBRARIES})
else()
    target_compile_definitions(NTComp PRIVATE cimg_display=0)
endif()

# JPG support
if(${YOU_NEED_JPG} EQUAL 1)
    message(STATUS "Looking for libjpg...")
    find_package(JPEG REQUIRED)
    include_directories(${JPEG_INCLUDE_DIR})
    target_link_libraries (NTComp ${JPEG_LIBRARY})
    target_compile_definitions(NTComp PRIVATE cimg_use_jpeg=1)
endif()

# PNG support
if(${YOU_NEED_PNG} EQUAL 1)
    message(STATUS "Looking for libpng...")
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries (NTComp ${PNG_LIBRARY})
    target_compile_definitions(NTComp PRIVATE cimg_use_png=1)
endif()
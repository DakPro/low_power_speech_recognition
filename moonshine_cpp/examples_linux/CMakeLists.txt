# examples/CMakeLists.txt

include(../cmake/FetchSDL.cmake)
find_package(SDL2 REQUIRED)
find_library(SNDFILE_LIBRARY
        NAMES sndfile libsndfile-1
        REQUIRED
)
find_path(SNDFILE_INCLUDE_DIR
        NAMES sndfile.h
        REQUIRED
)
# Create example executables
add_executable(moonshine_file file.cpp)
add_executable(moonshine_live live.cpp)

target_link_libraries(moonshine_file PRIVATE moonshine ${SNDFILE_LIBRARY})
target_include_directories(moonshine_file PRIVATE ${SNDFILE_INCLUDE_DIR})

target_include_directories(moonshine_live PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(moonshine_live PRIVATE moonshine SDL2::SDL2)

target_include_directories(moonshine_file
        PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIRS}
)

target_include_directories(moonshine_live
        PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
)

# Install example executable to bin directory
install(TARGETS moonshine_live
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS moonshine_file
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
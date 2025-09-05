# examples/CMakeLists.txt

include(../cmake/FetchSDL.cmake)
find_package(SDL2 REQUIRED)

# Create example executables
add_executable(moonshine_live live.cpp alt_conio.cpp alt_conio.hpp)

#target_link_libraries(moonshine_example
#        PRIVATE
#        moonshine
#)
target_include_directories(moonshine_live PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(moonshine_live PRIVATE moonshine SDL2::SDL2)

#target_include_directories(moonshine_example
#        PRIVATE
#        ${ONNXRUNTIME_INCLUDE_DIRS}
#)

target_include_directories(moonshine_live
        PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
)

# Install example executable to bin directory
install(TARGETS moonshine_live
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

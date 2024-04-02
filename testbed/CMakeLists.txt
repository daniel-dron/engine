add_library(testbed SHARED
    src/main.cpp
)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_property(TARGET testbed PROPERTY CXX_STANDARD 20)

target_include_directories(testbed PUBLIC ${PROJECT_SOURCE_DIR}/engine/src)

target_link_libraries(testbed PRIVATE engine)
target_link_libraries(testbed PRIVATE imgui)
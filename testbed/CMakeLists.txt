add_library(testbed SHARED
    src/main.cpp
)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(testbed PUBLIC ${PROJECT_SOURCE_DIR}/engine/src)

target_link_libraries(testbed PRIVATE engine)
target_link_libraries(testbed PRIVATE imgui)
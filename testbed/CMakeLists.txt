add_executable(testbed
    src/main.cpp
)

target_include_directories(testbed PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/)

target_link_libraries(testbed PRIVATE engine)
target_link_libraries(testbed PRIVATE imgui)
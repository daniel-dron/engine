add_library(engine SHARED
    src/engine.cpp
    src/glad/glad.c
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set_target_properties(engine PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/)

add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/imgui)
target_link_libraries(engine PRIVATE glfw)
target_link_libraries(engine PRIVATE imgui)

# copy engine.dll to folder
add_custom_command(TARGET engine POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                            ${CMAKE_BINARY_DIR}/engine/engine.dll
                            ${CMAKE_BINARY_DIR}/testbed/engine.dll)

message(${CMAKE_BINARY_DIR})
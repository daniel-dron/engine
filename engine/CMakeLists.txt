add_executable(engine
    src/entry.cpp
    src/renderer/resources/buffer.cpp
    src/renderer/resources/vertex_layout.cpp
    src/renderer/resources/vertex_array.cpp
    src/renderer/resources/shader_program.cpp
    src/renderer/resources/gl_errors.cpp
    src/glad/glad.c
)
set_property(TARGET engine PROPERTY ENABLE_EXPORTS 1)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_property(TARGET engine PROPERTY CXX_STANDARD 20)

include_directories(C:/projects/engine/engine/src/)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/)

add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/glm)

target_link_libraries(engine PRIVATE glfw)
target_link_libraries(engine PRIVATE imgui)
target_link_libraries(engine PRIVATE assimp)
target_link_libraries(engine PRIVATE glm)
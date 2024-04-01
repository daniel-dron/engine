add_executable(engine
    src/engine.cpp
    src/glad/glad.c
)
set_property(TARGET engine PROPERTY ENABLE_EXPORTS 1)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/)

add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine/vendor/glm)

target_link_libraries(engine PRIVATE glfw)
target_link_libraries(engine PRIVATE imgui)
target_link_libraries(engine PRIVATE assimp)
target_link_libraries(engine PRIVATE glm)
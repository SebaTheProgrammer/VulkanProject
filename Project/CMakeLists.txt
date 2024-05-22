# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Models")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

file(GLOB_RECURSE MODEL_FILES
    "${MODELS_SOURCE_DIR}/*.obj"
)

# Define the sources for the executable
set(SOURCES
    "main.cpp"
    "AppBase.cpp"
    "Window.cpp"
    "Pipeline.cpp"
    "EngineDevice.cpp"
    "SwapChain.cpp"
    "Model.cpp"
    "Renderer.cpp"
    "SimpleRenderSystem.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES}        
    "Camera.h" "SDL2-2.28.3/SDL_keyboard.h" "Pipeline.h" 
    "Model.h" "GameObject.h" "Renderer.h" "Renderer.cpp" 
    "SimpleRenderSystem.cpp" "Input.h"
    "tiny_obj_loader.h" "Utils.h" "stb_image.h" )

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_dependencies(${PROJECT_NAME} Shaders)

# Add a custom target to copy .obj files
add_custom_target(CopyOBJFiles ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/Models"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MODEL_FILES} "${CMAKE_BINARY_DIR}/Models"
)

# Ensure the OBJ files are copied before building the project
add_dependencies(${PROJECT_NAME} CopyOBJFiles)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)

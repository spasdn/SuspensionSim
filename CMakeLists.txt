cmake_minimum_required(VERSION 3.20)
project(SuspensionSim VERSION 0.1.0 LANGUAGES C CXX)

# -----------------------------------------------------------------------------
# Opciones
# -----------------------------------------------------------------------------
option(SUSPSIM_BUILD_APP   "Compilar la aplicación gráfica (OpenGL + ImGui)" ON)
option(SUSPSIM_BUILD_TESTS "Compilar los tests del núcleo de cálculo"       ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # útil para VS Code / clangd

if(MSVC)
    add_compile_options(/W3 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES)
else()
    add_compile_options(-Wall -Wextra)
endif()

# -----------------------------------------------------------------------------
# Dependencias: se descargan solas la primera vez que configuras (necesita git)
# -----------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# GLM: matemáticas de vectores y matrices (como las de GLSL)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE)

# nlohmann/json: leer y escribir JSON
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE)
set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(glm json)

# -----------------------------------------------------------------------------
# Núcleo de cálculo (NO depende de OpenGL: se puede usar desde tests o CLI)
# -----------------------------------------------------------------------------
add_library(suspcore STATIC
    src/core/Suspension.cpp
    src/core/KinematicSolver.cpp
    src/analysis/Analysis.cpp
    src/analysis/Report.cpp
    src/analysis/Optimizer.cpp
    src/io/SuspensionLoader.cpp
    src/io/ObjLoader.cpp
)
target_include_directories(suspcore PUBLIC src)
target_link_libraries(suspcore PUBLIC glm::glm nlohmann_json::nlohmann_json)
# Ruta raíz del proyecto, para encontrar data/ y docs/ sin preocuparse del directorio de ejecución
target_compile_definitions(suspcore PUBLIC SUSPSIM_ROOT="${CMAKE_SOURCE_DIR}")

# -----------------------------------------------------------------------------
# Aplicación gráfica
# -----------------------------------------------------------------------------
if(SUSPSIM_BUILD_APP)
    # GLFW: crea la ventana, el contexto OpenGL y gestiona ratón/teclado
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
        GIT_SHALLOW    TRUE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

    # Dear ImGui: interfaz (botones, sliders, tablas, gráficas simples)
    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.8
        GIT_SHALLOW    TRUE)

    FetchContent_MakeAvailable(glfw imgui)

    # glad: cargador de funciones OpenGL 3.3 core (ya generado en third_party/)
    add_library(glad STATIC third_party/glad/src/glad.c)
    target_include_directories(glad PUBLIC third_party/glad/include)

    # ImGui no trae CMake propio: lo compilamos nosotros con sus backends GLFW + OpenGL3
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui PUBLIC glfw)
    # Que GLFW no incluya <GL/gl.h>: las funciones de OpenGL las pone glad
    target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_NONE)

    add_executable(suspsim
        src/main.cpp
        src/app/App.cpp
        src/app/Panels.cpp
        src/render/Shader.cpp
        src/render/Camera.cpp
        src/render/Renderer.cpp
        src/render/SceneBuilder.cpp
    )
    target_link_libraries(suspsim PRIVATE suspcore glad glfw imgui)
    if(NOT WIN32)
        target_link_libraries(suspsim PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()

# -----------------------------------------------------------------------------
# Tests (ejecutable de consola, sin ventana)
# -----------------------------------------------------------------------------
if(SUSPSIM_BUILD_TESTS)
    enable_testing()
    add_executable(test_kinematics tests/test_kinematics.cpp)
    target_link_libraries(test_kinematics PRIVATE suspcore)
    add_test(NAME kinematics COMMAND test_kinematics)
endif()

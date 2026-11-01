---
tipo: decision
fecha: 2026-10-31
estado: aceptada
tags: [decision]
---
# ADR-001 Stack técnico

## Contexto
Simulador de suspensiones en C++ con visualización sencilla (líneas y triángulos), interfaz para editar datos y ver resultados, multiplataforma, y un autor que está aprendiendo OpenGL.

## Decisión
| Necesidad | Elegido | Alternativas descartadas |
|---|---|---|
| Compilación | **CMake + FetchContent** | vcpkg/conan (más pasos de instalación) |
| Ventana y entrada | **GLFW 3.4** | SDL2 (más grande), Qt (pesado, licencia) |
| API gráfica | **OpenGL 3.3 core** | Vulkan/DX12 (excesivo), OpenGL 1.x inmediato (obsoleto, pero más fácil) |
| Cargador GL | **glad** (pregenerado en `third_party/`) | GLEW |
| Matemáticas | **GLM** (double en el cálculo, float en el dibujo) | Eigen (mejor para álgebra lineal grande: útil si llega el solver genérico) |
| Interfaz | **Dear ImGui** | Qt, wxWidgets |
| Datos | **JSON (nlohmann)** | XML, YAML |
| Mallas | **OBJ** propio | Assimp (muy completo pero pesado) |

## Consecuencias
- Todo se descarga solo: clonar + abrir en Visual Studio + ▶.
- El núcleo de cálculo no depende de OpenGL → se puede probar sin ventana y reutilizar (CLI, Python bindings…).
- Si se pasa a un solver multicuerpo genérico, valorar añadir **Eigen**.

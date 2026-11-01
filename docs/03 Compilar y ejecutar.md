---
tags: [proyecto, setup]
---
# Compilar y ejecutar

El proyecto usa **CMake**, que descarga solo las librerías (GLFW, GLM, ImGui, JSON) la primera vez. Solo necesitas un compilador de C++17, CMake ≥ 3.20 y git.

## Windows (recomendado: Visual Studio 2022)
1. Instala **Visual Studio 2022 Community** con la carga de trabajo *Desarrollo para el escritorio con C++* (incluye CMake).
2. Instala **git** (git-scm.com) si no lo tienes.
3. En Visual Studio: *Archivo → Abrir → Carpeta…* y elige la carpeta `SuspensionSim` (la que tiene `CMakeLists.txt`).
4. Espera a que termine "CMake generation" (la primera vez tarda 1-2 min: está descargando librerías).
5. Arriba, en el desplegable de destino de inicio, elige **suspsim.exe** y pulsa ▶.

Para ejecutar el test: elige `test_kinematics.exe` como destino y ▶. Debe terminar con `TODO OK`.

### Alternativa: VS Code
Extensiones *C/C++* y *CMake Tools* (de Microsoft). Abre la carpeta, elige el kit (compilador) que te proponga y usa los botones *Build* / *Run* de la barra inferior.

## Linux
```bash
sudo apt install build-essential cmake git libx11-dev libxrandr-dev libxinerama-dev \
                 libxcursor-dev libxi-dev libgl1-mesa-dev libwayland-dev libxkbcommon-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/test_kinematics
./build/suspsim                 # o: ./build/suspsim data/otro_modelo.json
```

## Desde la terminal (cualquier sistema)
```bash
cmake -S . -B build
cmake --build build --config Release
```

## Uso de la ventana
| Acción | Ratón / panel |
|---|---|
| Girar la vista | Botón izquierdo + arrastrar |
| Desplazar | Botón derecho, central o Mayús + izquierdo |
| Zoom | Rueda |
| Vistas tipo plano | Panel *Vista* → Frontal / Lateral / Planta + "Proyección ortogonal" |
| Mover la rueda | Panel *Condiciones de contorno* → slider *Recorrido* |
| Girar el volante | Slider *Cremallera* |
| Editar un punto | *Modelo → Puntos duros*: arrastra el valor o doble clic para escribir |
| Informe | *Informe → Generar informe* → aparece en `docs/Informes/` (lo verás en Obsidian) |

## Problemas típicos
- **"git no encontrado" al configurar** → instala git y reinicia Visual Studio.
- **La ventana no abre / "OpenGL 3.3 no disponible"** → actualiza el driver de la gráfica. En máquinas virtuales suele faltar OpenGL 3.3.
- **Cambio un archivo `.h` y no se recompila nada** → *Proyecto → Eliminar caché y reconfigurar*.
- **Opciones de compilación**: `-DSUSPSIM_BUILD_APP=OFF` compila solo el núcleo y el test (útil en un PC sin gráfica).

## Comprobado
El 31-10-2026 se compiló todo (núcleo, test y aplicación) con GCC 13 y CMake 3.28, y el test pasó. La ventana aún no se ha probado en un PC con pantalla → primera tarea: [[2026-10-31]].

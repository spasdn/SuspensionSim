# SuspensionSim

Simulador de cinemática de suspensiones en C++17 con visualización OpenGL 3.3 e interfaz Dear ImGui.

- Geometría por coordenadas (JSON) y chasis opcional en `.obj`
- Solver cinemático de doble trapecio (recorrido + dirección)
- Parámetros: caída, convergencia, avance, KPI, scrub, trail, centro de balanceo, centros instantáneos, anti-dive/squat, ratio de instalación…
- Informe Markdown (para Obsidian) + CSV
- Optimización de puntos duros (Nelder-Mead)

## Arranque rápido
```bash
cmake -S . -B build
cmake --build build --config Release
./build/test_kinematics     # debe acabar en "TODO OK"
./build/suspsim             # abre la ventana con el modelo de ejemplo
```
En Windows: Visual Studio 2022 → *Abrir carpeta* → elegir `suspsim.exe` → ▶.

## Documentación
En la bóveda de Obsidian, proyecto **30260009 SuspensionSim** (ver `docs/README.md`).

## Informes
El botón *Generar informe* escribe un `.md` y un `.csv` en la carpeta indicada en el panel *Informe*.
Por defecto es `output/informes`; para que caigan en la bóveda, fija una vez la variable de entorno:
```
setx SUSPSIM_INFORMES "C:\Users\spas2\Documents\Personal\FilosofiaDeVida\30-Proyectos\30260009 SuspensionSim\Informes"
```

## Estructura
```
src/core       datos y solver cinemático      (sin OpenGL)
src/analysis   parámetros, informe, optimizador (sin OpenGL)
src/io         JSON y OBJ                     (sin OpenGL)
src/render     shader, cámara, renderer, escena
src/app        ventana, bucle, paneles ImGui
tests/         test del núcleo
data/          modelos de ejemplo
third_party/   glad (cargador OpenGL generado)
docs/          puntero a la documentación (bóveda de Obsidian)
```

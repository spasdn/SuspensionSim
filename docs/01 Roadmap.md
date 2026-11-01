---
tags: [proyecto, roadmap]
---
# Roadmap

Marca las casillas según avances. Cada fase debería acabar con algo que se pueda **ver** en la ventana o en el informe.

## Fase 0 — Base (hecho)
- [x] CMake + dependencias automáticas (GLFW, glad, GLM, ImGui, nlohmann/json)
- [x] Renderer de líneas, triángulos y puntos → [[Cómo dibujar algo nuevo]]
- [x] Cámara orbital con vistas frontal / lateral / planta
- [x] Núcleo de cálculo separado de OpenGL (librería `suspcore`) + test de consola

## Fase 1 — Cinemática de una esquina (hecho, revisar)
- [x] Solver de doble trapecio → [[Solver cinemático]]
- [x] Parámetros estáticos y en barrido → [[Parámetros de suspensión]]
- [x] Informe Markdown → [[Informe]]
- [ ] **Validar** con una geometría conocida (comparar con un software comercial, un libro o una hoja de cálculo propia)
- [ ] Revisar criterios de signo con un caso real (anotarlo en el diario)

## Fase 2 — Coche completo
- [ ] Modelar las 4 esquinas (delantera + trasera, con simetría o sin ella)
- [ ] Movimiento de **balanceo** (roll) y **cabeceo** (pitch) del chasis: imponer el ángulo y resolver las cuatro ruedas
- [ ] Ackermann (dos ruedas delanteras con la misma cremallera)
- [ ] Centro de balanceo con las dos ruedas (sin suponer simetría)
- [ ] Eje de balanceo (une CB delantero y trasero)

## Fase 3 — Más tipos de suspensión
- [ ] McPherson (cambia el brazo superior por la unión deslizante del amortiguador)
- [ ] Push-rod / pull-rod con balancín (rocker): ratio de instalación real
- [ ] Multibrazo de 5 barras (solver genérico por restricciones de distancia)
- [ ] Idea: pasar a un **solver genérico** (todas las barras como restricciones |pi − pj| = L, resolver con Newton-Raphson multidimensional). Es más trabajo pero sirve para cualquier suspensión.

## Fase 4 — Cargas (cuasi-estático)
- [ ] Fuerzas en el contacto (vertical, lateral, longitudinal) como condición de contorno
- [ ] Fuerzas en cada barra (equilibrio de la mangueta: 6 ecuaciones, 6 incógnitas en doble trapecio)
- [ ] Rigidez del muelle → rigidez en rueda (wheel rate = k · MR²)
- [ ] Transferencia de carga en curva / frenada

## Fase 5 — Optimización seria
- [x] Nelder-Mead monoobjetivo
- [ ] Objetivo ponderado (varios criterios a la vez)
- [ ] Restricciones (p. ej. no chocar con la llanta, longitudes mínimas)
- [ ] Ejecutar en un hilo aparte para no congelar la ventana
- [ ] Probar otros métodos (enjambre de partículas, genético, NSGA-II para frente de Pareto)

## Fase 6 — Calidad de vida
- [ ] Diálogo de "Abrir archivo" (librería *nativefiledialog-extended* o *tinyfiledialogs*)
- [ ] Gráficas buenas con **ImPlot** (ejes, leyendas, cursor)
- [ ] Seleccionar un punto con el ratón y moverlo en 3D (*ImGuizmo*)
- [ ] Importar STL directamente (además de OBJ) → [[Importar geometría 3D]]
- [ ] Exportar captura de pantalla para el informe
- [ ] Texto en 3D (nombres de puntos) con `ImGui::GetForegroundDrawList()`

## Ideas sueltas
- Guardar "casos" (modelo + condiciones) para comparar dos diseños en la misma gráfica.

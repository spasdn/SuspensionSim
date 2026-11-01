---
tags: [moc]
---
# SuspensionSim — Inicio

Simulador de cinemática de suspensiones en C++ con visualización OpenGL (líneas y triángulos), condiciones de contorno, informe automático y optimización de puntos duros.

> [!info] Cómo está organizado
> Esta carpeta `docs/` es la **bóveda de Obsidian**. El código está al lado, en `../src`.
> Abre en Obsidian *solo* `docs/` (no la carpeta raíz: la carpeta `build/` tiene miles de archivos).

## Empezar aquí
1. [[03 Compilar y ejecutar]] — instalar herramientas y ver la primera ventana
2. [[OpenGL desde cero]] — lo justo de OpenGL para entender el código
3. [[02 Arquitectura]] — qué hace cada archivo
4. [[01 Roadmap]] — qué está hecho y qué falta

## Proyecto
- [[01 Roadmap]]
- [[02 Arquitectura]]
- [[03 Compilar y ejecutar]]
- Decisiones: [[ADR-001 Stack técnico]]
- Diario: carpeta `Diario/` (plantilla en `_plantillas/Diario`)

## OpenGL e interfaz
- [[OpenGL desde cero]]
- [[Cómo dibujar algo nuevo]]
- [[Dear ImGui en 5 minutos]]

## Teoría
- [[Sistema de coordenadas]]
- [[Parámetros de suspensión]]
- [[Solver cinemático]]

## Datos de entrada
- [[Formato JSON de hardpoints]]
- [[Importar geometría 3D]]

## Simulación y resultados
- [[Condiciones de contorno]]
- [[Informe]]
- [[Optimización]]
- Informes generados: carpeta `Informes/`

## Estado actual (31-10-2026)
- [x] Proyecto CMake que descarga sus dependencias solo
- [x] Ventana OpenGL 3.3 + interfaz ImGui + cámara orbital
- [x] Lectura de puntos duros (JSON) y chasis (.obj)
- [x] Solver cinemático de doble trapecio (recorrido + dirección)
- [x] Parámetros: caída, convergencia, avance, KPI, scrub, trail, CB, CI, anti-dive, ratio de instalación
- [x] Informe Markdown + CSV
- [x] Optimizador Nelder-Mead con 4 objetivos
- [ ] Balanceo y cabeceo del chasis → ver [[01 Roadmap]]

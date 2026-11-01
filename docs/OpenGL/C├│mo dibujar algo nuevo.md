---
tags: [opengl, receta]
---
# Cómo dibujar algo nuevo

Todo el dibujo pasa por `src/render/SceneBuilder.cpp`. No hace falta tocar OpenGL directamente.

## Las herramientas (`Renderer`)
```cpp
r.addLine(a, b, color);                    // segmento
r.addDashedLine(a, b, color, 20.0f);       // discontinua (trazo de 20 mm)
r.addTriangle(a, b, c, color);             // cara (alfa < 1 = transparente)
r.addPoint(p, color);                      // punto gordo
r.addCross(p, 25.0f, color);               // cruz 3D
r.addCircle(centro, normal, radio, color); // circunferencia
r.addGrid(z, mitad, paso, color);          // rejilla en el suelo
r.addAxes(origen, longitud);               // ejes X rojo, Y verde, Z azul
```
`a`, `b`, `c` son `glm::vec3` en **mm**. Los puntos del cálculo son `susp::Vec3` (double): convierte con `glm::vec3(punto)`.

Colores ya definidos: `colors::Red`, `Green`, `Blue`, `Orange`, `Yellow`, `Magenta`, `Cyan`, `Gray`, `Dark`, `White`. Transparencia: `colors::Blue.withAlpha(0.3f)`. O uno propio: `Color{0.2f, 0.7f, 0.4f, 1.0f}`.

## Receta: añadir un elemento con su casilla en el panel Vista
1. **Opción** en `SceneOptions` (`render/SceneBuilder.h`):
   ```cpp
   bool showTirePatch = true;
   ```
2. **Dibujo** en `buildScene()` (`render/SceneBuilder.cpp`):
   ```cpp
   if (opt.showTirePatch) {
       glm::vec3 cp(st.contactPatch);
       float L = 80, W = 100;                       // mm
       glm::vec3 a = cp + glm::vec3(-L, -W, 0), b = cp + glm::vec3(L, -W, 0);
       glm::vec3 c = cp + glm::vec3(L,  W, 0),  d = cp + glm::vec3(-L, W, 0);
       r.addTriangle(a, b, c, colors::Red.withAlpha(0.4f));
       r.addTriangle(a, c, d, colors::Red.withAlpha(0.4f));
   }
   ```
3. **Casilla** en `App::panelView()` (`app/Panels.cpp`):
   ```cpp
   ImGui::Checkbox("Huella", &sceneOpt_.showTirePatch);
   ```
Compila y ya está.

## Dibujar el lado opuesto
`drawCorner(..., mirror = true, ...)` refleja Y → −Y. Si añades algo dentro de `drawCorner`, usa la lambda `P(punto)` en lugar de `glm::vec3(punto)` y saldrá automáticamente en los dos lados.

## Rendimiento
Cada fotograma se vuelven a copiar todos los vértices a la GPU. Para una suspensión (unos miles de vértices) no importa. Si importas un chasis de cientos de miles de triángulos y va lento, la solución es subir esa malla **una sola vez** a un VBO propio (`GL_STATIC_DRAW`) y dibujarla en cada fotograma sin volver a copiarla. Está anotado en [[01 Roadmap]].

Relacionado: [[OpenGL desde cero]] · [[02 Arquitectura]]

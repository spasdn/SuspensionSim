---
tags: [datos, formato]
---
# Formato JSON de hardpoints

Ejemplo completo: `data/ejemplo_doble_trapecio.json`. Se admiten comentarios `//`. Coordenadas `[x, y, z]` en mm, ISO 8855 ([[Sistema de coordenadas]]).

```jsonc
{
  "name": "Mi coche - delantera",
  "axle": "front",          // "front" | "rear"
  "side": "left",           // "left" | "right"  (lado modelado)

  "hardpoints": {
    // Chasis
    "lca_front":        [x, y, z],  // brazo inferior, anclaje delantero
    "lca_rear":         [x, y, z],  // brazo inferior, anclaje trasero
    "uca_front":        [x, y, z],  // brazo superior, anclaje delantero
    "uca_rear":         [x, y, z],  // brazo superior, anclaje trasero
    "tie_rod_inner":    [x, y, z],  // bieleta, lado cremallera (o lado chasis en un eje trasero con bieleta de convergencia)
    // Mangueta
    "lower_ball_joint": [x, y, z],
    "upper_ball_joint": [x, y, z],
    "tie_rod_outer":    [x, y, z],
    "wheel_center":     [x, y, z],
    "spindle_point":    [x, y, z]   // un punto del eje de rueda HACIA FUERA (define caída y convergencia estáticas)
  },

  "damper": {                        // opcional
    "chassis": [x, y, z],
    "arm":     [x, y, z],
    "attached_to": "lower_arm"       // "lower_arm" | "upper_arm" | "upright"
  },

  "tire":    { "radius": 300, "width": 205 },
  "vehicle": { "wheelbase": 2600, "cg_height": 500, "mass": 1200,
               "front_brake_bias": 0.65, "front_drive_fraction": 1.0 },
  "boundary": { "travel_min": -60, "travel_max": 60, "steps": 25,
                "rack": 0, "rack_min": -30, "rack_max": 30 },

  "chassis_mesh": "chasis_simple.obj"   // opcional, ruta relativa a este JSON
}
```

## Cómo definir `spindle_point`
Es el truco para dar caída y convergencia estáticas sin ángulos: un punto a, por ejemplo, 100 mm del centro de rueda sobre el eje de giro de la rueda, hacia fuera.
- Caída 0, convergencia 0, lado izquierdo: `wheel_center + [0, 100, 0]`.
- Caída −1°: subir la z en 100·tan(1°) ≈ 1,75 mm (el eje apunta un poco hacia arriba hacia fuera → la parte superior de la rueda se va hacia dentro).
- Convergencia +0,1° (toe-in): sumar a x 100·tan(0,1°) ≈ 0,17 mm.

## Dónde se lee / escribe
- `io/SuspensionLoader.cpp` → `loadSuspensionJson()` / `saveSuspensionJson()`.
- El botón **Guardar** de la interfaz sobrescribe el archivo (sin los comentarios).
- Para añadir un punto nuevo: campo en `Hardpoints` (`core/Suspension.h`), nombre en `hardpointNames()` y `hardpointByName()` (`core/Suspension.cpp`), y lectura/escritura aquí.

## Plantilla para empezar un modelo propio
Copia el ejemplo, cambia `name` y los puntos, y cárgalo desde *Modelo → Archivo → Cargar* (o pásalo como argumento: `suspsim ruta.json`).

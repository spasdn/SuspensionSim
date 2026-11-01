---
tags: [simulacion]
---
# Condiciones de contorno

## Implementadas
| Condición | Dónde | Notas |
|---|---|---|
| Recorrido vertical del centro de rueda | Slider *Recorrido*, `KinematicInputs::travel` | + compresión |
| Desplazamiento de cremallera | Slider *Cremallera*, `KinematicInputs::rack` | Mueve `tie_rod_inner` en Y |
| Rango y resolución del barrido | *Recorrido min/max*, *Puntos* | Se guardan en `boundary` del JSON |
| Datos de vehículo | *Modelo → Rueda y vehículo* | Batalla, altura CdG, reparto de frenada/tracción (para anti-dive/squat) |

## Por implementar (ver [[01 Roadmap]])
- **Balanceo** del chasis (°): con 2 esquinas, una sube y otra baja; mejor imponer el ángulo y resolver.
- **Cabeceo** (°): igual entre eje delantero y trasero.
- **Altura de marcha** (ride height): desplazar todo el chasis.
- **Cargas en el contacto** (Fz, Fy, Fx) para la fase cuasi-estática.
- **Casos de carga** con nombre: "frenada 1 g", "curva 1,2 g", "bache 3 g"…

## Idea de diseño para casos
Guardar en el JSON una lista de casos:
```jsonc
"cases": [
  { "name": "curva derecha", "roll_deg": 2.0, "travel": 0, "rack": -15 },
  { "name": "frenada",       "pitch_deg": -1.0 }
]
```
y que el informe saque una tabla por caso.

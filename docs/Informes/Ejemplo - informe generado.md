---
tipo: informe
modelo: "Ejemplo doble trapecio"
fecha: 2026-10-31
eje: delantero
lado: izquierdo
tags: [informe, suspension]
---

# Informe de cinemática — Ejemplo doble trapecio

Generado por SuspensionSim el 2026-10-31 10:50. Criterios de signo en [[Parámetros de suspensión]].

## Condiciones de contorno

| Parámetro | Valor |
|---|---|
| Recorrido analizado | -60.0 a 60.0 mm |
| Puntos del barrido | 25 |
| Cremallera | 0.0 mm |
| Radio de rueda | 300.0 mm |
| Batalla / altura CdG | 2600 / 500 mm |
| Reparto de frenada delante | 65 % |
| Tracción delantera | 100 % |

## Parámetros en posición de diseño

| Parámetro | Valor | Unidad |
|---|---:|---|
| Caída | -1.003 | ° |
| Convergencia (+ toe-in) | 0.000 | ° |
| Avance (caster) | 3.69 | ° |
| Inclinación del pivote (KPI) | 7.35 | ° |
| Radio de pivotamiento (scrub) | 57.2 | mm |
| Avance mecánico (trail) | 14.0 | mm |
| Brazo del pivote en centro de rueda | 90.6 | mm |
| Altura del centro de balanceo | 30.6 | mm |
| Brazo oscilante frontal (FVSA) | 1352 | mm |
| Brazo oscilante lateral (SVSA) | 58004 | mm |
| Anti en frenada | 5.1 | % |
| Anti en aceleración | 5.2 | % |
| Semivía | 635.2 | mm |
| Longitud del amortiguador | 503.7 | mm |
| Ratio de instalación | 0.705 | - |

Centro instantáneo frontal: (-0.0, -717.0, 65.2)  
Centro instantáneo lateral: (-58003.7, 630.0, 875.6)

## Ganancias en torno a la posición de diseño

| Magnitud | Valor | Unidad |
|---|---:|---|
| Ganancia de caída | -1.028 | °/pulgada |
| Ganancia de caída | -0.405 | °/10 mm |
| Bump steer | -33.799 | °/m |
| Migración vertical del CB | -1.356 | mm/mm |
| Variación de semivía | 0.402 | mm/10 mm |
| Variación de caída en el barrido | 5.000 | ° |
| Variación de convergencia en el barrido | 4.150 | ° |

## Barrido de recorrido

| Recorrido [mm] | Caída [°] | Conv. [°] | Avance [°] | KPI [°] | Scrub [mm] | h CB [mm] | Semivía [mm] | Anti fren. [%] | MR [-] |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| -60.0 | 0.868 | 1.972 | 3.64 | 5.36 | 57.4 | 117.9 | 628.7 | 6.1 | 0.713 |
| -50.0 | 0.629 | 1.646 | 3.65 | 5.62 | 57.4 | 102.6 | 630.4 | 6.0 | 0.711 |
| -40.0 | 0.362 | 1.321 | 3.65 | 5.90 | 57.3 | 87.6 | 631.8 | 5.8 | 0.710 |
| -30.0 | 0.067 | 0.994 | 3.66 | 6.22 | 57.3 | 72.9 | 633.0 | 5.6 | 0.709 |
| -20.0 | -0.259 | 0.666 | 3.67 | 6.57 | 57.3 | 58.4 | 634.0 | 5.4 | 0.707 |
| -10.0 | -0.615 | 0.335 | 3.68 | 6.94 | 57.2 | 44.4 | 634.7 | 5.3 | 0.706 |
| 0.0 | -1.003 | 0.000 | 3.69 | 7.35 | 57.2 | 30.6 | 635.2 | 5.1 | 0.705 |
| 10.0 | -1.424 | -0.341 | 3.71 | 7.80 | 57.2 | 17.2 | 635.5 | 5.0 | 0.704 |
| 20.0 | -1.882 | -0.688 | 3.72 | 8.28 | 57.1 | 4.3 | 635.6 | 4.8 | 0.703 |
| 30.0 | -2.378 | -1.044 | 3.75 | 8.80 | 57.1 | -8.3 | 635.5 | 4.7 | 0.701 |
| 40.0 | -2.916 | -1.409 | 3.77 | 9.36 | 57.1 | -20.3 | 635.1 | 4.6 | 0.700 |
| 50.0 | -3.499 | -1.786 | 3.80 | 9.97 | 57.0 | -31.9 | 634.6 | 4.5 | 0.698 |
| 60.0 | -4.132 | -2.177 | 3.83 | 10.63 | 57.0 | -42.7 | 633.8 | 4.4 | 0.696 |

## Puntos duros (mm, ISO 8855)

| Punto | X | Y | Z |
|---|---:|---:|---:|
| lca_front | 150.0 | 250.0 | 120.0 |
| lca_rear | -200.0 | 260.0 | 125.0 |
| uca_front | 120.0 | 330.0 | 390.0 |
| uca_rear | -160.0 | 340.0 | 395.0 |
| tie_rod_inner | 120.0 | 270.0 | 190.0 |
| lower_ball_joint | 5.0 | 560.0 | 140.0 |
| upper_ball_joint | -15.0 | 520.0 | 450.0 |
| tie_rod_outer | 115.0 | 555.0 | 200.0 |
| wheel_center | 0.0 | 630.0 | 300.0 |
| spindle_point | 0.0 | 730.0 | 301.8 |
| damper_chassis | -30.0 | 420.0 | 650.0 |
| damper_arm | -20.0 | 480.0 | 150.0 |

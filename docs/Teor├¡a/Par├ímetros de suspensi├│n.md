---
tags: [teoria, parametros]
---
# Parámetros de suspensión

Definiciones, criterio de signo y cómo se calcula cada uno en `analysis/Analysis.cpp` (`computeGeometry()`). Ejes en [[Sistema de coordenadas]].

Notación: **WC** centro de rueda, **CP** contacto con el suelo, **LBJ/UBJ** rótulas inferior/superior, **a** = vector unitario del eje de mangueta hacia fuera, **k** = vector unitario del eje de pivote (LBJ → UBJ), *s* = +1 lado izquierdo / −1 derecho.

## Alineación de rueda
| Parámetro | Definición | Signo | Cálculo | Estado |
|---|---|---|---|---|
| Caída (*camber*) | Inclinación del plano de rueda respecto a la vertical, vista frontal | − = parte superior hacia dentro | `-atan2(a.z, √(a.x²+a.y²))` | ✅ |
| Convergencia (*toe*) | Ángulo del plano de rueda respecto a X, vista en planta | + = convergencia (*toe-in*) | `atan2(a.x, s·a.y)` | ✅ |

## Eje de pivote (*kingpin*)
| Parámetro | Definición | Signo | Estado |
|---|---|---|---|
| Avance (*caster*) | Inclinación de k en vista lateral | + = parte superior hacia atrás | ✅ |
| KPI / SAI | Inclinación de k en vista frontal | + = parte superior hacia dentro | ✅ |
| Radio de pivotamiento (*scrub radius*) | Distancia lateral en el suelo entre CP y el corte de k con el suelo | + = el eje corta el suelo por dentro de CP | ✅ |
| Avance mecánico (*trail*) | Distancia longitudinal en el suelo entre el corte de k y CP | + = el eje corta por delante de CP | ✅ |
| Brazo del pivote en el centro de rueda | Distancia lateral de WC al eje k a la altura de WC (palanca de las fuerzas longitudinales con transmisión/frenos interiores) | + = WC por fuera del eje | ✅ |

## Centros instantáneos y centro de balanceo
**Vista frontal (FVIC):** cada trapecio es un plano (sus dos anclajes + su rótula). Se corta cada plano con el plano vertical X = X_WC → dos rectas. Su intersección es el centro instantáneo frontal.
- **FVSA** (*front view swing arm*): distancia lateral CP → FVIC. Controla la ganancia de caída: aproximadamente ganancia ≈ 1/FVSA [rad/mm].
- **Centro de balanceo (CB)**: recta CP → FVIC cortada con el plano de simetría Y = 0. **Supone simetría izquierda/derecha.** Altura medida desde el suelo bajo la rueda.
- Si los brazos son paralelos el FVIC está en el infinito y se usa una recta paralela a los brazos por CP.

**Vista lateral (SVIC):** igual, cortando con el plano Y = Y_WC.
- **SVSA**: distancia longitudinal CP → SVIC.
- **Anti en frenada** (anti-dive delante / anti-lift detrás), frenos exteriores (fuerza en CP):
  `anti% = tan θ · (L / h) · reparto_frenada · 100`, con θ = ángulo de la recta CP → SVIC (positivo si sube hacia el centro del coche).
- **Anti en aceleración** (anti-lift delante con tracción delantera / anti-squat detrás), transmisión interior (fuerza en WC): igual pero con la recta WC → SVIC y el reparto de tracción.
- L = batalla, h = altura del CdG (en `vehicle` del JSON).

> [!note] Suposiciones
> Neumático rígido (sin deformación), contacto en el punto más bajo de la circunferencia de rueda, chasis fijo. Ver [[Solver cinemático]].

## Posición de rueda
| Parámetro | Definición | Estado |
|---|---|---|
| Semivía | Coordenada lateral de CP (la vía es el doble si hay simetría) | ✅ |
| Posición longitudinal de WC | Para ver el retroceso de la rueda (*recession*) en el barrido | ✅ (en CSV) |

## Muelle/amortiguador
| Parámetro | Definición | Estado |
|---|---|---|
| Longitud | Distancia entre anclaje en chasis y en brazo | ✅ |
| Ratio de instalación (MR) | −d(longitud)/d(recorrido de rueda), por diferencias finitas (±0,5 mm) | ✅ |
| Rigidez en rueda | k_rueda = k_muelle · MR² | ⬜ pendiente |

## Ganancias (derivadas en el diseño)
Calculadas del barrido con `gradientAtZero()`:
- **Ganancia de caída** [°/10 mm] y [°/pulgada]
- **Bump steer** [°/m] — variación de convergencia con el recorrido
- **Migración vertical del CB** [mm/mm]

## Pendientes
- [ ] Ackermann (necesita las dos ruedas)
- [ ] Caída y CB con balanceo del chasis
- [ ] Altura del eje de balanceo
- [ ] Rigidez en rueda y frecuencia natural
- [ ] *Scrub* y caída con el volante girado (ya se puede: mover la cremallera y mirar la tabla)

Bibliografía recomendada: *Race Car Vehicle Dynamics* (Milliken & Milliken), cap. 17; *Tune to Win* (Carroll Smith); *The Automotive Chassis* (Reimpell).

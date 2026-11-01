---
tags: [teoria, solver]
---
# Solver cinemático (doble trapecio)

Código: `src/core/KinematicSolver.cpp`.

## Modelo
- Chasis **fijo**. Se mueve la rueda.
- Trapecio inferior y superior: sólidos rígidos que giran alrededor del eje definido por sus dos anclajes al chasis (1 grado de libertad cada uno).
- Mangueta: sólido rígido unido a las dos rótulas y a la bieleta.
- Bieleta: barra de longitud fija entre la cremallera y la mangueta.
- Total: **1 GdL** (recorrido) **+ 1 GdL** de entrada (cremallera).

## Algoritmo para un ángulo θ del brazo inferior
1. **Rótula inferior**: girar LBJ un ángulo θ alrededor del eje del brazo inferior (fórmula de Rodrigues).
2. **Rótula superior**: buscar el ángulo φ del brazo superior tal que |UBJ(φ) − LBJ| = longitud original del pivote. Es una ecuación de una incógnita → Newton.
3. **Rótula de la bieleta (TRo)**: está a distancia fija de LBJ, de UBJ (mangueta rígida) y de la bieleta interior (desplazada por la cremallera). Es la **intersección de tres esferas** (trilateración). De las dos soluciones se toma la más cercana a la anterior.
4. **Transformación rígida**: con los tres puntos (LBJ, UBJ, TRo) antes y después se construyen dos bases ortonormales y se obtiene la rotación R = F₁·F₀ᵀ. Con ella se mueven el centro de rueda, el eje de mangueta y cualquier otro punto de la mangueta.
5. **Contacto**: punto más bajo de la circunferencia de rueda (neumático rígido).

## Imponer el recorrido
El usuario impone **el desplazamiento vertical del centro de rueda**, no θ. Se busca θ con Newton (derivada numérica), arrancando de la última solución (*arranque en caliente*). Por eso el barrido se hace desde 0 hacia fuera.

## Comprobaciones (test)
`tests/test_kinematics.cpp` verifica que en todo el barrido se conservan las longitudes rígidas (error < 1e-5 mm) y que en recorrido 0 los puntos no se mueven.

## Limitaciones y siguientes pasos
- Solo doble trapecio. Para McPherson, multibrazo o push-rod conviene un **solver genérico**: todas las uniones como restricciones de distancia `|pᵢ − pⱼ|² − Lᵢⱼ² = 0` y resolver el sistema con Newton-Raphson multidimensional (jacobiano numérico o analítico). Ver [[01 Roadmap]] fase 3.
- Sin elasticidades (casquillos rígidos).
- Si la geometría se "bloquea" (las esferas no se cortan) el estado sale `valid = false` y la interfaz lo indica.

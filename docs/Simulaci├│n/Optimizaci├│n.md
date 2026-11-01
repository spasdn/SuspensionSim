---
tags: [simulacion, optimizacion]
---
# Optimización

Código: `src/analysis/Optimizer.cpp`. Panel *Optimizacion* en la app.

## Cómo funciona
1. **Variables de diseño**: coordenadas de puntos duros que se permiten mover, con un rango (p. ej. `tie_rod_inner.Z` entre −30 y +30 mm).
2. **Objetivo**: una función de coste (menor = mejor) que se evalúa haciendo un barrido de recorrido completo con el solver.
3. **Algoritmo**: Nelder-Mead (simplex). No necesita derivadas, va bien con pocas variables (≲ 10). Los límites se aplican recortando.
4. Al terminar muestra el resultado; *Aplicar resultado al modelo* lo copia al modelo actual y *Guardar* lo escribe en el JSON.

## Objetivos disponibles
| Objetivo | Coste |
|---|---|
| Minimizar bump steer | RMS de (convergencia − convergencia de diseño) en el barrido [°] |
| Ganancia de caída objetivo | (ganancia [°/10 mm] − objetivo)² |
| Altura de CB objetivo | ((h_CB − objetivo)/10)² |
| Minimizar migración del CB | máx − mín de la altura del CB en el barrido |

Ejemplo (test): mover solo la Z de la bieleta interior baja el RMS de bump steer del ejemplo de 1,30° a 0,07° (−20 mm).

## Añadir un objetivo
1. Nuevo valor en `enum class ObjectiveType` (`Optimizer.h`).
2. Su nombre en `objectiveName()`.
3. Su coste en `evaluateObjective()`.
4. Añadirlo a la lista `names[]` de `panelOptimizer()`.

## Siguientes pasos
- [ ] **Multiobjetivo ponderado**: coste = Σ wᵢ · costeᵢ normalizado
- [ ] **Restricciones** con penalización (interferencia con la llanta, ángulos de rótula, longitudes)
- [ ] Ejecutar en otro hilo (`std::thread`) con barra de progreso: ahora la ventana se congela mientras optimiza
- [ ] Algoritmos globales (enjambre de partículas, evolución diferencial) para evitar mínimos locales
- [ ] Análisis de sensibilidad: cuánto cambia cada parámetro al mover cada punto 1 mm (muy útil para entender el diseño antes de optimizar)

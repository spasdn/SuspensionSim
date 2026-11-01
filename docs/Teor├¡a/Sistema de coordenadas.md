---
tags: [teoria]
---
# Sistema de coordenadas

Se usa el sistema de vehículo **ISO 8855**:

| Eje | Sentido positivo | Color en la ventana |
|---|---|---|
| X | hacia **delante** | rojo |
| Y | hacia la **izquierda** | verde |
| Z | hacia **arriba** | azul |

- Unidades: **mm**.
- Origen recomendado: en el suelo, en el plano de simetría del coche, a la altura del eje delantero (X = 0 en el centro de rueda delantero). El solver no lo exige, pero el **centro de balanceo** se calcula cortando con el plano **Y = 0**, así que el plano de simetría tiene que ser Y = 0.
- El suelo es el plano horizontal que pasa por el contacto del neumático en la posición de diseño.

> [!warning] Ojo con el CAD
> SolidWorks, Inventor, etc. suelen tener **Y hacia arriba**. Al pasar puntos del CAD al JSON:
> X_iso = Z_cad (o −Z según cómo esté orientado el coche), Y_iso = X_cad, Z_iso = Y_cad. Comprueba siempre con un punto conocido (el centro de rueda) en la vista frontal.
> Otra opción: crea en el CAD un sistema de coordenadas propio con la orientación ISO y mide respecto a él.

## Lado izquierdo y derecho
Se modela una esquina (por defecto la **izquierda**, Y > 0). La derecha se dibuja reflejada (Y → −Y). Los parámetros de salida usan el criterio de signo habitual en ambos lados gracias a `sideSign()` (+1 izquierda, −1 derecha).

## Recorrido
- **Recorrido > 0**: compresión (*bump*), la rueda sube respecto al chasis.
- **Recorrido < 0**: extensión (*rebound / droop*).
- El chasis se considera fijo y es la rueda la que se mueve (equivale a mover el chasis con el suelo fijo).

Relacionado: [[Parámetros de suspensión]] · [[Formato JSON de hardpoints]]

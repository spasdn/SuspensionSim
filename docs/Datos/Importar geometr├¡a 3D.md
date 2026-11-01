---
tags: [datos, cad]
---
# Importar geometría 3D

## Qué se puede importar hoy
- **Chasis o cualquier pieza como malla `.obj`** (solo vértices y caras). Se dibuja transparente con sus aristas. Se indica en el JSON con `"chassis_mesh"`. Lector: `io/ObjLoader.cpp`.
- Los **puntos duros** no se leen del 3D: se introducen en el JSON (es lo habitual en software de suspensiones porque los puntos son pocos y tienen que ser exactos).

## Desde el CAD (SolidWorks, Inventor, Fusion…)
1. Crea en el ensamblaje un **sistema de coordenadas ISO** (X adelante, Y izquierda, Z arriba) en el suelo, plano de simetría, eje delantero. Ver [[Sistema de coordenadas]].
2. Exporta la pieza como **STL** (en binario o ASCII) usando ese sistema de coordenadas y en **mm**.
3. Convierte STL → OBJ con **Blender** (*Import STL → Export OBJ*, desmarcando normales/UV y con "Forward: Y, Up: Z" para que no gire los ejes) o con **MeshLab**.
4. Si sale enorme (millones de triángulos): en Blender, modificador *Decimate* hasta unas decenas de miles.
5. Si las unidades no cuadran (todo diminuto): estaba en metros → usa `scale = 1000` en `loadObj()`.

## Puntos duros desde el CAD
Opción manual (fiable): medir cada punto respecto al sistema ISO y copiarlo al JSON.
Opción semiautomática (futura): exportar desde el CAD una tabla de puntos (bocetos 3D con puntos con nombre → macro → CSV) y leerla. Anotado en el [[01 Roadmap]].

## Pendiente
- [ ] Lector STL directo (formato muy simple: 50 bytes por triángulo en binario)
- [ ] Malla estática en GPU para mallas grandes → [[Cómo dibujar algo nuevo]]
- [ ] Importar puntos desde CSV

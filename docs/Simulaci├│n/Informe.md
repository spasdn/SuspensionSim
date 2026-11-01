---
tags: [simulacion, informe]
---
# Informe

Botón *Informe → Generar informe* en la aplicación. Genera:
- `docs/Informes/Informe <modelo> <fecha hora>.md` → aparece aquí en Obsidian, con propiedades (tipo, modelo, fecha…) para filtrarlo.
- `output/Informe <modelo> <fecha hora>.csv` → todo el barrido, separado por `;`, para LibreOffice/Excel.

Ejemplo generado: [[Ejemplo - informe generado]]

## Contenido
1. Condiciones de contorno usadas
2. Parámetros en posición de diseño ([[Parámetros de suspensión]])
3. Ganancias (caída, bump steer, migración del CB, variación de vía)
4. Tabla del barrido
5. Puntos duros
6. Notas (p. ej. resultado de la última [[Optimización]])

## Ampliaciones previstas
- [ ] Gráficas en el informe: exportar PNG desde la app o generar gráficas con el plugin *Charts* de Obsidian a partir del CSV
- [ ] Captura de la vista 3D (`glReadPixels` → PNG con *stb_image_write*)
- [ ] Comparación de dos diseños
- [ ] Exportar a PDF (desde Obsidian ya se puede: *Exportar a PDF*)

## Listar informes (plugin Dataview, opcional)
```dataview
TABLE modelo, fecha, eje FROM "Informes" WHERE tipo = "informe" SORT fecha DESC
```

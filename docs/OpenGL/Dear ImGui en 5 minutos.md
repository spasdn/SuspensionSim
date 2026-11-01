---
tags: [imgui, aprendizaje]
---
# Dear ImGui en 5 minutos

ImGui es una librería de interfaz **de modo inmediato**: la interfaz se declara de nuevo en cada fotograma, dentro del bucle principal. No hay objetos "botón" que guardar.

```cpp
ImGui::Begin("Mi panel");                      // abre una ventana
ImGui::Text("Caida: %.2f deg", m.camber);      // texto con formato printf
if (ImGui::Button("Calcular")) calcular();     // true el fotograma en que se pulsa
ImGui::SliderFloat("Radio", &radio, 200, 400); // modifica 'radio' directamente
ImGui::Checkbox("Ver chasis", &verChasis);
ImGui::End();
```
Cada widget devuelve `true` si **ha cambiado** este fotograma: así sabemos cuándo recalcular (ver `panelModel()`: `if (changed) rebuildSolver();`).

## Widgets más útiles para este proyecto
| Widget | Uso |
|---|---|
| `DragScalarN(..., ImGuiDataType_Double, &v.x, 3, ...)` | Editar un punto (x,y,z) en double |
| `SliderScalar` / `InputDouble` | Valores numéricos |
| `Combo` | Elegir de una lista |
| `CollapsingHeader` / `TreeNode` | Secciones plegables |
| `BeginTable` / `TableNextRow` | Tablas de resultados |
| `PlotLines` | Gráfica simple de una lista de valores |
| `SeparatorText` | Separador con título |

## Etiquetas e IDs
ImGui identifica cada widget por su texto. Si hay dos con el mismo texto, choca. Soluciones:
- `"##id"`: texto invisible (`ImGui::Combo("##eje", ...)`).
- `ImGui::PushID(i)` / `PopID()` dentro de un bucle (ver `panelOptimizer()`).

## El ratón: ¿ImGui o la escena 3D?
`ImGui::GetIO().WantCaptureMouse` es `true` si el ratón está sobre un panel. `App::handleCameraInput()` lo comprueba para no girar la cámara mientras arrastras un slider.

## Aprender viendo
En la app: *Vista → Demo de ImGui (para aprender)*. Abre una ventana con **todos** los widgets; el código de cada uno está en `build/_deps/imgui-src/imgui_demo.cpp` (busca el texto del widget).

## Fuente, acentos y eñes
Los textos de los paneles se han escrito sin tildes por prudencia. La fuente por defecto de ImGui es pequeña y pixelada; para una interfaz más legible (y con tildes garantizadas) carga una fuente TTF al iniciar, después de `ImGui::CreateContext()`:
```cpp
ImGui::GetIO().Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 17.0f);
```
Tarea pequeña para el [[01 Roadmap]].

Relacionado: [[OpenGL desde cero]]

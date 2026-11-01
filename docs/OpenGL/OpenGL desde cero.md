---
tags: [opengl, aprendizaje]
---
# OpenGL desde cero (lo justo para este proyecto)

> [!tip] Idea clave
> OpenGL **no** dibuja "una suspensión". Solo sabe dibujar **puntos, líneas y triángulos** que le pasas como listas de coordenadas. Todo lo demás (rueda, trapecios, chasis) son muchas líneas y triángulos. En este proyecto ya está resuelto: tú llamas a `addLine()` / `addTriangle()` y listo.

## 1. Las piezas y quién hace qué

| Pieza | Qué hace | Dónde está en el código |
|---|---|---|
| **GLFW** | Abre la ventana, crea el *contexto* OpenGL, lee ratón y teclado | `App::init()` |
| **glad** | "Engancha" las funciones de OpenGL del driver de tu gráfica. Sin él, `glDrawArrays` no existe | `gladLoadGLLoader(...)` en `App::init()` |
| **GLM** | Vectores y matrices (`glm::vec3`, `glm::mat4`, `lookAt`, `perspective`) | En todas partes |
| **Shaders** | Pequeños programas que se ejecutan en la GPU | `render/Renderer.cpp` (texto GLSL) |
| **VBO / VAO** | Memoria de la GPU con los vértices / la "receta" para leerlos | `Renderer::init()` |
| **Dear ImGui** | Botones, sliders y tablas dibujados encima de la escena | `app/Panels.cpp` |

## 2. El bucle principal
Un programa gráfico es un bucle que se repite ~60 veces por segundo (`App::run()`):

```cpp
while (!ventana_cerrada) {
    leer_eventos();                 // glfwPollEvents
    construir_interfaz();           // ImGui
    borrar_pantalla();              // glClear
    dibujar_escena_3d();            // nuestras líneas y triángulos
    dibujar_interfaz();             // ImGui encima
    presentar();                    // glfwSwapBuffers (doble buffer: se dibuja oculto y se muestra de golpe)
}
```
**No hay que "actualizar" nada a mano**: cada fotograma se vuelve a dibujar todo desde el estado actual. Si cambias un punto, el siguiente fotograma ya sale cambiado.

## 3. Del punto 3D al píxel: las matrices
Cada vértice pasa por esta cadena (en el *vertex shader*):

```
pantalla = Proyección × Vista × (Modelo) × posición_en_mundo
```
- **Modelo**: coloca un objeto en el mundo. Aquí no la usamos: los puntos ya están en coordenadas del coche.
- **Vista** (`Camera::view()`): mueve el mundo para que parezca que la cámara está donde la pusimos. Se calcula con `glm::lookAt(posición_cámara, objetivo, arriba)`. Usamos Z como "arriba" (ver [[Sistema de coordenadas]]).
- **Proyección** (`Camera::projection()`): perspectiva (lo lejano más pequeño) u ortogonal (como un plano técnico).

En el código es una línea: `renderer_.flush(camera_.projection(aspect) * camera_.view());`

## 4. Shaders en 30 segundos
Son dos funciones en GLSL (parecido a C) que corren en la GPU:

```glsl
// VERTEX: una vez por vértice -> dónde cae en pantalla
gl_Position = uViewProj * vec4(aPos, 1.0);

// FRAGMENT: una vez por píxel -> de qué color es
FragColor = vColor;
```
- `in` / `layout(location=N) in`: datos de cada vértice (posición, color).
- `uniform`: constantes para toda la llamada (la matriz de cámara).
- `out` del vertex → `in` del fragment: se interpolan entre vértices.

Nuestro shader es el más simple posible: no hay luces en la GPU. El sombreado de los triángulos se calcula en la CPU en `Renderer::addTriangle()` (más claro si la cara mira a una "luz" fija). Para empezar es más que suficiente.

## 5. VBO y VAO (la parte más rara de OpenGL)
- **VBO** (*Vertex Buffer Object*): un bloque de memoria en la tarjeta gráfica. Le copiamos un array de `Vertex { vec3 pos; vec4 color; }`.
- **VAO** (*Vertex Array Object*): guarda *cómo* leer ese bloque: "el atributo 0 son 3 floats al principio de cada vértice, el atributo 1 son 4 floats después".

Se configura **una vez** en `Renderer::init()` con `glVertexAttribPointer`. Después, en cada fotograma:
```cpp
glBufferData(GL_ARRAY_BUFFER, bytes, datos, GL_DYNAMIC_DRAW); // copiar a la GPU
glDrawArrays(GL_LINES, 0, numVertices);                       // dibujar
```
Con `GL_LINES` cada 2 vértices son una línea; con `GL_TRIANGLES`, cada 3 un triángulo; con `GL_POINTS`, cada vértice un punto.

## 6. Profundidad y transparencia
- `glEnable(GL_DEPTH_TEST)`: lo que está detrás queda tapado (*z-buffer*).
- Transparencia: `glEnable(GL_BLEND)` + alfa < 1 en el color. Los triángulos transparentes se dibujan **al final** y sin escribir profundidad (`glDepthMask(GL_FALSE)`), si no, tapan lo que hay detrás. El renderer ya lo hace: si el color tiene `a < 1` va a la lista de transparentes.

## 7. Lo que NO necesitas (de momento)
Texturas, iluminación por píxel, sombras, framebuffers, instancing. Si algún día quieres el chasis "bonito", el siguiente paso sería pasar normales al shader e iluminar en el fragment shader (capítulo *Basic Lighting* de LearnOpenGL).

## 8. Para aprender más
- **LearnOpenGL** (learnopengl.com, también en español): capítulos *Hello Window*, *Hello Triangle*, *Shaders*, *Transformations*, *Coordinate Systems*, *Camera*. Con esos seis entiendes todo el código del proyecto.
- Documentación de funciones: docs.gl
- Demo de ImGui: en la app, *Vista → Demo de ImGui*. Ver [[Dear ImGui en 5 minutos]].

## Ejercicios propuestos (para soltarse)
- [ ] Cambia el color de fondo en `App::run()` (`glClearColor`).
- [ ] Dibuja una línea del centro de rueda al centro de balanceo en `SceneBuilder.cpp`.
- [ ] Cambia el tamaño de los puntos (`flush(..., pointSize)`) y añade un slider para él.
- [ ] Añade una vista "trasera" a `Camera` (yaw = 180).
- [ ] Dibuja la huella del neumático como un rectángulo (2 triángulos) en el suelo.

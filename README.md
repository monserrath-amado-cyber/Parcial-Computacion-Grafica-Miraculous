# Parcial Final - Computación Gráfica: Escena 3D Miraculous

Escena gráfica interactiva en 3D desarrollada en **C++** con **OpenGL 3.3 Core Profile**, inspirada en el universo de *Miraculous*. El proyecto integra carga de modelos 3D en formato `glTF`, iluminación dinámica de Phong mediante shaders externalizados, generación procedural de geometría, animaciones orbitales con curvas de Bézier cúbicas e interfaz gráfica en tiempo real.

---

## Características Principales

* **Carga de Modelos 3D (`glTF`):** Importación de personajes principales (*Ladybug* y *Chat Noir*) y cuatro accesorios (*Aretes*, *Anillo*, *Broche Pavo Real* y *Broche Mariposa*) con texturas PBR mediante `tiny_gltf` y `stb_image`, incluyendo cálculo automático de *bounding box* para centrar el pivote de rotación de los objetos flotantes.
* **Shaders Externalizados (GLSL 330):** Separación completa de los programas de sombreado en la carpeta `/shaders`:
  * `lighting.vert` / `lighting.frag`: Implementación del modelo de iluminación de **Phong** (componentes ambiental, difusa y especular) con matriz normal (`transpose(inverse(model))`).
  * `background.vert` / `background.frag`: Renderizado procedural de fondo degradado vertical en tres tonos.
* **Geometría Procedural:** Generación matemática de la plataforma circular del escenario (64 segmentos trigonométricos) y el plano de fondo.
* **Animación con Curvas de Bézier Cúbicas:** Trayectorias orbitales cerradas ($P_0 = P_3$) evaluadas polinómicamente para cada Miraculous, con opción de visualizar las curvas en tiempo real mediante `GL_LINE_STRIP`.
* **Cámara Libre (FPS) e Interfaz Interactiva:** Navegación espacial fluida sincronizada con `deltaTime` y panel de control flotante con **Dear ImGui** para modificar luces, rotaciones y velocidad de animación en vivo.

---

## Controles de Teclado y Ratón

| Tecla / Acción | Función |
| :--- | :--- |
| **`W` / `A` / `S` / `D`** | Desplazamiento horizontal de la cámara (adelante, izquierda, atrás, derecha) |
| **`Q` / `E`** | Subir o bajar la cámara en el eje vertical |
| **Movimiento del Ratón** | Rotación libre de la cámara (`Yaw` y `Pitch`) |
| **`M` o `TAB`** | Alternar entre modo cámara y cursor libre para interactuar con el menú **ImGui** |
| **`B`** | Mostrar / ocultar las líneas de trayectoria de las curvas de Bézier |
| **`Espacio`** | Pausar o reanudar la animación de los Miraculous |
| **`R`** | Restablecer la cámara a su posición y orientación inicial |
| **`ESC`** | Cerrar la aplicación |

---

## Tecnologías y Librerías Utilizadas

* **Lenguaje:** C++17
* **API Gráfica:** OpenGL 3.3 (Core Profile)
* **Gestión de Ventana y Entrada:** GLFW 3
* **Cargador de Funciones OpenGL:** GLAD
* **Matemáticas 3D:** GLM (OpenGL Mathematics)
* **Importación 3D y Texturas:** `tiny_gltf` & `stb_image`
* **Interfaz Gráfica:** Dear ImGui (`imgui_impl_glfw`, `imgui_impl_opengl3`)
* **Entorno de Desarrollo:** Visual Studio 2022 (x64)

---

## Estructura del Proyecto

```text
Parcial-Computacion-Grafica-Miraculous/
├── shaders/
│   ├── background.frag       # Fragment shader del cielo degradado
│   ├── background.vert       # Vertex shader del plano de fondo
│   ├── lighting.frag         # Fragment shader con iluminación de Phong
│   └── lighting.vert         # Vertex shader con matriz normal y MVP
├── lady_bug_-_miraculous/    # Modelo 3D y texturas de Ladybug (.gltf)
├── chat_noir_-_miraculous/   # Modelo 3D y texturas de Chat Noir (.gltf)
├── aretes/                   # Modelo 3D y texturas de los Aretes (.gltf)
├── anillo cat noir/          # Modelo 3D y texturas del Anillo (.gltf)
├── pavo real/                # Modelo 3D y texturas del Miraculous del Pavo Real (.gltf)
├── hockmoth/                 # Modelo 3D y texturas del Miraculous de la Mariposa (.gltf)
├── Source.cpp                # Código fuente principal (lógica, renderizado e ImGui)
├── Ladybug.slnx              # Solución de Visual Studio
└── README.md                 # Documentación del proyecto
```

---

## Compilación y Ejecución

1. Clonar el repositorio:
   ```bash
   git clone [https://github.com/monserrath-amado-cyber/Parcial-Computacion-Grafica-Miraculous.git](https://github.com/monserrath-amado-cyber/Parcial-Computacion-Grafica-Miraculous.git)
   ```
2. Abrir el archivo de solución **`Ladybug.slnx`** en **Visual Studio**.
3. Verificar que la configuración esté en **`Debug` / `Release`** y arquitectura **`x64`**.
4. Compilar y ejecutar presionando **`F5`**.

---

## Autora

* **Monserrath Amado**
* Video explicativo: https://www.youtube.com/watch?v=r2VkTe6phfk*
   

# Motor de Recomendacion de Contenido (C++17)

**Proyecto universitario de consola para Visual Studio 2022.** Incluye cinco estructuras de datos implementadas manualmente, un catalogo inicial, sesiones para usuarios normales y administrador, recomendaciones deterministas y persistencia local.

## Como abrirlo en Visual Studio

1. Instala Visual Studio 2022 con la carga de trabajo **Desarrollo para el escritorio con C++** (MSVC v143 y Windows SDK).
2. Abre `MotorRecomendacion.sln` desde Visual Studio (no confundir con Visual Studio Code).
3. En el Explorador de soluciones, haz clic derecho sobre **MotorRecomendacion** -> **Establecer como proyecto de inicio**.
4. Selecciona `Debug` y `x64`, y presiona **Ctrl+F5** para ejecutar o **F5** para depurar.
5. La carpeta de trabajo de la aplicacion esta configurada a `MotorRecomendacion/`. Los cuatro archivos de datos se guardan en `MotorRecomendacion/datos/`. Si se eliminan **los cuatro**, el sistema vuelve a crear el catalogo inicial automaticamente. No elimines solo uno, porque el programa no sobrescribe conjuntos incompletos.
6. Para ejecutar las pruebas, establece **Pruebas** como proyecto de inicio y ejecutalo. Restablece despues **MotorRecomendacion** como proyecto de inicio.

En caso de incompatibilidad con tu version de Visual Studio, revisa en las propiedades del proyecto la plataforma de herramientas (`v143` para Visual Studio 2022).

## Cuentas iniciales

| Rol | ID | Clave |
| --- | --- | --- |
| Administrador | `1` | `admin123` |
| Usuario normal de demostracion | `2` | `demo123` |
| Usuario normal Alex | `3` | `demo123` |

Las claves se almacenan **en texto plano solo con fines academicos**. No uses credenciales personales ni despliegues este ejemplo como un servicio real.

## Que incluye

- **TablaHash<T>**: encadenamiento separado mediante nodos, funcion modulo y redimensionamiento manual.
- **Grafo**: lista de vertices y listas enlazadas de adyacencia; conexiones no dirigidas con peso 0 a 1.
- **ArbolCategorias**: arbol N-ario por representacion *primer hijo / siguiente hermano*; jerarquias con `/`.
- **MatrizDispersa**: listas ortogonales enlazadas simultaneamente por usuario y contenido; solo guarda celdas con interacciones.
- **HeapMaximo**: arreglo dinamico con `subir` y `bajar`; obtiene candidatos en orden de afinidad descendente, con desempate por ID ascendente.
- **Sistema**, **MotorRecomendacion**, **GestorPersistencia**: coordinacion, recomendaciones y almacenamiento en archivos.

Los punteros y arreglos para las estructuras son propios. Se utiliza `std::string` para texto, `std::filesystem` y `fstream` para archivos, `std::unique_ptr<Candidato[]>` para una matriz temporal de resultados de interfaz; ninguno sustituye las cinco estructuras obligatorias.

## Catalogo inicial

Se incluyen **19 contenidos** educativos y de entretenimiento (programacion, videojuegos, ciencia ficcion, arte, musica, fisica y mas). Hay ejemplos de interacciones previas de los usuarios 2 y 3. Puedes agregar, editar y eliminar contenidos como administrador. Esas acciones reconstruyen automaticamente el arbol de categorias y las conexiones del grafo; al eliminar un contenido tambien se retiran sus interacciones.

## Funcion de recomendacion

El motor no es inteligencia artificial: usa una regla determinista sobre datos existentes.

```text
Afinidad = 0.35 * P + 0.25 * H + 0.30 * G + 0.10 * Popularidad
```

- `P`: proporcion de preferencias explicitas del usuario que coinciden, sin distinguir mayusculas ASCII, con titulo, ruta de categoria o etiquetas del contenido. Si no hay preferencias es 0.
- `H`: promedio ponderado de satisfaccion en contenidos anteriormente registrados de la misma categoria (peso `1.0`) o familia principal (peso `0.4`), con 0 si no hay historial relevante.
- `G`: promedio de satisfaccion ponderado por los pesos de similitud del grafo (0 a 1) de los contenidos anteriores, con 0 si no hay relaciones relevantes.
- `Popularidad`: atributo del contenido dividido entre 100.
- Satisfaccion de una calificacion 1 a 5 = `(calificacion - 1) / 4`. Sin nota, una visualizacion equivale a `0.5`; marcar favorito eleva su satisfaccion como minimo a `0.9`.

Los cuatro componentes permanecen en `[0,1]`, y los pesos suman `1.0`, por lo que la puntuacion final esta en `[0,1]`. El heap ordena por puntuacion descendente y, en caso de empate, por ID ascendente. **Se excluye cualquier contenido con interaccion registrada**, incluso si solo esta marcado como favorito.

Ejemplo numerico: `P=0.50`, `H=1.00`, `G=1.00`, `Popularidad=0.80`:

`0.35(0.50) + 0.25(1.00) + 0.30(1.00) + 0.10(0.80) = 0.805` -> afinidad de **80.5 %**.

### Calculo de similitudes del grafo

Las conexiones se generan de forma determinista al registrar, modificar o eliminar contenidos:
- Misma ruta completa de categoria: `+0.55`.
- Misma categoria raiz, distinta subcategoria: `+0.20`.
- Etiquetas compartidas: `+0.45 * (interseccion / union)` (coeficiente de Jaccard).
- Se aplica maximo `1.0` y solo se guarda una arista si el peso es al menos `0.20`.

Se guarda una sola copia de cada relacion en `similitudes.txt` y al cargarla se reconstruyen las dos direcciones de la arista.

## Persistencia

Archivos en `MotorRecomendacion/datos/`:

- `usuarios.txt`: ID, nombre, clave, rol, preferencias.
- `contenidos.txt`: ID, popularidad, titulo, categoria, etiquetas.
- `interacciones.txt`: ID usuario, ID contenido, vistas, calificacion, favorito.
- `similitudes.txt`: pares de ID de contenidos y pesos.

Los campos de texto se serializan con `std::quoted` para conservar espacios y comillas. Cada cambio se guarda inmediatamente. Se escriben primero archivos temporales y luego se reemplazan por los nuevos; **no existe una transaccion atomica conjunta entre los cuatro archivos**, limitacion de este prototipo. Ante un error de escritura, la operacion en memoria podria haberse realizado aunque se informe un fallo de persistencia.

## Estructura principal y complejidad

| Estructura | Operacion | Complejidad |
| --- | --- | --- |
| Hash | buscar/insertar/eliminar | promedio `O(1)`, peor `O(n)` |
| Grafo | localizar vertice y arista | `O(V + grado)` en la version enlazada |
| Arbol N-ario | insertar/buscar ruta | proporcional a la cantidad de hermanos recorridos en cada nivel |
| Matriz dispersa | consultar celda | `O(F + r)`; F = numero de encabezados de fila, r = elementos de la fila |
| Matriz dispersa | historial de usuario | `O(F + r)` |
| Heap | insertar/extraer maximo | `O(log n)` |
| Recomendacion | todos los candidatos | incluye exploracion del historial por candidato, insercion en heap `O(C log C)` mas consultas |

## Pruebas

El proyecto `Pruebas` verifica inserciones, colisiones y redimensionamiento de hash; relaciones del grafo; rutas del arbol; enlaces ortogonales y eliminaciones; orden del heap; exclusiones y normalizacion de recomendaciones; y guardado, reinicio y eliminacion de contenido. Las pruebas crean su propia carpeta temporal `datos_pruebas_motor` y la eliminan cuando terminan.

## Limitaciones deliberadas

Este es un prototipo de consola, no un servicio multiusuario ni un sistema comercial. No hay cifrado de claves, recuperacion de contrasenas, control de acceso frente a modificaciones directas de los archivos ni transacciones atomicas entre cuatro archivos. El control de rol se aplica en el menu de consola; una aplicacion real requeriria controles adicionales de seguridad. Las coincidencias de texto pasan a minusculas en ASCII y no implementan comparacion Unicode completa.

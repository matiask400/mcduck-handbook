# Revisión del handbook — 26 de octubre de 2026

Se revisó el contenido incluido por `main.tex` y se agregaron pruebas. **No se
modificaron algoritmos, explicaciones ni archivos `.tex` en esta revisión.**
Los cambios locales que ya existían se conservaron. El compilador del editor
compiló correctamente el proyecto `main.tex` desde `graphs/shortest_paths.tex`.
Eso valida LaTeX; el código C++ impreso necesita sus propias pruebas.

## Referencias consultadas primero

- [KACTL, KTH](https://github.com/kth-competitive-programming/kactl): código corto
  y pruebas contra implementaciones simples. Se adoptó ese método de validación,
  sin reemplazar las plantillas por las de otro equipo.
- [Notebook de Stanford](https://github.com/jaehyunp/stanfordacm) y su
  [índice](https://raw.githubusercontent.com/jaehyunp/stanfordacm/master/contents.txt):
  contraste de temas, flujos, geometría y estructuras.
- [CP-Algorithms](https://cp-algorithms.com/): contratos, complejidades y
  limitaciones numéricas; referencias concretas junto a los hallazgos.
- [USACO Guide](https://usaco.guide/): referencia complementaria de temas y uso
  en competencia.

## Resultado de las pruebas

La suite nueva ejecutó **1.843.759 comprobaciones** en siete grupos, pasando
con Clang + AddressSanitizer/UndefinedBehaviorSanitizer y con GNU G++ 15.2,
C++17 y `-O2`. Son comprobaciones de invariantes/resultados, no esa cantidad de
entradas independientes. Las entradas aleatorias usan semilla 20261026;
Treap/Pollard conservan sus generadores propios de las plantillas.

| Grupo nuevo | Comprobaciones | Oráculo principal |
|---|---:|---|
| Estructuras | 628.214 | Vector, sumas directas, ordenamiento y mapa de puntos |
| Grafos | 39.146 | Borrar vértices/aristas, enumerar bloques/cortes/flujos/asignaciones/caminos |
| Árboles | 906.663 | Subir padres, distancias directas y comprobar balance/ancestros |
| Matemática | 156.692 | Aritmética `i128/u128`, división por tentativa, Pascal y determinante por permutaciones |
| Geometría | 30.068 | Todos los pares, pertenencia al hull y ecuaciones de las intersecciones |
| Técnicas/DP | 75.973 | Ventanas cuadráticas, Mo directo y transición DP cuadrática |
| Base C++ | 7.003 | Identidades, resultados impresos, extremos y ejemplos |

También pasaron las **14 suites existentes**, incluyendo recuperaciones de BFS,
Dijkstra 0/1, Bellman–Ford, Floyd, DP, caminos hamiltonianos, flujo/corte y HLD.
La reconstrucción opcional de Dijkstra común se activó y probó en la suite
nueva. El resultado inicial de `validate_recovery_examples.py` fue un warning
de Clang por `move` sin `std::`, elevado a error; pasó conservando el código y
desactivando exclusivamente `-Wunqualified-std-cast-call`.

La integración recorre **85 entradas** contando `main.tex` y configuración:
los 82 archivos de contenido están incluidos. Valida sintaxis de 11 bloques
Python, ejecuta tres entradas del template Python, comprueba los resultados de
los ejemplos Python y prueba 10.000 operaciones de PBDS con GNU.
`geometry/convex_detour.tex` es una receta sin implementación C++: no se le
atribuye un test de ejecución. `core/includes_reference.tex` es una lista de
headers, revisada como tal. Los tests de los otros archivos prueban plantillas
y casos seleccionados; no certifican cada adaptación posible del lector.

Las **15 reproducciones adicionales fallan deliberadamente** contra el libro
actual bajo Clang (14 casos de ejecución y el template que Clang rechaza). GNU
G++ 15.2 acepta el template en el modo habitual; con `-pedantic-errors` rechaza
la declaración de alias. El runner devuelve
estado distinto de cero para conservar visibles esos hallazgos. Cuatro pruebas
de profundidad también fallan, como se detalla más abajo.

## Cambios que propongo decidir primero

| Prioridad | Archivo y evidencia | Propuesta, sin aplicar | Costo y contrapartida |
|---|---|---|---|
| Portabilidad | `core/template_cpp.tex:21`: `using ull = unsigned ll;` falla en Clang y con `-pedantic-errors` en GNU; GNU 15.2 normal lo acepta | Escribir `unsigned long long` | Sustituir una línea; ningún costo de ejecución |
| Alta | `graphs/shortest_paths.tex:233`: Floyd desborda con **40 vértices**, diagonal cero y aristas de peso **−1** | Manejar la suma y la cota inferior durante Floyd; mantener la detección de pares afectados por ciclos negativos | Conserva O(V³)/O(V²); promoción/comparaciones pueden agregar constantes: medir antes de elegir |
| Alta si n es grande | HLD, LowLink y rerooting fallan en cadenas de 200.000 nodos con pila de 8 MiB | Ofrecer recorrido iterativo o explicitar el límite de profundidad | Mismo orden de tiempo/memoria; recorrido iterativo exige más código. Euler LCA ya advierte este riesgo |
| Media | `data_structures/disjoint_intervals.tex:27`: `assign(3,3,99)` sobre `[0,10)` con valor 7 destruye la partición | Devolver inmediatamente para rango vacío; afirmar `left <= right` si se desea | Una guarda, O(1); no cambia la complejidad |
| Media | `math/matrices.tex:25`: `matrix_power({{42}},0,1)` devuelve 1; corresponde 0 | Identidad `1 % mod` y validar módulo positivo al entrar | Una línea sustituida y eventualmente una aserción; misma complejidad |
| Media | `techniques/binary_search.tex:19,25`: `r-l` desborda en `[INT_MIN,INT_MAX]`; `m+1` desborda si no hay respuesta en `INT_MAX` | Para dominio int, usar límites/centinela en `ll`; para todo el dominio ll, definir otra interfaz o usar i128 | Mismo O(log rango); hay que acordar cómo representar «no existe» |
| Media | `dp/divide_conquer_optimization.tex:22–27`: con `previous={LINF}` aborta | Documentar/exigir que cada estado procesado tenga candidato finito y acotar el rango de la capa; si se quieren estados imposibles, diseñar su tratamiento preservando monotonía | Restringir las llamadas evita costo adicional; no basta borrar el assert |

En Floyd los ciclos negativos hacen crecer la magnitud aunque los pesos sean
pequeños; no alcanza con verificar que cada peso entre en `ll`.
[CP-Algorithms explica este desbordamiento](https://cp-algorithms.com/graph/all-pair-shortest-path-floyd-warshall.html#the-case-of-negative-cycles).
La búsqueda binaria también puede desbordar con la fórmula `l+(r-l)/2` cuando
los extremos tienen signos distintos, como advierte su
[referencia de búsqueda binaria](https://cp-algorithms.com/num_methods/binary_search.html).
La optimización de DP requiere probar la monotonía y definir correctamente los
[estados y rangos de transición](https://cp-algorithms.com/dynamic_programming/divide-and-conquer-dp.html).
El [borrador de C++](https://eel.is/c++draft/dcl.type.general) permite combinar
`unsigned` con los especificadores enteros indicados, no con un alias typedef.
En el GNU local, el alias aceptado se comprobó como unsigned long long de ocho
bytes; no se atribuye a ese compilador un error de ejecución por esta línea.

## Límites de uso e inconsistencias adicionales

Estos casos no justifican hacer más lento todo el código automáticamente.
Para competencia, varias restricciones se pueden documentar y comprobar con
los límites del enunciado.

| Caso reproducible | Qué ocurre | Decisión recomendada |
|---|---|---|
| `dijkstra_large` | Una arista de peso `LINF+1` se devuelve como inalcanzable | Mantener el heap para grafos dispersos y documentar que distancias finitas deben ser menores que LINF y las sumas evaluadas deben entrar en ll; variante amplia sólo si hace falta |
| `window_threshold` | `longest_window_signed({0},LLONG_MIN)` desborda en `prefix-limit` | Promover esa diferencia o explicitar el rango. Además las sumas prefijas deben entrar en ll |
| `point_subtraction` | Distancia entre `(LLONG_MIN,0)` y `(1,0)` desborda **antes** del producto i128 | Documentar límites de coordenadas/diferencias; para coordenadas extremas promover antes de restar. El producto también debe entrar en i128 |
| `bcc_loop` | Un único vértice con un lazo no devuelve ningún bloque | Definir que no se aceptan lazos o tratar correctamente el vértice aislado después de ignorarlos. Aristas paralelas sin lazos sí pasaron |
| `base_roundtrip` | Se genera `-2A` desde −42, pero `from_base` rechaza el signo | Documentar parser no negativo o admitir signo y comprobar overflow. La conversión a base sí maneja LLONG_MIN |
| `flow_reverse_index` | El índice inverso de un lazo apunta a sí mismo | Excluir lazos de `add_edge` o ajustar sus índices. **No falló el valor de max-flow** en los tests con lazos; los helpers de seguimiento ya exigen extremos distintos |
| `stl_empty` | `common_algorithms({})` desreferencia el resultado de min/max sobre vacío | Añadir guarda o precondición visible de vector no vacío; el resto de ejemplos con vacío que lo admiten se probó |
| `mcmf_disconnected_cycle` | Camino 0→1 de costo 0 más ciclo 2→3→2 de costo −1: devuelve costo 0; el flujo con circulación cuesta −1 | Exigir ausencia de ciclos negativos de capacidad positiva en todo el modelo si se usa como min-cost general; la condición actual «alcanzables» es insuficiente para optimizar circulaciones desconectadas |

El último caso distingue un flujo de costo mínimo general de enviar solamente
caminos s→t. No propongo agregar cancelación de ciclos a la plantilla habitual:
agregaría complejidad para un caso que se puede excluir explícitamente.
[KACTL también excluye ciclos negativos en su MCMF](https://raw.githubusercontent.com/kth-competitive-programming/kactl/main/content/graph/MinCostMaxFlow.h).

## Rendimiento medido y elección para ICPC

GNU G++ 15.2, C++17, `-O2`, macOS ARM64. Mediana de tres ejecuciones; RSS máximo
de esas tres. Cada caso corre en un proceso independiente con pila de 8 MiB.
El tiempo incluye arranque, generación de entrada sintética y verificaciones;
excluye compilación. No son tiempos de un juez oficial ni límites universales
de ICPC. Estos seis escenarios exitosos quedan por debajo de un presupuesto
**hipotético de 1 s y 256 MiB en esta máquina**.

| Plantilla | Entrada y trabajo | Tiempo | RSS pico | Resultado local |
|---|---|---:|---:|---|
| Segment Tree | n=200.000, un millón de updates + un millón de queries | 42,59 ms | 9,56 MiB | Pasa el presupuesto local |
| Kosaraju | DAG, V=200.000, E=399.998 | 36,49 ms | 19,52 MiB | Pasa el presupuesto local |
| NTT | Dos vectores de 262.144 coeficientes | 75,68 ms | 9,66 MiB | Pasa el presupuesto local |
| Suffix Array + LCP | 200.000 caracteres, alfabeto de cuatro letras | 43,01 ms | 9,56 MiB | Pasa el presupuesto local |
| Dijkstra | V=200.000, E=399.999, pesos no negativos | 47,04 ms | 18,13 MiB | Pasa el presupuesto local |
| HLD | Construir árbol aleatorio de 200.000 nodos | 49,14 ms | 23,11 MiB | Pasa el presupuesto local |
| HLD / LowLink / rerooting / Euler LCA | Cadena de 200.000 nodos | — | — | Las cuatro fallan por pila en las tres ejecuciones |

La prueba de la cadena demuestra un problema de forma/profundidad, aunque el
algoritmo use O(n) memoria total. No se debe interpretar el RSS del proceso que
falló como memoria suficiente para terminar. Cambiar el límite de pila o el
compilador puede cambiar el umbral del fallo.

No se propone optimizar por intuición:

- **Mantendría Dijkstra con `priority_queue`** para grafos dispersos: descarta
  entradas viejas y es la variante práctica estándar descrita en
  [CP-Algorithms](https://cp-algorithms.com/graph/dijkstra_sparse.html). En grafos
  muy densos puede convenir O(V²), y con pesos 0/1 ya está la alternativa lineal.
  En multigrafos con un número no acotado de paralelas, la cota del heap debe
  expresarse con log E, en vez de asumir automáticamente log V.
- **Mantendría Segment Tree/Fenwick, HLD y NTT en su forma actual** en cuanto a
  orden de complejidad para las operaciones que implementan. En HLD sí atendería
  la profundidad. Los tests no demuestran optimalidad universal.
- **Suffix Array no es asintóticamente el más rápido:** usa sort por ronda,
  O(n log² n). Hay una variante O(n log n) con counting/radix sort, como la de
  [KACTL](https://raw.githubusercontent.com/kth-competitive-programming/kactl/main/content/strings/SuffixArray.h).
  El benchmark actual tiene margen; sólo compararía esa alternativa si el
  enunciado pide mucho más volumen. No se implementó ni se inventó un speedup.
- **MCMF con SPFA** conserva un peor caso O(FVE); potenciales + Dijkstra pueden
  reducirlo a O(VE + FE log V), a costa de más código e inicialización con
  negativos. Es una alternativa por medir para redes grandes, no una sustitución
  automática. [Referencia de costos](https://cp-algorithms.com/graph/min_cost_flow.html#complexity).
- **ODT** no garantiza buen peor caso; eso ya está advertido. En concreto,
  la asignación incluida puede borrar bloques amortizadamente, por lo que no
  hay que extrapolarle sin más el comportamiento de variantes ODT con sumas o
  recorrido de intervalos. Por análisis de esta implementación, cada asignación
  agrega O(1) cortes y cada intervalo borrado se cobra una sola vez: para sólo
  asignaciones, con I₀ intervalos iniciales y q operaciones, una cota total es
  O((I₀+q) log(I₀+q)). Conviene aclarar que la advertencia cuadrática se refiere
  a operaciones/variantes ODT más generales. Para garantía por operación usar
  Lazy Segment Tree.
- **LCS/edit/palíndromos** mantienen tablas cuadráticas para reconstruir.
  Sigue vigente el [benchmark previo](new_modules_benchmarks.md): LCS 10.000²
  excede 256 MiB. Es un límite documentado, no un nuevo error; Hirschberg sería
  otra decisión de memoria, simplicidad y constantes.
- **Aho, BCC y Virtual Tree** ya documentan sus costos corregidos. No apareció
  evidencia nueva que justifique reescribirlos para los casos probados.

## Líneas de código y comparación antes/después

**Cambios de implementación aplicados: 0 líneas.** Por lo tanto no hay un
algoritmo «después» cuyo tiempo haya aumentado. Los benchmarks describen la
versión actual. Cualquier modificación futura necesita su comparación aparte.
Estas son líneas de listings C++ actuales, contando comentarios, ejemplos y
líneas en blanco, no todo el archivo LaTeX:

| Archivo | Líneas C++ actuales | Delta en esta revisión |
|---|---:|---:|
| Template C++ | 26 | 0 |
| Caminos mínimos (cuatro algoritmos y ejemplos) | 194 | 0 |
| ODT | 24 | 0 |
| Matrices | 27 | 0 |
| Búsqueda binaria | 28 | 0 |
| Divide and Conquer DP | 22 | 0 |
| HLD | 89 | 0 |
| LowLink | 50 | 0 |
| Rerooting | 35 | 0 |
| Suffix Array y LCP | 54 | 0 |
| Flujos (tres algoritmos y ejemplos) | 186 | 0 |

## Archivos de pruebas y reproducción

Desde la raíz del repositorio:

```sh
# Casos normales: debe terminar en cero.
python3 tests/audit_book.py --case correctness \
  --cxxflag=-fsanitize=address,undefined --cxxflag=-fno-sanitize-recover=all

# Auditoría completa: actualmente termina en 1 por los hallazgos confirmados.
python3 tests/audit_book.py \
  --cxxflag=-fsanitize=address,undefined --cxxflag=-fno-sanitize-recover=all

# Una reproducción aislada.
python3 tests/audit_book.py --case floyd_negative_overflow \
  --cxxflag=-fsanitize=address,undefined --cxxflag=-fno-sanitize-recover=all

# Benchmark sin sanitizadores; estado 1 porque las cadenas fallan.
python3 tests/audit_book.py --compiler g++ --benchmark

# main.tex, Python; PBDS opcional requiere GNU y sus headers.
python3 tests/audit_integration.py
python3 tests/audit_integration.py --with-pbds --compiler g++
```

En esta Mac `g++` apunta a Apple Clang. Para GNU se usó `--compiler g++-15` y:

```sh
--cxxflag=-isysroot \
--cxxflag=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk \
--cxxflag=-idirafter \
--cxxflag=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
```

Los resultados completos, comandos adicionales, semillas y hashes de fuentes
están en `audit_sanitized_results.json`, `audit_gcc_results.json`,
`audit_benchmark_gcc_results.json`, `audit_benchmark_results.json`,
`audit_integration_results.json` y `audit_existing_results.json`.
El harness creció durante la revisión; los benchmarks guardan el hash exacto
del harness usado y las mismas plantillas del libro.

`validate_snippets.py` también se ejecutó y falló: su extractor no admite el
espacio entre `lstlisting` y `[style=cpp]`, omite el template y luego no conoce
`ll`. Además clasifica Python por `language/title` pero el libro usa `style=py`.
No debe contarse como una validación general exitosa. Los nuevos tests reconocen
ambas formas y separan definiciones de los ejemplos que van dentro de `main`.

La suite nueva conserva los fallos, sin parchear los algoritmos dentro del test.
Para la reconstrucción de Dijkstra sólo se activan las tres líneas opcionales
publicadas. Para compilar el template con Clang local se sustituye únicamente
`bits/stdc++.h` por headers estándar; la declaración inválida se mantiene.

La evidencia permite decidir correcciones concretas. No equivale a Accepted en
un juez, a una prueba formal de todos los algoritmos ni a garantizar que toda
entrada válida de cualquier problema pase sus límites.

# Nuestro handbook frente a KACTL

Comparación local del 26 de octubre de 2026. **No se cambió código ni texto del
libro. Este Markdown queda local, fuera de los commits.**

Se descargó [KACTL de KTH](https://github.com/kth-competitive-programming/kactl)
en un directorio temporal, fijado al commit
`b3548e973f8772010c1ef84e6d427c749733fd2c`. Los resultados no se atribuyen a otra
versión ni a una ejecución en el juez ICPC.

## Los 81 stress tests originales

Se seleccionaron los 81 `.cpp` que selecciona el
[runner de KACTL](https://github.com/kth-competitive-programming/kactl/blob/b3548e973f8772010c1ef84e6d427c749733fd2c/doc/scripts/run-all.sh).
Se intentó compilar y ejecutar cada uno, sin reducir bucles ni editar los tests.
Se usaron sus flags C++17/O2/Wall/Wconversion/Wfatal-errors, GNU G++ 15.2 y las
rutas del SDK necesarias en esta Mac. Cada test tuvo su propio ejecutable;
compilación y ejecución podían solaparse con otro test. Sus tiempos de stress
no se usan para comparar velocidades entre bibliotecas.

| Resultado | Cantidad |
|---|---:|
| Pasaron ejecutando sus comprobaciones | 74 |
| Se salteó a sí mismo por plataforma | 1 |
| Fallaron en ejecución | 3 |
| No compilaron en macOS | 2 |
| Agotó el límite local de 180 segundos | 1 |
| Total intentado | 81 |

No se afirma «81 aprobados» ni «todos los algoritmos de KACTL fallan». Sus
tests comprueban KACTL; no son una adaptación completa de cada test a nuestras
interfaces, y varios temas de la suite no existen en nuestro libro.

Las causas comprobadas fueron:

- **2-SAT:** terminó con SIGSEGV. Una ejecución de diagnóstico con Clang y
  AddressSanitizer, conservando el stress original y suministrando solamente
  un reemplazo de `bits/stdc++.h` con headers estándar, confirmó
  `stack-overflow`. El runner oficial amplía la pila a **512 MiB**; aquí la
  pila principal heredada ronda **8 MiB** y no pudo ampliarse.
- **Miller–Rabin:** discrepa en `754221686909783899`. Nuestro Miller simple y
  Montgomery lo identifican como primo, igual que la referencia independiente
  del stress; el KACTL local lo rechaza. Esta Mac tiene `long double` con
  **53 bits** de mantisa. El
  [ModMulLL de KACTL](https://github.com/kth-competitive-programming/kactl/blob/b3548e973f8772010c1ef84e6d427c749733fd2c/content/number-theory/ModMulLL.h)
  declara que su rango grande requiere x87 de 80 bits; con precisión de 64
  bits totales limita el módulo a menos de 2^52. El número del fallo excede
  ese rango. No se extrapola el fallo a Linux x86 con x87.
- **ModMulLL:** detectó la mantisa incompatible y escribió `Skipped.`. Aunque
  su proceso devuelve cero, se contó como salteado y no como prueba aprobada.
- **ModSum:** el fallo fue de la **comprobación estadística aproximada del
  stress**, no del valor calculado. Con `n=134456, c=1129900996,
  k=6490600292, m=246235914`, devuelve `16554116784994`. La comprobación exacta
  con enteros arbitrarios y floor-sum devuelve el mismo número. Su error
  respecto de la aproximación m*n/2 es 1,019446e-5, apenas mayor que el umbral
  1e-5 usado por el stress.
- **DelaunayTriangulation y FastDelaunay:** sus tests usan `feenableexcept`,
  ausente en este entorno macOS. No se atribuyó esto al algoritmo geométrico.
- **Factor:** no terminó en 180 s. Se registra como timeout, sin convertirlo
  en una aprobación ni asegurar una causa única. Sus entradas grandes también
  exceden el rango declarado de ModMulLL para esta plataforma.

El registro está en `kactl_stress_results.json`; los diagnósticos separados,
incluyendo las adaptaciones de diagnóstico, en `kactl_diagnostics_results.json`.
Las adaptaciones de diagnóstico no reemplazan los resultados de la suite original.

## Cómo se comparó rendimiento

Se extrajo el código real de nuestros listings y se leyó el código de la copia
fijada de KACTL. Los nombres se aislaron en namespaces; no se sustituyeron los
algoritmos por reimplementaciones inventadas para el benchmark.

- Misma máquina macOS ARM64, GNU G++ 15.2, C++17 y `-O2` para ambos.
- Cinco ejecuciones por implementación y escenario, alternando cuál corre
  primero. Procesos separados: las cachés estáticas de cada implementación
  empiezan en frío. La suite de stress ya había terminado.
- Entradas deterministas con semilla 20261026, generadas antes de empezar el
  cronómetro. Se excluyen compilación, arranque del proceso e impresión.
- El tiempo incluye construcción/preproceso y consultas cuando el escenario
  los pide. Se comparan resultados o checksums de resultados normalizados.
  En SCC se normaliza la partición completa por el menor vértice de cada bloque.
- RSS es el máximo del **proceso completo**, con entrada y resultado incluidos,
  no la memoria aislada de la estructura. En algunos casos se conservan las dos
  representaciones de entrada para excluir su conversión del tiempo. Por eso
  no equivale a la memoria mínima de una solución enviada al juez.
- Los casos de primalidad/factorización se limitaron al dominio común <=10^12;
  no se usa un cálculo incorrecto de KACTL fuera de su rango como speedup.
- No hay calentamiento fijo, afinidad de CPU ni máquina dedicada. Diferencias
  pequeñas se tratan como empate práctico; los resultados de ARM64 no predicen
  exactamente Linux x86. No hay una garantía universal de 1 s/256 MiB para ICPC.

Las configuraciones para igualar operaciones están registradas:

- Segment Tree de KACTL: sus parámetros documentados `T/f/unit` se configuraron
  para **suma de ll con identidad cero**, como el nuestro. No se comparó suma
  contra máximo ni ll contra int.
- Aho: se configuró su alfabeto documentado a minúsculas. Se enumeran todas las
  coincidencias en ambos, aunque la representación de salida es distinta.
- NTT: se fijó el módulo de su dependencia ModPow a 998244353 y se eliminó la
  declaración duplicada del módulo al unir ambos headers. Es la configuración
  requerida para componerlos, sin cambiar las mariposas ni la convolución.
- Li Chao/LineContainer: se negaron pendientes, ordenadas y resultado para
  comparar mínimos contra mínimos; son estructuras diferentes para la misma
  operación en el dominio ensayado.
- Suffix Array: se comparan SA y LCP; se elimina el sufijo centinela y se ajusta
  el índice de LCP de KACTL fuera del tiempo del algoritmo.
- Binary lifting: el tiempo de KACTL incluye construir padres y profundidades,
  que exige como entrada; nuestro constructor ya los calcula.

Los registros de las entradas, flags, métricas de código, hashes y todas las
mediciones están en `kactl_comparison_results.json`. Distintas revisiones del
harness se identifican por hash; las plantillas del libro permanecieron iguales.

## Tiempos y memoria

La tabla muestra mediana en milisegundos y RSS máximo en MiB. «Similar» significa
una diferencia menor al 10% en estas medianas, sin afirmar equivalencia estadística.
Cada fila tiene su entrada exacta en el JSON. No se deben sumar speedups ni
contar ganadores como si los casos representaran la frecuencia de problemas ICPC.

| Escenario | Nuestro ms | KACTL ms | Nuestro MiB | KACTL MiB | Menor tiempo local |
|---|---:|---:|---:|---:|---|
| Fenwick suma | 29.374 | 32.042 | 27.00 | 26.98 | Similar |
| Fenwick lower_bound | 56.262 | 61.084 | 27.02 | 27.00 | Similar |
| Segment Tree suma | 85.448 | 94.792 | 29.47 | 28.53 | Similar |
| Sparse Table RMQ | 8.350 | 5.702 | 41.77 | 37.47 | KACTL ×1.46 |
| DSU | 5.740 | 5.697 | 10.97 | 10.98 | Similar |
| SCC: DAG | 15.503 | 0.937 | 19.47 | 13.33 | KACTL ×16.55 |
| SCC: componentes con ciclos | 14.554 | 2.436 | 19.98 | 13.34 | KACTL ×5.97 |
| Topológico | 3.119 | 3.275 | 13.06 | 12.77 | Similar |
| LCA binary lifting | 159.562 | 256.482 | 34.44 | 34.36 | Nuestro ×1.61 |
| LCA Euler/RMQ | 15.115 | 11.130 | 50.06 | 33.81 | KACTL ×1.36 |
| Dinic | 7.528 | 16.354 | 9.55 | 9.55 | Nuestro ×2.17 |
| Hopcroft–Karp | 8.261 | 5.603 | 9.55 | 9.55 | KACTL ×1.47 |
| Húngaro | 9.771 | 9.335 | 9.55 | 9.55 | Similar |
| Suffix Array + LCP: aleatorio | 87.749 | 17.507 | 9.55 | 14.16 | KACTL ×5.01 |
| Suffix Array + LCP: repetido | 138.680 | 44.139 | 9.55 | 14.14 | KACTL ×3.14 |
| Manacher | 24.890 | 20.747 | 51.33 | 28.39 | KACTL ×1.20 |
| KMP | 9.262 | 10.823 | 9.55 | 19.81 | Nuestro ×1.17 |
| Z-function | 10.151 | 10.391 | 16.92 | 16.92 | Similar |
| Aho: patrones aleatorios | 45.843 | 43.546 | 113.33 | 102.53 | Similar |
| Aho: muchos sufijos | 22.212 | 17.839 | 112.45 | 42.47 | KACTL ×1.25 |
| NTT: caso grande | 63.556 | 29.308 | 14.64 | 32.69 | KACTL ×2.17 |
| NTT: salida potencia de dos | 29.432 | 28.825 | 9.55 | 23.70 | Similar |
| LIS | 33.399 | 36.148 | 14.14 | 14.14 | Similar |
| Convex hull | 31.227 | 30.077 | 40.38 | 48.03 | Similar |
| Calipers | 0.449 | 0.245 | 9.55 | 9.55 | KACTL ×1.83 |
| Li Chao / LineContainer | 20.197 | 24.151 | 13.23 | 9.55 | Nuestro ×1.20 |
| Miller–Rabin: nuestro Montgomery | 0.607 | 1.564 | 9.55 | 9.55 | Nuestro ×2.58 |
| Miller–Rabin: nuestro simple | 2.135 | 1.604 | 9.55 | 9.55 | KACTL ×1.33 |
| Factorización: nuestro Montgomery | 4.486 | 7.526 | 9.55 | 9.55 | Nuestro ×1.68 |
| Factorización: nuestro simple | 18.774 | 7.548 | 9.55 | 9.55 | KACTL ×2.49 |
| Floyd: pesos positivos | 17.801 | 26.086 | 9.55 | 9.55 | Nuestro ×1.47 |
| Bellman–Ford: DAG | 0.770 | 12.869 | 9.55 | 9.55 | Nuestro ×16.71 |
| Cobertura mínima + matching | 9.026 | 1078.459 | 9.55 | 9.55 | Nuestro ×119.48 |
| Min-cost max-flow | 48.868 | 109.193 | 9.56 | 9.56 | Nuestro ×2.23 |

Algunas diferencias que explican los resultados:

- **SCC:** KACTL usa Tarjan en una pasada; nosotros Kosaraju, construyendo el
  grafo transpuesto y usando funciones recursivas `std::function`. Se midieron
  tanto un DAG como 2.000 componentes con ciclos. El estado de KACTL es global;
  el nuestro es local a la llamada. Ambos siguen siendo O(V+E).
- **Dinic:** KACTL aplica escalado de capacidad y guarda capacidad original para
  obtener flujo por arista. Nuestro Dinic es más simple y ganó en esta red con
  capacidades hasta 1.000. Otra distribución puede favorecer el escalado.
- **König:** la cobertura de KACTL llama internamente a DFSMatching, O(VE);
  nuestra ruta usa Hopcroft–Karp y luego el recorrido de cobertura. La gran
  diferencia es esa elección de matching y la posibilidad de reutilizarlo.
  KACTL también tiene HopcroftKarp: componerlo con su cobertura sería una
  variante distinta, no la versión original que se midió.
- **Manacher:** nuestro `return {odd, even}` copia los dos vectores locales;
  KACTL devuelve un objeto que puede aprovechar NRVO. Para tres millones de
  caracteres, esa copia explica aproximadamente 23 MiB adicionales de RSS.
  Evitarla es una posible mejora puntual, todavía sin aplicar ni medir como
  variante separada; no exige cambiar el algoritmo.
- **Aho con muchos sufijos:** nuestro resultado contiene pares `(fin,id)`;
  KACTL guarda listas de IDs por posición inicial. La diferencia de memoria
  incluye esa representación de millones de coincidencias, no sólo el autómata.
- **NTT:** KACTL reutiliza raíces precalculadas y es más rápido en el caso grande,
  pero usa coeficientes ll y buffers adicionales. Cuando el tamaño de salida
  es exactamente una potencia de dos, su código elige la potencia siguiente;
  ahí se reduce la ventaja y conserva un mayor consumo de memoria.
- **Bellman–Ford:** el nuestro corta cuando una ronda no cambia nada. KACTL
  ordena las aristas y ejecuta un número fijo de rondas y propagación de ciclos
  negativos. El caso medido es un DAG, favorable al corte temprano. KACTL
  marca los vértices afectados con −inf; el nuestro devuelve un indicador global.
- **Floyd:** el caso cronometrado sólo tiene pesos positivos. KACTL además
  propaga pares afectados por ciclos negativos y pasó el caso de 40 vértices
  con pesos −1 que hace desbordar nuestro código. El menor tiempo de nuestra
  versión no corrige esa diferencia de robustez.
- **Min-cost max-flow:** nuestro SPFA ganó en la red bipartita de 1.602
  vértices y 9.600 aristas medida. KACTL usa potenciales y una priority queue
  PBDS con decrease-key; tiene una cota por aumento mejor, pero más constantes.
  Se incluyó su inicialización `setpi` porque hay costos negativos. Este caso
  tampoco justifica reemplazar automáticamente nuestra versión por la otra.

## Cantidad de líneas de código

Son líneas físicas **activas**, sin comentarios, vacíos, includes ni ejemplos;
los helpers opcionales comentados no se cuentan como activos. No equivalen a
cantidad de operaciones: KACTL pone más expresiones por línea, usa macros y
nombres más cortos. La métrica de caracteres también está en el JSON.
Se muestran dependencias para no presentar como autocontenido un header corto.

| Implementación | Nuestro | KACTL |
|---|---:|---:|
| Fenwick | 34 | 21 |
| Segment Tree suma | 29 | 19 |
| Sparse Table RMQ | 21 | 16 |
| DSU | 17 | 14 |
| SCC | 24 | 23 |
| Topológico | 15 | 8 |
| LCA binary lifting | 53 | 23 + rooting externo |
| LCA Euler/RMQ | 47 | 18 + RMQ 16 = 34 |
| Dinic | 48 | 42 |
| Hopcroft–Karp | 49 | 20 |
| Húngaro | 44 | 24 |
| Suffix Array + LCP | 43 | 22 |
| Manacher | 20 | 13 |
| KMP (prefix + búsqueda) | 30 | 15 |
| Z-function | 10 | 12 |
| Aho–Corasick | 61 | 63 |
| NTT | 48 | 33 + ModPow 7 = 40 |
| LIS | 23 | 16 |
| Convex hull sin base Point | 30 | 13 |
| Calipers sin base Point | 38 | 12 |
| Li Chao / LineContainer | 52 | 28 |
| Miller–Rabin simple con producto/potencia | 36 | 12 + ModMul 11 = 23 |
| Factorización simple autocontenida | 86 | 18 + Miller 12 + ModMul 11 = 41 |
| Factorización Montgomery autocontenida | 141 | 41 |
| Floyd + chequeo de ciclo | 16 | 12 |
| Bellman–Ford | 32 | 21 |
| Cobertura + matching | 20 + HK 49 = 69 | 20 + DFSMatching 22 = 42 |
| Min-cost max-flow | 47 | 67 |

En nuestro LCA, padres/profundidades y método de distancia están dentro de la
clase. BinaryLifting de KACTL necesita padres/profundidades externos; el adaptador
de rooting usado en la prueba tiene siete líneas activas adicionales. Para
geometría, los helpers base compartidos suman 28 líneas en nuestro bloque y 26
en Point.h de KACTL; incluyen operaciones que no usa cada benchmark concreto.

## Comparación de explicaciones

| Aspecto | Nuestro libro | KACTL | Qué adoptaría |
|---|---|---|---|
| Reconocer el problema | `whenuse`, alternativas y ejemplos en español | Descripción breve de la operación y API | Mantener nuestro enfoque |
| Consulta rápida | Nombres descriptivos, código más extendido | Macros, nombres breves, contratos junto al header | Un resumen de contrato al inicio de cada sección |
| Evidencia de pruebas | Está sobre todo en tests/reportes del repositorio | `Status`, jueces concretos, autores y fuentes en muchos headers | Referencias a los tests y su alcance; no sólo «testeado» |
| Precondiciones | Más advertencias narrativas y ejemplos; algunas restricciones faltan | Contratos concisos como raíz, centinela o límites de aritmética | Hacer explícitas las condiciones que afectan corrección |
| Reconstrucción | Variantes opcionales y ejemplos completos en caminos y DP | Según tema, entrega índices, padres, extremos o flujo directamente | Conservar nuestras variantes y aclarar qué devuelve cada función |
| Memoria | Generalmente incluye la cota junto al tiempo | Varios headers indican tiempo sin memoria | Mantener la cota de memoria de nuestro libro |
| Densidad de impresión | Material más didáctico | Diseñado como referencia de 25 páginas | Elegir según aprendizaje o consulta durante el contest |

La comparación por tema tiene diferencias concretas:

| Tema | Ventaja de nuestra explicación | Ventaja o advertencia importante de KACTL |
|---|---|---|
| Fenwick | 1-indexado explícito, rangos y requisito no negativo para lower_bound | Prefijo [0,pos), resultado n si no existe y −1 para suma vacía; su salto inicial fijo requiere atención |
| SCC y 2-SAT | Explica el uso de SCC en dependencias y 2-SAT, con ejemplo de cláusulas | Explicita **orden topológico inverso** de sus IDs. Sustituir nuestro Kosaraju requiere adaptar la comparación de 2-SAT |
| LCA | Fórmula de distancia, jump y root configurable | Binary lifting declara que la raíz se apunta a sí misma; LCA RMQ sólo admite raíz 0 en esa interfaz |
| Suffix Array | Ejemplo banana, substrings distintos y uso de LCP | Explica el sufijo vacío extra, el índice de LCP y la prohibición de NUL. Nuestro texto de doubling podría explicar mejor sus invariantes |
| NTT | Primo, raíz, límite 2^23, normalización y ejemplo de conteo de pares | Explica la inversión con reverse/escalado y raíz precalculada, pero componer ModPow requiere configurar el módulo |
| Manacher | Describe radios impares incluyendo centro y pares | Radios impares excluyen centro; no pegar una fórmula de longitudes de una versión sobre la otra |
| KMP | Patrón vacío definido y búsqueda con memoria O(m) | Construye `pat + NUL + texto`, con memoria O(n+m). Para patrón vacío devuelve 3 posiciones en abc; el nuestro devuelve las 4 fronteras |
| Aho | IDs y posición final, construcción una vez, complejidad sensible a salida | Ofrece además el patrón más largo por posición y contadores; findAll agrupa por posición inicial |
| LIS | Explica estricta/no decreciente y devuelve valores reconstruidos | Devuelve **índices**, útil si se necesita identificar objetos originales; no es exactamente el mismo contrato |
| Geometría/calipers | i128, vacíos, colinealidad y ambas orientaciones | Más corto, pero exige hull CCW sin duplicados/colineales; devuelve extremos, no sólo distancia |
| Flujos | Modelado, alternativas y ejemplos de recuperación comentados | Flujo/min-cut disponibles directamente y metadatos de complejidad/validación junto a la plantilla |
| Pollard/Miller | Versiones corta y Montgomery con dominio completo u64 y correcciones enteras | Justifica varias optimizaciones, pero los límites de ModMulLL dependen del long double del juez |

El objetivo de KACTL es una referencia de competencia compacta, según su
[README](https://github.com/kth-competitive-programming/kactl/blob/b3548e973f8772010c1ef84e6d427c749733fd2c/README.md).
Una explicación más larga no implica un algoritmo más lento; los comentarios
y el texto LaTeX no se ejecutan. Tampoco un menor número de líneas garantiza
menor tiempo: Fenwick, Segment Tree y Bellman son ejemplos de esa diferencia.

## Qué propondría decidir, sin aplicarlo

1. **Evaluar Suffix Array de KACTL y Tarjan:** diferencias grandes en los casos
   medidos. En SA adaptar la interfaz del centinela/LCP; en SCC revisar todos
   los consumidores de sus IDs, especialmente 2-SAT.
2. **Considerar su LCA RMQ/RMQ compacto** si consultas o memoria lo justifican.
   No cambiar binary lifting automáticamente: nuestro constructor resulta
   cómodo y las mediciones no muestran una ventaja de KACTL allí.
3. **NTT como alternativa de velocidad**, conservando nuestra versión de menor
   memoria. Medir una adaptación con la misma API antes de decidir reemplazo.
4. **Mantener nuestro Dinic, KMP, DSU y cobertura con Hopcroft–Karp** para los
   casos medidos; el código más corto del otro equipo no es motivo suficiente.
5. **Atender primero Floyd y la copia de Manacher:** son correcciones/mejoras
   puntuales identificables; no hace falta migrar toda una biblioteca.
6. **Conservar reconocimiento, ejemplos y memoria de nuestras explicaciones**;
   incorporar contratos breves, fuentes y referencias verificables a pruebas.

No se hizo ranking de velocidad para cada archivo del libro. Dijkstra, por
ejemplo, se excluye expresamente de KACTL por ser básico. Lazy Segment Tree,
HLD y Treap tienen interfaces/prestaciones/allocadores diferentes y no se les
atribuye un speedup sin un ensayo equivalente. La tabla contiene sólo los
escenarios efectivamente ejecutados y descritos.

## Reproducción

```sh
git clone https://github.com/kth-competitive-programming/kactl.git /tmp/kactl
git -C /tmp/kactl checkout b3548e973f8772010c1ef84e6d427c749733fd2c

python3 tests/run_kactl_stress.py /tmp/kactl --compiler g++ \
  --json-output /tmp/kactl-stress.json

python3 tests/compare_kactl.py /tmp/kactl --compiler g++ \
  --json-output /tmp/kactl-comparison.json
```

En esta Mac se usó `g++-15` y los flags extra del SDK guardados en los JSON.
En GNU/Linux normalmente no hacen falta. El runner de stress registra el
límite real de pila; para reproducir el runner oficial en Linux puede ampliarse
antes con `ulimit -s 524288`. Eso no se pudo hacer en la Mac actual.

Los scripts y resultados pueden guardarse en Git; este `.md` permanece local.

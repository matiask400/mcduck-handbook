# Módulos agregados: corrección, tiempo y memoria

Medición del 26 de octubre de 2026. Los algoritmos se extraen de los `.tex`,
no de implementaciones alternativas copiadas en los tests.

## Método

- Máquina local ARM64 con macOS; GNU G++ 15.2, C++17, `-O2`.
- Mediana de tres procesos independientes para tiempo de pared; máximo de sus
  picos RSS para memoria. El tiempo incluye inicio del ejecutable, generación de
  la entrada sintética, algoritmo y verificaciones del benchmark. No incluye
  compilación ni reproduce lectura e impresión de una entrada completa del juez.
- La memoria es RSS del proceso completo, no solamente la tabla del algoritmo.
  Incluye runtime y efectos del allocator; por eso puede superar la cuenta de
  bytes teórica de los arreglos.
- También pasaron corrección con Clang y AddressSanitizer/UndefinedBehaviorSanitizer.
- Los presupuestos de 1/2 s y 256 MiB son escenarios de prueba, no límites
  universales de ICPC. Para FootXOR se contrastaron los límites de su mirror:
  0,5 s y 1024 MB. En Palindromic Distance se verificó el segundo de tiempo y
  la suma de longitudes de 3000; los 256 MiB siguen siendo un presupuesto local.
- **“Entra localmente” no significa Accepted:** no se enviaron soluciones al juez
  ni se dispone de sus casos ocultos. Estos escenarios tampoco demuestran el
  peor tiempo posible para cada algoritmo; se combinan con las cotas documentadas.

## Resultados con GNU G++

| Algoritmo / entrada | Tiempo (ms) | Pico RSS (MiB) | Presupuesto | Resultado local |
|---|---:|---:|---|---|
| LCS, 2000 × 2000 | 13,99 | 18,28 | 2 s / 256 MiB | Entra |
| LCS, 5000 × 5000 | 32,81 | 107,30 | 2 s / 256 MiB | Entra |
| LCS, 10000 × 10000 | 112,68 | 473,50 | 2 s / 256 MiB | **No entra por memoria** |
| Edición entre dos cadenas de 3000 | 46,33 | 40,42 | 1 s / 256 MiB | Entra |
| Palíndromo más cercano, longitud 3000 | 29,20 | 40,28 | 1 s / 256 MiB | Entra |
| Mochila ilimitada, 1000 tipos, W=100000 | 61,80 | 9,55 | 2 s / 256 MiB | Entra |
| Suma exacta, 100 objetos, W=100000 | 14,46 | 9,55 | 2 s / 256 MiB | Entra |
| Dos tablas para Nine Packs, 100 objetos por lado, W=100000 | 20,07 | 9,55 | 2 s / 256 MiB | Entra |
| Subset sum bitset, 1000 objetos, un millón de sumas | 20,60 | 9,55 | 2 s / 256 MiB | Entra |
| Subset sum bitset, 100000 pesos iguales a 1, un millón de sumas | 1223,79 | 9,55 | 2 s / 256 MiB | Entra, con menos margen |
| MITM, 40 elementos, búsqueda sin solución | 205,26 | 34,56 | 2 s / 256 MiB | Entra |
| Diámetro, cadena de 200000 nodos | 19,94 | 17,41 | 1 s / 256 MiB | Entra; sin recursión |
| Matching + König, 100000/50000 nodos y 300000 aristas | 17,48 | 9,55 | 2 s / 256 MiB | Entra |
| Digit DP, 10000 consultas cerca de 10^18 | 50,89 | 9,55 | 2 s / 256 MiB | Entra para este estado |
| Base XOR escalar, un millón de inserciones | 177,53 | 9,55 | 2 s / 256 MiB | Entra |
| Dependencia GF(2), 1500 vectores de 1501 bits | 17,68 | 9,55 | 0,5 s / 1024 MB | Entra localmente |

El benchmark GF(2) genera vectores independientes mediante operaciones de filas
que conservan el bit de paridad, y coloca una dependencia al final. Se verifica
que los índices recuperados tengan cardinalidad par y XOR cero. Los cuatro
ejemplos publicados de Brazilian FootXOR también se ejecutan mediante el
adaptador impreso en el libro y se valida la salida permitiendo distintas respuestas.

## Qué se validó por separado

- 300 pares de cadenas: LCS frente a enumeración de subsecuencias; edición frente
  a una DP de filas independiente. Se aplican todas las operaciones recuperadas y
  se exige que la cadena final sea el objetivo.
- 100 palabras cortas: palíndromo más cercano frente a enumeración de palíndromos.
- 400 mochilas y bitsets: óptimos frente a DP alternativa y enumeración de
  subconjuntos; peso cero en suma exacta, soluciones imposibles y objetivo cero.
- 180 MITM frente a enumeración completa, incluyendo negativos y extremos de `ll`.
- 400 árboles: diámetro frente a distancias desde todos los nodos y validación
  del camino; también pesos cero y una cadena larga sin DFS recursivo.
- 300 grafos bipartitos: cobertura recuperada frente a enumeración de coberturas.
- Digit DP frente a enumeración hasta 100000, conteos cerrados por longitud y
  extremos cercanos a `LLONG_MAX`.
- 300 bases XOR frente a todos los subconjuntos: pertenencia, máximo, rango y
  reconstrucción de dependencias pares; también el bit 63 y los casos sin solución.

## Correspondencia con los problemas de referencia

| Referencia | Alcance de la plantilla |
|---|---|
| Brazilian FootXOR, LAC 2025 B | Se incluye adaptador completo de entrada/salida para n,k≤1500, bit de paridad y recuperación de equipos. Pasa los cuatro ejemplos publicados; falta verificación en el juez. |
| Palindromic Distance, Taiwan Online 2025 D | La función calcula exactamente el costo pedido, además de un palíndromo óptimo. Se probó longitud 3000; el enunciado limita la suma a 3000. |
| Nine Packs, NAQ 2016 H | Se pueden recuperar todas las sumas en una pasada por lado con `all_costs`. Falta el pequeño adaptador de lectura/impresión; no repetir la DP por cada suma. |
| Adjoin the Networks, NCPC 2015 A | La función resuelve un árbol conectado. El problema completo exige separar componentes, obtener sus centros/radios y combinar los tres radios mayores. |
| Jewelry Exhibition, GCPC 2014 G | König recupera la cobertura del grafo ya modelado. Faltan la lectura y el mapeo geométrico a filas/columnas. |
| Many LCS, SEERC 2021 D | **No queda resuelto**: pide construir cadenas con exactamente K LCS distintas, no hallar una LCS de dos cadenas dadas. |
| Score Values, WF 2025 H | **No queda resuelto**: la plantilla muestra ceros iniciales y `tight`, pero ese problema necesita residuos, máximos de frecuencias y alcanzabilidad de puntajes. |
| Jigsaw Present, GCPC 2024 J | MITM recupera una suma con hasta 40 elementos. La entrada del problema necesita una reducción adicional; no se puede enumerar directamente miles de elementos. |

Fuentes: [LAC 2025 B](https://codeforces.com/gym/105789/problem/B),
[Taiwan 2025](https://topc.icpc.tw/topc2025-problemset.pdf),
[NAQ 2016](https://cs.baylor.edu/~hamerly/icpc/qualifier_2016/naq-2016-explanations.pdf),
[NCPC 2015](https://icpcarchive.github.io/Europe%20Subcontests/Nordic%20Collegiate%20Programming%20Contest%20%28NCPC%29/2015%20Nordic%20Collegiate%20Programming%20Contest/solution.pdf),
[GCPC 2014](https://2014.gcpc.nwerc.eu/problemset_2014.pdf),
[SEERC 2021](https://seerc.icpc.global/2021/prob/problems-2021-2022.pdf),
[WF 2025](https://icpc.global/worldfinals/problems/2025-ICPC-World-Finals/2025-ICPC-Solutions.pdf),
[GCPC 2024](https://2024.gcpc.nwerc.eu/solutions.pdf).

## Líneas agregadas al libro

Se cuentan líneas de los bloques C++, incluidos comentarios, blancos y ejemplos;
no se cuentan explicaciones LaTeX ni los archivos de pruebas.

| Archivo | Líneas C++ |
|---|---:|
| `dp/string_dp.tex` | 75 |
| `dp/knapsack_variants.tex` | 63 |
| `dp/digit_dp.tex` | 26 |
| `techniques/meet_in_the_middle.tex` | 34 |
| `graphs/tree_diameter.tex` | 37 |
| `graphs/konig_vertex_cover.tex` | 27 |
| `math/xor_basis.tex` | 73 |
| **Total** | **335** |

## Reproducir

En GNU/Linux con G++ disponible:

```sh
python3 tests/validate_new_modules.py --compiler g++ --benchmark --repeat 3 --json-output /tmp/new-modules.json
```

En esta Mac se usó G++ de Homebrew con el SDK instalado:

```sh
python3 tests/validate_new_modules.py --compiler g++-15 --cxxflag=-isysroot --cxxflag=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk --cxxflag=-idirafter --cxxflag=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include --benchmark --repeat 3
```

Corrección con sanitizadores, sin los benchmarks de memoria grande:

```sh
python3 tests/validate_new_modules.py --cxxflag=-fsanitize=address,undefined --cxxflag=-fno-sanitize-recover=all
```

Los resultados completos de ambas compilaciones se guardan en
`new_modules_benchmarks_gcc.json` y `new_modules_benchmarks_clang.json`.

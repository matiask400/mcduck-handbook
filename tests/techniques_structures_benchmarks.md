# Pruebas y benchmarks de técnicas y estructuras

Mediciones locales en macOS ARM64, compilación C++17 con `-O2`.
No equivalen a un AC ni garantizan los mismos tiempos en un juez ICPC.

## Búsqueda binaria y Segment Tree

Mediana de cinco ejecuciones con Apple Clang 21; comparación con el commit
`668592c`, anterior a esta publicación. Los tiempos incluyen el lote completo.

| Caso | Antes | Ahora |
|---|---:|---:|
| Búsqueda binaria real: 100000 consultas | 3.983 ms | 4.774 ms |
| Segment Tree: 200000 updates/consultas | 6.068 ms | 5.707 ms |

La binaria conserva tolerancia absoluta `1e-6`: para sqrt(2), ambas versiones
hacen 21 chequeos y dan error aproximado 5.718e-7 respecto de `sqrt`.
La nueva corta si el punto medio coincide con un extremo para evitar bloqueos.
Ese chequeo tiene un costo adicional; con magnitudes grandes, la precisión de
`double` puede impedir alcanzar la tolerancia pedida.

Segment Tree conserva sus complejidades: build O(n), update/query O(log n).
Explicitar `combine` y `NEUTRAL` facilita la adaptación. La pequeña diferencia
de tiempo medida no demuestra una mejora; puede ser variación del benchmark.

Líneas de los bloques C++, incluidos blancos/comentarios:
- Binaria: 26 -> 28.
- Segment Tree: 31 -> 33.
- MITM: 34 (módulo nuevo).

Pruebas: binaria entera/real y límites de precisión; 20000 operaciones de
Segment Tree frente a recorrido directo, con suma, mínimo, máximo, GCD y XOR.
El harness compartido también comprueba Aho y los grafos ya publicados.

## Meet-in-the-middle

180 casos pequeños frente a todos los subconjuntos, recuperación de índices,
negativos y extremos de long long. Todas las sumas de subconjuntos deben entrar
en long long; el test de extremos respeta esa condición.

Benchmark n=40: pesos pares aleatorios y objetivo impar, de modo que se
recorren las listas sin encontrar solución. Tiempo de pared mediano de tres
procesos; RSS es el máximo de esas ejecuciones, sin incluir el compilador.

| Compilador | Tiempo de pared | Pico RSS |
|---|---:|---:|
| clang++ | 188.889 ms | 33.594 MiB |
| g++-15 | 224.910 ms | 34.469 MiB |

Los JSON adyacentes conservan las muestras y opciones de compilación.
Las dos listas de hasta 2^20 pares (suma,máscara) ocupan alrededor de 32 MiB;
RSS incluye además el resto del proceso. No escalar esta plantilla a n=50.

## Reproducir

```sh
python3 tests/validate_complexity_examples.py --baseline-ref 668592c
python3 tests/validate_mitm.py --benchmark
python3 tests/validate_mitm.py --compiler g++ --benchmark
```

En esta Mac, Homebrew GCC necesitó las opciones del SDK registradas en
`mitm_benchmarks_gcc.json`. Las pruebas normales también permiten sanitizadores:

```sh
python3 tests/validate_mitm.py --cxxflag=-fsanitize=address,undefined --cxxflag=-fno-sanitize-recover=all
```

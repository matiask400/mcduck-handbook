# Cambios de la reorganización

- Se reemplazó la estructura original por capítulos temáticos y nombres sin espacios.
- Se corrigieron Dijkstra, LCA, búsqueda binaria, Floyd–Warshall, cribas, Segment Tree y Pollard–Rho.
- Se integraron DSU, Fenwick, Lazy Segment Tree y Disjoint Sparse Table validados.
- Se agregaron topological sort, SCC, 0–1 BFS, Bellman–Ford, Prim, puentes, articulaciones y flujos.
- Se agregó Ford–Fulkerson con DFS sobre el grafo residual y pruebas contra Dinic.
- Se agregaron KMP, Z-function, trie, aritmética modular, CRT, combinatoria, matrices y geometría 2D.
- Se unificaron tipos, convenciones de rangos, complejidades y casos límite.
- Se amplió la base de C++17 con sintaxis, referencias, lambdas, contenedores,
  algoritmos STL, comparadores, operaciones de bits, aleatoriedad y errores comunes.

## Ampliación basada en El Vasito

- Se agregaron 2-SAT, camino euleriano, Hopcroft–Karp y algoritmo Húngaro.
- Se agregaron Heavy-Light Decomposition y Segment Tree persistente.
- Se agregaron Manacher, Aho–Corasick, Suffix Array y LCP.
- Se agregaron Mo's Algorithm y Divide and Conquer DP Optimization.
- Se agregó NTT para convolución exacta módulo 998244353.
- Se amplió geometría con rectas, proyecciones, círculos, punto en polígono y rotating calipers.
- Se incorporaron pruebas deterministas y aleatorias contra implementaciones directas o equivalentes.

## Ampliaci\'on basada en TAP 2023--2025

- Se agregaron BFS multiorigen, Virtual Tree, LCA con Euler tour + RMQ,
  Centroid Decomposition y rerooting DP.
- Se agregaron componentes biconexas por v\'ertices.
- Se agregaron PBDS, Fenwick 2D offline, Wavelet Tree, Treap impl\'icito,
  Li Chao Tree e intervalos disjuntos.
- Se document\'o la separaci\'on por ra\'iz cuadrada entre elementos frecuentes
  y poco frecuentes.
- Se agregaron Lucas, eliminaci\'on gaussiana, determinante y geometr\'ia 3D.
- Se agregaron una DP con cota triangular $O(n\sqrt n)$ y la receta para caminos
  que rodean un pol\'igono convexo.
- Cada incorporaci\'on incluye criterio de uso, tiempo, memoria y advertencias.
- Se ampliaron las pruebas para cubrir LCA RMQ, rerooting, Virtual Tree, Lucas,
  Fenwick 2D, Wavelet Tree, Li Chao y eliminaci\'on gaussiana.

## Base y teor\'ia de competencia

- Se incorpor\'o el template cl\'asico con `pb`, `fst`, `snd`, `fore`, `SZ`,
  `ALL`, `mset`, `FIN`, `ll` e `ii`.
- Se agreg\'o una tabla de includes est\'andar para reemplazar `bits/stdc++.h`
  cuando el compilador no sea GNU C++.
- Se agreg\'o un template Python 3 con entrada r\'apida, deque, heap, bisect,
  memoizaci\'on, `isqrt` y potencia modular.
- Se documentaron `sqrt`, ra\'iz entera segura, potencias, logaritmos,
  redondeos, divisiones piso/techo con negativos y comparaci\'on flotante.
- Se agreg\'o una hoja de teoremas e identidades: palomar, handshake, cota
  triangular, serie arm\'onica, inclusi\'on--exclusi\'on, stars and bars,
  Vandermonde, Catalan, Burnside, B\'ezout, Fermat, Euler, Wilson, Euler planar
  y Pick.
- El validador ahora comprueba tambi\'en la sintaxis del template Python.

## Compilaci\'on r\'apida en Overleaf

- Se elimin\'o el lexer de C++/Python, coloreado sem\'antico, n\'umeros de l\'inea
  y marcos de los m\'as de 100 bloques `listings`, conservando el texto completo.
- Se quitaron `inconsolata`, `tocloft`, `graphicx` e `hyperref`, que no eran
  necesarios y aumentaban el tiempo de arranque de pdfLaTeX.
- Se cambi\'o la opci\'on de fuente no soportada de 9 pt a 10 pt.
- El template Python sigue valid\'andose como Python aunque el PDF no cargue el
  lexer de lenguaje de `listings`.

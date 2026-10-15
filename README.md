# McDuck ICPC Handbook

Handbook compacto de programación competitiva en C++17.

## Compilación en Overleaf

- Archivo principal: `main.tex`.
- Compilador recomendado: pdfLaTeX.
- No se requieren comandos externos ni `shell-escape`.
- El estilo de `listings` es deliberadamente liviano: conserva todo el código
  pero omite lexer, colores, marcos y números de línea para entrar en el límite
  de compilación del plan gratuito de Overleaf.
- Si Overleaf conserva una compilación anterior, usar **Recompile from scratch**
  una vez después de reemplazar los archivos.

## Convenciones

- Vértices e índices internos: 0-indexados, salvo Fenwick (1-indexado).
- Rangos: `[l,r)` salvo que el texto indique explícitamente que son inclusivos.
- `INF`/`LINF`, `ll`, `i128` y `u128` se definen en el template base.
- Todos los algoritmos indican precondiciones y complejidad.
- Los módulos avanzados están inspirados en la cobertura temática del notebook
  de referencia “El Vasito”, con interfaces y validaciones propias de McDuck.

Los snippets se validan con `tests/validate_snippets.py` usando GNU++17.

## Selecci\'on r\'apida de los m\'odulos nuevos

- Consultas sobre pocos nodos marcados de un \`arbol: Virtual Tree.
- Respuesta para todas las ra\'ices: rerooting DP.
- Distancia al origen m\'as cercano: BFS multiorigen.
- Frecuencias muy desbalanceadas: separaci\'on frecuentes/poco frecuentes.
- Consultas 2D con coordenadas conocidas: Fenwick 2D offline.
- K-\'esimo en subarreglo est\'atico: Wavelet Tree.
- Secuencia editable: Treap impl\'icito.
- DP de rectas: CHT/Li Chao.
- Combinaciones con primo peque\~no y `n` enorme: Lucas.
- Sistemas lineales: eliminaci\'on gaussiana.

Cada m\'odulo indica cu\'ando usarlo, tiempo, memoria y precondiciones importantes.

La base incluye dos estilos de arranque: template C++17 con las macros hist\'oricas
del equipo y template Python 3. Tambi\'en contiene una referencia de headers
portables para reemplazar `bits/stdc++.h`, matem\'atica cotidiana de C++
(`sqrt`, `isqrt`, divisiones con signo y comparaci\'on flotante) y una hoja de
teoremas para reconocer soluciones.

**Facultad de Ciencias Exactas, Físicas Y Naturales - Universidad Nacional de Córdoba**

Departamento de Computación - Cátedra de Sistemas Operativos I

Trabajo Práctico Nº 3
# SOI-Memory-management

## Objetivos
Con el desarrollo del siguiente Trabajo Práctico (TP), se busca:
- Memory Allocation and Management
- Performance Optimization
- Analisis dinámico y profiling

## Introducción
Este trabajo práctico consta en la elaboración de un programa en lenguaje **C** sobre _GNU/Linux_. El trabajo se divide en soluciones incrementales.

### Memory allocation
Se debe implementar una librería con las funciones:
```C
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
```
También se debe implentar la función 
```C
void check_heap();
```
que debe imprimir el estado del heap. Esta función les ayudará a _debugguear_ el programa.

### Explicit free list
A fines de evitar fragmentación de memoria y para mayor rendimiento, se debe implementar una lista doblemente enlazada para la gestión de memoria libre.

### Memory Allocation Policy
Se debe implementar el comando _malloc_control_, que permita seleccionar el algoritmo de alocación de memoria:
- best fit
- first fit    

### Memory Map
Mejorar el comando _malloc_ a partir del uso de la syscall _mmap_. Justificar su utilización.  Por qué no se utiliza siempre el comando _mmap_?

### Valgrind (opcional)
Se debe utilizar el conjunto de herramientas provistas por Valgrind para realizar un análisis dinámico y profiling del trabajo. Documentarlo.

### Verificación
Se deberá presentar un plan de verificación. Se debe incluir un archivo main.c para la verificación de la librería.

## Criterios de Corrección
- Se debe compilar el código con los flags de compilación: Wall -Pedantic -Werror -Wextra -Wconversion -std=gnu11
- Dividir el código en módulos de manera juiciosa.
- Estilo de código.
- El código no debe contener errores, ni warnings.
- El código no debe contener errores de cppcheck.

### Qué se debe Entregar
- Informe del desarrollo del proyecto.
- Código (funcionando bajo las especificaciones dadas y bajo cualquier caso de test de parámetros de entrada).
- Makefile

## Recursos
- [malloc(3): allocate/free dynamic memory - Linux man page](https://linux.die.net/man/3/malloc "man malloc(3)")
- The Linux Programming Interface. Chap 7. Memory Allocation
- pdfs shown in class
- [Valgrind QSG](https://valgrind.org/docs/manual/quick-start.html)


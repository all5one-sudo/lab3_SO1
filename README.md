# Trabajo Practico 3: Sistemas Operativos 1

## Introduccion
En este archivo se ira documentando el desarrollo del tercer trabajo practico de la materia Sistemas Operativos I. Este repositorio corresponde al alumno Villar Federico Ignacio, de la carrera Ingeniera en Computacion e Ingenieria Electronica. Por separado se mostraran las consignas, implementaciones y resultados obtenidos.

## Herramientas utilizadas
Para el desarrollo del laboratorio, se utilizaron las siguientes herramientas:
- Ubuntu 22.04 LTS
- Visual Studio Code
- Obsidian

## Consigna
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

## Desarrollo
### Memory allocation
Para poder implementar la libreria solicitada, se creo un archivo `memory.h`, que define de forma general los prototipos a utilizar  en `memory.c`. El codigo de este ultimo es:
```c
#include "memory.h"

typedef struct s_block *t_block;
void *base = NULL;
int method = 0;

t_block find_block(t_block *last, size_t size)
{
    t_block b = base;
    while (b && !(b->free && b->size >= size))
    {
        *last = b;
        b = b->next;
    }
    return (b);
}

t_block extend_heap(t_block last, size_t s)
{
    t_block b;
    b = sbrk(0);
    if (sbrk(BLOCK_SIZE + s) == (void *)-1)
        return (NULL);
    b->size = s;
    b->next = NULL;
    if (last)
        last->next = b;
    b->free = 0;
    return (b);
}

void split_block(t_block b, size_t s)
{
    t_block new;
    new = b->data + s;
    new->size = b->size - s - BLOCK_SIZE;
    new->next = b->next;
    new->free = 1;
    b->size = s;
    b->next = new;
}

void copy_block(t_block src, t_block dst)
{
    int *sdata, *ddata;
    size_t i;
    sdata = src->ptr;
    ddata = dst->ptr;
    for (i = 0; (i * 4) < src->size && (i * 4) < dst->size; i++)
        ddata[i] = sdata[i];
}

t_block get_block(void *p)
{
    char *tmp;
    tmp = p;
    return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void *p)
{
    if (base)
    {
        if (p > base && p < sbrk(0))
        {
            return (p == (get_block(p))->ptr);
        }
    }
    return (0);
}

t_block fusion(t_block b)
{
    if (b->next && b->next->free)
    {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next)
            b->next->prev = b;
    }
    return (b);
}

void get_method(int m)
{
    method = m;
}

void set_method(int m)
{
    method = m;
}

void *malloc(size_t size)
{
    t_block b, last;
    size_t s;
    s = align4(size);
    if (base)
    {
        last = base;
        b = find_block(&last, s);
        if (b)
        {
            if ((b->size - s) >= (BLOCK_SIZE + 4))
                split_block(b, s);
            b->free = 0;
        }
        else
        {
            b = extend_heap(last, s);
            if (!b)
                return (NULL);
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
            return (NULL);
        base = b;
    }
    return (b->data);
}

void free(void *ptr)
{
    t_block b;
    if (valid_addr(ptr))
    {
        b = get_block(ptr);
        b->free = 1;
        if (b->next && b->next->free)
            fusion(b);
        if (b->prev && b->prev->free)
            fusion(b->prev);
        else
        {
            if (b->next)
                b->next->prev = b;
            if (b->prev)
                b->prev->next = b;
            else
                base = b;
            b->free = 1;
            b->prev = NULL;
        }
    }
}

void *calloc(size_t number, size_t size)
{
    size_t *new;
    size_t s4, i;
    new = malloc(number * size);
    if (new)
    {
        s4 = align4(number * size) << 2;
        for (i = 0; i < s4; i++)
            new[i] = 0;
    }
    return (new);
}

void *realloc(void *ptr, size_t size)
{
    size_t s;
    t_block b, new;
    void *newp;
    if (!ptr)
        return (malloc(size));
    if (valid_addr(ptr))
    {
        s = align4(size);
        b = get_block(ptr);
        if (b->size >= s)
        {
            if (b->size - s >= (BLOCK_SIZE + 4))
                split_block(b, s);
        }
        else
        {
            if (b->next && b->next->free && (b->size + BLOCK_SIZE + b->next->size) >= s)
            {
                fusion(b);
                if (b->size - s >= (BLOCK_SIZE + 4))
                    split_block(b, s);
            }
            else
            {
                newp = malloc(s);
                if (!newp)
                    return (NULL);
                new = get_block(newp);
                copy_block(b, new);
                free(ptr);
                return (newp);
            }
        }
        return (ptr);
    }
    return (NULL);
}

void check_heap()
{
    t_block b;
    b = base;
    while (b)
    {
        if (b->free)
        {
            if (b->next && b->next->free)
            {
                printf("Error: two consecutive free blocks\n");
                exit(1);
            }
        }
        b = b->next;
    }
}
```

Las funciones utilizadas fueron:
- `typedef struct s_block *t_block; void *base = NULL; int method = 0;`: Aquí estás definiendo un tipo de puntero a tu estructura `s_block` y también estás definiendo dos variables globales, `base` y `method`.
- `find_block(t_block *last, size_t size)`: Esta función busca un bloque de memoria libre que sea suficientemente grande para contener `size` bytes. Devuelve un puntero al bloque si lo encuentra, o `NULL` si no.
- `extend_heap(t_block last, size_t s)`: Esta función extiende el heap en `s` bytes. Devuelve un puntero al nuevo bloque de memoria.
- `split_block(t_block b, size_t s)`: Esta función divide un bloque de memoria en dos, con el primer bloque siendo de tamaño `s`.
- `copy_block(t_block src, t_block dst)`: Esta función copia los datos del bloque `src` al bloque `dst`.
- `get_block(void *p)`: Esta función devuelve el bloque de memoria que contiene el puntero `p`.
- `valid_addr(void *p)`: Esta función comprueba si el puntero `p` apunta a un bloque de memoria válido.
- `fusion(t_block b)`: Esta función fusiona dos bloques de memoria adyacentes en uno solo.
- `get_method(int m)` y `set_method(int m)`: Estas funciones obtienen y establecen el método de asignación de memoria, respectivamente.
- `malloc(size_t size)`: Esta función asigna un bloque de memoria de tamaño `size`.
- `free(void *ptr)`: Esta función libera el bloque de memoria al que apunta `ptr`.
- `calloc(size_t number, size_t size)`: Esta función asigna un bloque de memoria suficientemente grande para contener `number` elementos de tamaño `size`, y luego inicializa ese bloque a cero.
- `realloc(void *ptr, size_t size)`: Esta función cambia el tamaño del bloque de memoria al que apunta `ptr` al tamaño `size`.
- `check_heap()`: Esta función comprueba el estado del heap y muestra un error si encuentra dos bloques libres consecutivos.

### Explicit free list
Para la implementacion de la doble lista enlazada, se modificaron levemente las implementaciones de las funciones, se agrega a `s_block` dos punteros nuevos a `*prev` y `*next`, con el fin de realizar chequeos y cumplir con la consigna. Se usa el siguiente codigo:
```c
#include "memory.h"

typedef struct s_block *t_block;
void *base = NULL;
int method = 0;

t_block find_block(t_block *last, size_t size)
{
    t_block b = base;
    while (b && !(b->free && b->size >= size))
    {
        *last = b;
        b = b->next;
    }
    return (b);
}

t_block extend_heap(t_block last, size_t s)
{
    t_block b;
    b = sbrk(0);
    if (sbrk(BLOCK_SIZE + s) == (void *)-1)
        return (NULL);
    b->size = s;
    b->next = NULL;
    if (last)
        last->next = b;
    b->free = 0;
    return (b);
}

void split_block(t_block b, size_t s)
{
    t_block new;
    new = b->data + s;
    new->size = b->size - s - BLOCK_SIZE;
    new->next = b->next;
    new->free = 1;
    b->size = s;
    b->next = new;
}

void copy_block(t_block src, t_block dst)
{
    int *sdata, *ddata;
    size_t i;
    sdata = src->ptr;
    ddata = dst->ptr;
    for (i = 0; (i * 4) < src->size && (i * 4) < dst->size; i++)
        ddata[i] = sdata[i];
}

t_block get_block(void *p)
{
    char *tmp;
    tmp = p;
    return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void *p)
{
    if (base)
    {
        if (p > base && p < sbrk(0))
        {
            return (p == (get_block(p))->ptr);
        }
    }
    return (0);
}

t_block fusion(t_block b)
{
    if (b->next && b->next->free)
    {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next)
            b->next->prev = b;
    }
    return b;
}

void get_method(int m)
{
    method = m;
}

void set_method(int m)
{
    method = m;
}

void *malloc(size_t size)
{
    t_block b, last;
    size_t s;
    s = align4(size);
    if (base)
    {
        last = base;
        b = find_block(&last, s);
        if (b)
        {
            if ((b->size - s) >= (BLOCK_SIZE + 4))
                split_block(b, s);
            b->free = 0;
        }
        else
        {
            b = extend_heap(last, s);
            if (!b)
                return (NULL);
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
            return (NULL);
        base = b;
    }
    return (b->data);
}

void free(void *ptr)
{
    t_block b;
    if (valid_addr(ptr))
    {
        b = get_block(ptr);
        b->free = 1;
        if (b->next && b->next->free)
            fusion(b);
        if (b->prev && b->prev->free)
            fusion(b->prev);
        else
        {
            if (b->next)
                b->next->prev = b;
            if (b->prev)
                b->prev->next = b;
            else
                base = b;
            b->free = 1;
            b->prev = NULL;
        }
    }
}

void *calloc(size_t number, size_t size)
{
    size_t *new;
    size_t s4, i;
    new = malloc(number * size);
    if (new)
    {
        s4 = align4(number * size) << 2;
        for (i = 0; i < s4; i++)
            new[i] = 0;
    }
    return (new);
}

void *realloc(void *ptr, size_t size)
{
    size_t s;
    t_block b, new;
    void *newp;
    if (!ptr)
        return (malloc(size));
    if (valid_addr(ptr))
    {
        s = align4(size);
        b = get_block(ptr);
        if (b->size >= s)
        {
            if (b->size - s >= (BLOCK_SIZE + 4))
                split_block(b, s);
        }
        else
        {
            if (b->next && b->next->free && (b->size + BLOCK_SIZE + b->next->size) >= s)
            {
                fusion(b);
                if (b->size - s >= (BLOCK_SIZE + 4))
                    split_block(b, s);
            }
            else
            {
                newp = malloc(s);
                if (!newp)
                    return (NULL);
                new = get_block(newp);
                copy_block(b, new);
                free(ptr);
                return (newp);
            }
        }
        return (ptr);
    }
    return (NULL);
}

void check_heap()
{
    t_block b;
    b = base;
    while (b)
    {
        if (b->free)
        {
            if (b->next && b->next->free)
            {
                printf("Error: two consecutive free blocks\n");
                exit(1);
            }
        }
        b = b->next;
    }
}
```

### Memory allocation policy
Para poder implementar un algoritmo best fit o un first fit, se crea una funcion llamada `malloc_control()` que modifica la variable `method`, de modo de plantear una u otra forma de ejecucion en la busqueda de bloques. La funcion modificada es:
```c
t_block find_block(t_block *last, size_t size)
{
    t_block b = base;
    if (method == FIRST_FIT)
    {
        while (b && !(b->free && b->size >= size))
        {
            *last = b;
            b = b->next;
        }
        return (b);
    }
    else
    {
        size_t dif = PAGESIZE;
        t_block best = NULL;
        while (b)
        {
            if (b->free)
            {
                if (b->size == size)
                {
                    return b;
                }
                if (b->size > size && (b->size - size) < dif)
                {
                    dif = b->size - size;
                    best = b;
                }
            }
            *last = b;
            b = b->next;
        }
        return best;
    }
}
```
Y se agrega:
```c
void malloc_control(int m)
{
    if (m == 0)
    {
        set_method(0);
    }
    else if (m == 1)
    {
        set_method(1);
    }
    else
    {
        printf("Error: invalid method\n");
    }
}
```

### Memory map
Para mejorar `malloc` con `mmap`, se reemplaza `stbrk` por `mmap` en `extend_heap()`.  
```c
t_block extend_heap(t_block last, size_t s)
{
    t_block b;
    b = mmap(0, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (b == MAP_FAILED) {
        return NULL;
    }
    b->size = s;
    b->next = NULL;
    b->prev = last;
    b->ptr = b->data;
    if (last)
        last->next = b;
    b->free = 0;
    return b;
}
```
GitHub Copilot: La función `mmap` se utiliza para mapear páginas de memoria en el espacio de direcciones de un proceso. Tiene varias ventajas sobre `malloc` y `sbrk`:
1. **Mapeo de archivos**: `mmap` puede mapear un archivo directamente en la memoria, lo que puede ser más eficiente que leer el archivo en un búfer.
2. **Granularidad de la página**: `mmap` asigna memoria en bloques del tamaño de una página, lo que puede ser más eficiente para grandes cantidades de memoria.
3. **Memoria compartida**: `mmap` puede crear regiones de memoria que se comparten entre varios procesos.
4. **Asignación de memoria anónima**: `mmap` puede asignar memoria que no está respaldada por un archivo, similar a `malloc` y `sbrk`.

Sin embargo, `mmap` también tiene algunas desventajas que pueden hacer que no siempre sea la mejor opción:
1. **Overhead**: `mmap` tiene un overhead significativo en comparación con `malloc` y `sbrk`. Cada llamada a `mmap` implica una llamada al sistema, que es más costosa que una llamada a una función de biblioteca como `malloc`.
2. **Fragmentación de memoria**: Aunque `mmap` puede reducir la fragmentación de memoria en comparación con `malloc` y `sbrk`, todavía puede haber fragmentación si se asignan y liberan bloques de diferentes tamaños.
3. **Portabilidad**: No todos los sistemas operativos soportan `mmap`, o pueden no soportar todas las características de `mmap`.

Por estas razones, aunque `mmap` puede ser útil en algunas situaciones, no siempre es la mejor opción para la asignación de memoria.

### Validacion
El codigo completo de `memory.c` es:
```c
#include "memory.h"
#include <sys/mman.h>

typedef struct s_block *t_block;
void *base = NULL;
int method = 0;

t_block find_block(t_block *last, size_t size)
{
    t_block b = base;
    if (method == FIRST_FIT)
    {
        while (b && !(b->free && b->size >= size))
        {
            *last = b;
            b = b->next;
        }
        return (b);
    }
    else
    {
        size_t dif = PAGESIZE;
        t_block best = NULL;
        while (b)
        {
            if (b->free)
            {
                if (b->size == size)
                {
                    return b;
                }
                if (b->size > size && (b->size - size) < dif)
                {
                    dif = b->size - size;
                    best = b;
                }
            }
            *last = b;
            b = b->next;
        }
        return best;
    }
}

void split_block(t_block b, size_t s)
{
    t_block new;
    new = (t_block)(b->data + s);
    new->size = b->size - s - BLOCK_SIZE;
    new->next = b->next;
    new->free = 1;
    b->size = s;
    b->next = new;
}

void copy_block(t_block src, t_block dst)
{
    int *sdata, *ddata;
    size_t i;
    sdata = src->ptr;
    ddata = dst->ptr;
    for (i = 0; (i * 4) < src->size && (i * 4) < dst->size; i++)
        ddata[i] = sdata[i];
}

t_block get_block(void *p)
{
    char *tmp;
    tmp = p;
    return (p = tmp -= BLOCK_SIZE);
}

int valid_addr(void *p)
{
    if (base)
    {
        if (p > base && p < sbrk(0))
        {
            return (p == (get_block(p))->ptr);
        }
    }
    return (0);
}

t_block fusion(t_block b)
{
    if (b->next && b->next->free)
    {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next)
            b->next->prev = b;
    }
    return b;
}

t_block extend_heap(t_block last, size_t s)
{
    t_block b;
    b = mmap(0, s, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (b == MAP_FAILED) {
        return NULL;
    }
    b->size = s;
    b->next = NULL;
    b->prev = last;
    b->ptr = b->data;
    if (last)
        last->next = b;
    b->free = 0;
    return b;
}

void get_method(int m)
{
    method = m;
}

void set_method(int m)
{
    method = m;
}

void malloc_control(int m)
{
    if (m == 0)
    {
        set_method(0);
    }
    else if (m == 1)
    {
        set_method(1);
    }
    else
    {
        printf("Error: invalid method\n");
    }
}

void *malloc(size_t size)
{
    t_block b, last;
    size_t s;
    s = align(size);
    if (base)
    {
        last = base;
        b = find_block(&last, s);
        if (b)
        {
            if ((b->size - s) >= (BLOCK_SIZE + 4))
                split_block(b, s);
            b->free = 0;
        }
        else
        {
            b = extend_heap(last, s);
            if (!b)
                return (NULL);
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
            return (NULL);
        base = b;
    }
    return (b->data);
}

void free(void *ptr)
{
    t_block b;
    if (valid_addr(ptr))
    {
        b = get_block(ptr);
        b->free = 1;
        if (b->next && b->next->free)
            fusion(b);
        if (b->prev && b->prev->free)
            fusion(b->prev);
        else
        {
            if (b->next)
                b->next->prev = b;
            if (b->prev)
                b->prev->next = b;
            else
                base = b;
            b->free = 1;
            b->prev = NULL;
        }
    }
}

void *calloc(size_t number, size_t size)
{
    size_t *new;
    size_t s4, i;
    if (!number || !size)
    {
        return (NULL);
    }
    new = malloc(number * size);
    if (new)
    {
        s4 = align(number * size) << 2;
        for (i = 0; i < s4; i++)
            new[i] = 0;
    }
    return (new);
}

void *realloc(void *ptr, size_t size)
{
    size_t s;
    t_block b, new;
    void *newp;
    if (!ptr)
        return (malloc(size));
    if (valid_addr(ptr))
    {
        s = align(size);
        b = get_block(ptr);
        if (b->size >= s)
        {
            if (b->size - s >= (BLOCK_SIZE + 4))
                split_block(b, s);
        }
        else
        {
            if (b->next && b->next->free && (b->size + BLOCK_SIZE + b->next->size) >= s)
            {
                fusion(b);
                if (b->size - s >= (BLOCK_SIZE + 4))
                    split_block(b, s);
            }
            else
            {
                newp = malloc(s);
                if (!newp)
                    return (NULL);
                new = get_block(newp);
                copy_block(b, new);
                free(ptr);
                return (newp);
            }
        }
        return (ptr);
    }
    return (NULL);
}

void check_heap(void *data)
{
	t_block block = get_block(data);
    printf("\033[1;33mHeap check\033[0m\n");
    printf("Size: %zu\n", block->size);
    printf("Next block: %p\n", (void *)(block->next));
    printf("Prev block: %p\n", (void *)(block->prev));
    printf("Free: %d\n", block->free);
    printf("Beginning data address: %p\n", block->ptr);
    printf("Last data address: %p\n", (void *)((char *)(block->ptr) + block->size));
    printf("Heap address: %p\n", sbrk(0));
}
```
Se crea un `main.c` para validar el programa, el cual es:
```c
#include <stdio.h>
#include "memory.h"

int main()
{
    printf("\033[0;34mMalloc First Fit Test\033[0m\n");
    malloc_control(FIRST_FIT);
    char *char1 = NULL;
    char1 = malloc(32);
    strcpy(char1, "str 1");
    check_heap(char1);

    printf("\n\033[0;34mMalloc Best Fit Test\033[0m\n");
    malloc_control(BEST_FIT);
    char *char2 = NULL;
    char2 = malloc(16);
    strcpy(char2, "str 2");
    check_heap(char2);

    printf("\n\033[0;34mCalloc Test\033[0m\n");
    char *char3 = NULL;
    char3 = calloc(64, sizeof(char));
    strcpy(char3, "str 3");
    check_heap(char3);

    printf("\n\033[0;34mRealloc Test\033[0m\n");
    char *char4 = NULL;
    char4 = realloc(char4, 128);
    strcpy(char4, "str 4");
    check_heap(char4);

    free(char1);
    free(char2);
    free(char3);
    free(char4);
    return 0;
}
```
Se tiene un Makefile como a continuacion:
```makefile
CC = gcc
CFLAGS = -Wall -pedantic -Werror -Wextra -Wconversion -std=gnu11
DEPS = memory.h
OBJ = main.o memory.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

program: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ) program
```
Una vez compilado (no tiene errores ni warnings), al ejecutar se logra la siguiente salida:
```c
Malloc First Fit Test
Heap check
Size: 32
Next block: (nil)
Prev block: 0x7f58ec459000
Free: 0
Beginning data address: 0x7f58ec41f028
Last data address: 0x7f58ec41f048
Heap address: 0x56192d54b000

Malloc Best Fit Test
Heap check
Size: 16
Next block: (nil)
Prev block: 0x7f58ec41f000
Free: 0
Beginning data address: 0x7f58ec41e028
Last data address: 0x7f58ec41e038
Heap address: 0x56192d54b000

Calloc Test
Heap check
Size: 64
Next block: (nil)
Prev block: 0x7f58ec41e000
Free: 0
Beginning data address: 0x7f58ec41d028
Last data address: 0x7f58ec41d068
Heap address: 0x56192d54b000

Realloc Test
Heap check
Size: 128
Next block: (nil)
Prev block: 0x7f58ec41d000
Free: 0
Beginning data address: 0x7f58ec41c028
Last data address: 0x7f58ec41c0a8
Heap address: 0x56192d54b000
```

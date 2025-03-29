#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define N 8                 // Tamaño del buffer
#define LIM 60              // Número de iteraciones

typedef struct {
    char buffer[N];         // Cadena compartida< de caracteres
    int elementos;          // Número de elementos en el buffer
} SharedMemory;

// Función que retira del buffer el caracter que corresponda y lo devuelve
char remove_item(SharedMemory *shm){
    char caracter = shm->buffer[shm->elementos];
    printf("\033[1;34m[Consumidor] Extraído %c de posición %d\n", caracter, shm->elementos);
    --shm->elementos;
    return caracter;
}

// Función que consume un caracter
void consume_item(char caracter){
    printf("\033[1;34m[Consumidor] Consumido: %c\n", caracter);
}

int main(){
    srand(time(NULL));

    // Abrir semáforos creados por el productor
    sem_t *vacias = sem_open("VACIAS", 0);
    sem_t *llenas = sem_open("LLENAS", 0);
    sem_t *mutex  = sem_open("MUTEX", 0);

    // Abrir memoria compartida
    int fd = shm_open("/shm_prodcons", O_RDWR, 0666);
    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for (int i = 0; i < LIM; i++) {
        sem_wait(llenas);  // Espera hasta que haya elementos llenos
        sem_wait(mutex);   // Entra a la sección crítica

        char c = remove_item(shm);

        sem_post(mutex);   // Sale de la sección crítica
        sem_post(vacias);  // Incrementa los espacios vacíos

        consume_item(c);
        sleep(rand() % 4); // Pausa aleatoria (0-3 seg) fuera de la región crítica
    }

    // Liberar recursos
    munmap(shm, sizeof(SharedMemory));
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);

    return 0;
}

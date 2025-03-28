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
    char buffer[N];         // Cadena compartida de caracteres
    int elementos;          // Número de elementos en el buffer
} SharedMemory;

char produce_item(){
    return 'A' + (rand() % 26);
}

void insert_item(SharedMemory *shm, char caracter){
    shm->buffer[shm->elementos] = caracter;
    printf("[Productor] Insertado %c en posición %d\n", caracter, shm->elementos - 1);
    shm->elementos++;
}

int main(){
    srand(time(NULL));

    // Eliminar semáforos previos (por seguridad)
    sem_unlink("VACIAS"); sem_unlink("LLENAS"); sem_unlink("MUTEX");

    // Crear semáforos
    sem_t *vacias = sem_open("VACIAS", O_CREAT, 0700, N);  // Inicialmente hay N espacios vacíos
    sem_t *llenas = sem_open("LLENAS", O_CREAT, 0700, 0);  // No hay elementos al inicio
    sem_t *mutex  = sem_open("MUTEX", O_CREAT, 0700, 1);   // Exclusión mutua inicializada en 1

    // Crear memoria compartida
    int fd = shm_open("/shm_prodcons", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedMemory));
    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    shm->elementos = 0;  // Inicializar buffer

    for (int i = 0; i < LIM; i++) {
        sleep(rand() % 4);  // Pausa aleatoria (0-2 seg) fuera de la región crítica
        char c = produce_item();

        sem_wait(vacias);   // Decrementa espacios vacíos
        sem_wait(mutex);    // Entra a la sección crítica

        insert_item(shm, c);

        sem_post(mutex);    // Sale de la sección crítica
        sem_post(llenas);   // Incrementa elementos llenos
    }

    // Liberar recursos
    munmap(shm, sizeof(SharedMemory));
    sem_close(vacias);
    sem_close(llenas);
    sem_close(mutex);

    // Eliminar semáforos
    sem_unlink("VACIAS"); sem_unlink("LLENAS"); sem_unlink("MUTEX");

    return 0;
}

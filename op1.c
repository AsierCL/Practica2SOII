#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define N 8
#define LIM 60

typedef struct {
    char buffer[N];
    int elementos;
} SharedMemory;
SharedMemory shm;

char buffer_local[N];
int buff = 0;

sem_t vacias;
sem_t llenas;
sem_t mutex;

char produce_item() {
    return 'A' + (rand() % 26);
}

void insert_item(SharedMemory *shm, char caracter) {
    shm->buffer[shm->elementos] = caracter;
    printf("\033[1;33m[Productor] Insertado %c en posición %d\n", caracter, shm->elementos);
    shm->elementos++;
}

char remove_item(SharedMemory *shm) {
    shm->elementos--;
    char caracter = shm->buffer[shm->elementos];
    printf("\033[1;34m[Consumidor] Extraído %c de posición %d\n", caracter, shm->elementos);
    return caracter;
}

// Función que consume un caracter y lo imprime 
void consume_item(char caracter){
    if(buff < N){
        buffer_local[buff]=caracter;
        buff++;
    }else{
        buffer_local[N]=caracter;
    }

    printf("\033[1;34m[Consumidor] Buffer local: %s\n", buffer_local);
}

// Hilo productor
void* productor(void* arg) {
    for (int i = 0; i < LIM; i++) {
        sleep(rand() % 4);  // Pausa aleatoria (0-3 segundos)
        char c = produce_item();

        sem_wait(&vacias);
        sem_wait(&mutex);

        insert_item(&shm, c);

        sem_post(&mutex);   // Sale de la sección crítica
        sem_post(&llenas);  // Indica que hay un elemento disponible
    }
    pthread_exit(NULL);
}

// Hilo consumidor
void* consumidor(void* arg) {
    for (int i = 0; i < LIM; i++) {
        sem_wait(&llenas);
        sem_wait(&mutex);

        char c = remove_item(&shm);

        sem_post(&mutex);   // Sale de la sección crítica
        sem_post(&vacias);  // Indica que hay un espacio vacío

        consume_item(c);
        sleep(rand() % 4);  // Pausa aleatoria (0-3 segundos)
    }
    pthread_exit(NULL);
}

int main(){
    srand(time(NULL));
    shm.elementos = 0;

    // Inicializar buffer_local a cadena vacía
    buffer_local[0] = '\0';

    // Inicializar semáforos
    sem_init(&vacias, 0, N);
    sem_init(&llenas, 0, 0);
    sem_init(&mutex, 0, 1);

    pthread_t prod, cons;

    // Crear hilos
    pthread_create(&prod, NULL, productor, NULL);
    pthread_create(&cons, NULL, consumidor, NULL);

    // Esperar a que terminen los hilos
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    // Destruir semáforos
    sem_destroy(&vacias);
    sem_destroy(&llenas);
    sem_destroy(&mutex);

    return 0;
}

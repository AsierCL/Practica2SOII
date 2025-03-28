// Don Asier Cabo (que no Caba) Lodeiro y Hugo Gilsanz Ortellado
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define N 8                 // Tamaño del buffer
typedef struct {
    char buffer[N];         // Cadena compartida de caracteres
    int elementos;          // Número de elementos rellenados del buffer
} SharedMemory;

//Buffer local cola LIFO
char buffer_local[N];
int buff;

// Función que retira del buffer el caracter que corresponda y lo devuelve
char remove_item(SharedMemory *shm){
    char caracter = shm->buffer[shm->elementos];
    printf("[Consumidor] Extraído %c de posición %d\n", caracter, shm->elementos);
    /*  
    *   Facemos sleep entre o borrado e o decremento do contador  
    *   para que o proceso perda a CPU e aumentar a probabilidade de
    *   que ocorra unha carreira crítica.
    */
    sleep(1);
    shm->elementos--;
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

    printf("[Consumidor] Buffer local: %s\n", buffer_local);
}

int main(int argc, char const *argv[]) {
    // Abrir memoria compartida
    int fd = shm_open("/shm_prodcons", O_RDWR, 0666);
    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(int i = 0; i < N; i++) {
        buffer_local[i] = '\0';
    }
    buff = 0;

    while (1) {
        char c = remove_item(shm);
        consume_item(c);

        if (shm->elementos <= 0) {
            printf("[Consumidor] Buffer vacío, esperando...\n");
            while (shm->elementos <= 0);  // Espera activa
        }
    }

    return 0;
}

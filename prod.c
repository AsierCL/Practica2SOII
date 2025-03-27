// Don Asier Cabo (que no Caba) Lodeiro y Hugo Gilsanz Ortellado
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define N 8                 // Tamaño del buffer
typedef struct {
    char buffer[N];         // Cadena compartida de caracteres
    int elementos;          // Número de elementos rellenados del buffer
} SharedMemory;

// Función que genera una mayúscula aleatoria
char produce_item(){
    return 'A' + (rand() % 26);
}

// Función que recibe un caracter y lo añade a la cadena pasada por referencia
void insert_item(SharedMemory *shm, char caracter){
    shm->buffer[shm->elementos] = caracter;
    printf("[Productor] Insertado %c en posición %d\n", caracter, shm->elementos);
    sleep(1);
    shm->elementos++;
}

int main(int argc, char const *argv[]){
    srand(time(NULL));
    
    // Crear memoria compartida
    int fd = shm_open("/shm_prodcons", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedMemory));
    SharedMemory *shm = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    shm->elementos = 0;  // Inicializa el contador de elementos
    for(int i = 0; i < N; i++) shm->buffer[i] = '\0';
    while (1) {
        char c = produce_item();
        insert_item(shm, c);
        
        if (shm->elementos >= N) {
            printf("[Productor] Buffer lleno, esperando...\n");
            while (shm->elementos >= N);  // Espera activa
        }
    }

    return 0;
}
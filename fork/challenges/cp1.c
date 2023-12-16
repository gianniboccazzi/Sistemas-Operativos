#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


int
main(int argc, char *argv[])
{   if (argc != 3) {
        printf("Error en el input");
        exit(-1);
    }
    int fd_origen;                 
    struct stat stat_origen;  
    int fd_destino;                 
    struct stat stat_destino;  
    fd_origen = open(argv[1], O_RDWR);
    if (fd_origen == -1) {
        printf("Error al leer el archivo");
        exit(-1);
    }
    fstat(fd_origen, &stat_origen);
    char *contenido_origen = mmap(NULL, stat_origen.st_size, PROT_READ, MAP_PRIVATE, fd_origen, 0);
    if (contenido_origen == MAP_FAILED) {
        printf("Fallo en mmap");
        exit(-1);
    }
    fd_destino = open(argv[2], O_RDWR | O_CREAT, 0666);
    if (fd_destino == -1) {
        printf("Error al crear el archivo destino");
        exit(-1);
    }
    ftruncate(fd_destino, stat_origen.st_size);
    char *contenido_destino = mmap(NULL, stat_destino.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_destino, 0);
    if (contenido_destino == MAP_FAILED) {
        perror("Fallo en mmap");
        exit(-1);
    }
    memcpy(contenido_destino, contenido_origen, stat_origen.st_size);
    close(fd_origen);
    close(fd_destino);
    munmap(contenido_origen, stat_origen.st_size);
    munmap(contenido_destino, stat_destino.st_size);
    return 0;
}


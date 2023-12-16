#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h> 
#include <sys/stat.h>
#include <fcntl.h>




void leer_directorio(char* directorio, char* argumento, int dir_fd, char *(*funcion_usada)(const char *, const char *), char* ruta_completa);
char *caso_sensitivo(const char *cadena, const char *subcadena);
char *caso_insensitivo(const char *cadena, const char *subcadena);

int
main(int argc, char *argv[])
{   
    char *(*funcion_usada)(const char *, const char *);
    char *argumento = argv[1];
    char ruta_completa[PATH_MAX];
    if (argc < 2) {
        printf("No se proporcionaron argumentos\n");
        exit(-1);
    }
    if (argc > 3 && strcmp(argv[1], "-i") == 0) {
        funcion_usada = caso_insensitivo;
        argumento = argv[2];
    } else {
        funcion_usada = caso_sensitivo;
    }
    DIR *directory = opendir(".");
    if (directory == NULL){
        printf("Error con opendir");
        exit(-1);
    }
    struct dirent* entry;
    while((entry = readdir(directory))){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (entry->d_type == DT_DIR){
            if(funcion_usada(entry->d_name, argumento) != NULL){
                printf("%s\n", entry->d_name);
            }
            int dir_fd_actual = dirfd(directory);
            strcpy(ruta_completa, entry->d_name);
            leer_directorio(entry->d_name, argumento, dir_fd_actual, funcion_usada, ruta_completa);
            
        }
        if(entry->d_type == DT_REG){
            if(funcion_usada(entry->d_name, argumento) != NULL){
                printf("%s\n", entry->d_name);
            }
        }
    }
	return 0;
}

void leer_directorio(char* directorio, char* argumento, int dir_fd, char *(*funcion_usada)(const char *, const char *), char* ruta_completa){
    int archivo_fd = openat(dir_fd, directorio, O_DIRECTORY);
    if (archivo_fd == -1) {
        printf("Error al abrir el archivo");
        exit(-1);
    }
    DIR *dir_nuevo = fdopendir(archivo_fd);
    if (dir_nuevo == NULL) {
        printf("Error al convertir el fd en un DIR");
        exit(-1);
    }
    struct dirent* entry;
    while((entry = readdir(dir_nuevo))){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (entry->d_type == DT_DIR){
            char ruta_nueva_completa[PATH_MAX];
            snprintf(ruta_nueva_completa, sizeof(ruta_nueva_completa), "%s/%s", ruta_completa, entry->d_name);
            if(funcion_usada(entry->d_name, argumento) != NULL){
                printf("%s\n", ruta_nueva_completa);
            }
            int dir_fd_actual = dirfd(dir_nuevo);
            leer_directorio(entry->d_name, argumento, dir_fd_actual, funcion_usada, ruta_nueva_completa);
        }
        if(entry->d_type == DT_REG){
            if(funcion_usada(entry->d_name, argumento) != NULL){
                char ruta_nueva_completa[PATH_MAX];
                snprintf(ruta_nueva_completa, sizeof(ruta_nueva_completa), "%s/%s", ruta_completa, entry->d_name);
                printf("%s\n", ruta_nueva_completa);
            }
        }
    }
}   



char *caso_sensitivo(const char *cadena, const char *subcadena) {
    return strstr(cadena, subcadena);
}

char *caso_insensitivo(const char *cadena, const char *subcadena) {
    return strcasestr(cadena, subcadena);
}
#define _GNU_SOURCE
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


char* nombre_proceso(FILE *ruta);
int
main(int argc, char *argv[])
{   
    DIR *directory = opendir("/proc");
    if (directory == NULL){
        printf("Error con opendir");
        exit(-1);
    }
    struct dirent* entry;
    while((entry = readdir(directory))){
        if (entry->d_type == DT_DIR){
            char ruta_str[PATH_MAX];
            if(!isdigit(entry->d_name[0])){
                continue;
            }
            snprintf(ruta_str, sizeof(ruta_str), "/proc/%s/comm", entry->d_name);
            FILE *ruta = fopen(ruta_str, "r");
            char *comando = nombre_proceso(ruta);
            printf("%-1s %s", entry->d_name, comando);
            free(comando);
            fclose(ruta);
        }
    }
    return 0;
}

char* nombre_proceso(FILE *ruta){
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    if ((read = getline(&line, &len, ruta)) != -1) {
        return line;
    } else {
        printf("Error al leer el archivo");
    }
}
#include "argumentos.h"

extern FILE *fp;

void procesar_argumentos(int* argc, char* argv[]) {

    for (int i=0; i < *argc - 1; i++) {
        if (strcmp(argv[i], "-bu") == 0) {
            
            if (access(argv[i+1], F_OK) == 0) {
                fp = fopen(argv[i+1], "r+");
            } else {
                fp = fopen(argv[i+1], "w+");
            }

            *argc -= 2;

            return;
        }
    }

    fp = fopen("backup.fisop", "w+");

    return;
}

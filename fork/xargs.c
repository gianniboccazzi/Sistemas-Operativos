#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#ifndef NARGS
#define NARGS 4
#endif
#define ARGUMENTOS_AGREGADOS 2

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Error en el input");
		exit(-1);
	}
	bool terminado = false;
	while (terminado == false) {
		char *linea = NULL;
		char *rutas[NARGS + ARGUMENTOS_AGREGADOS] = { NULL };
		size_t len = 0;
		for (int i = 1; i <= NARGS; i++) {
			int check = getline(&linea, &len, stdin);
			if (check < 0) {
				if (i == 1) {
					return 0;
				}
				terminado = true;
				break;
			}
			if (linea[check - 1] == '\n') {
				linea[check - 1] = '\0';
			}
			rutas[i] = strdup(linea);
		}
		rutas[0] = strdup(argv[1]);
		pid_t hijo_id = fork();
		if (hijo_id < 0) {
			printf("Error en fork\n");
			exit(-1);
		}
		if (hijo_id == 0) {
			int res = execvp(argv[1], rutas);
			if (res < 0) {
				printf("Error en execvp\n");
				exit(-1);
			}
		} else {
			waitpid(hijo_id, NULL, 0);
			free(linea);
		}
	}
	return 0;
}
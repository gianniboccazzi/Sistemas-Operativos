#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define READ 0
#define WRITE 1

void filtrar_numero(int fds_anterior[2]);

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Error en el input");
		exit(-1);
	}
	int n = atoi(argv[1]);
	int padre_filtro1_fds[2];
	if (pipe(padre_filtro1_fds) < 0) {
		printf("Error en pipe\n");
		exit(-1);
	}
	pid_t hijo_id = fork();
	if (hijo_id < 0) {
		printf("Error en fork\n");
		exit(-1);
	}
	if (hijo_id == 0) {
		close(padre_filtro1_fds[WRITE]);
		filtrar_numero(padre_filtro1_fds);
	} else {
		for (int i = 2; i <= n; i++) {
			int res = write(padre_filtro1_fds[WRITE], &i, sizeof i);
			if (res < 0) {
				printf("Error en write padre\n");
				exit(-1);
			}
		}
		close(padre_filtro1_fds[READ]);
		close(padre_filtro1_fds[WRITE]);
		waitpid(hijo_id, NULL, 0);
	}
	return 0;
}

void
filtrar_numero(int fds_anterior[2])
{
	int nuevo_filtro_fds[2];
	int valor_recibido;
	int check =
	        read(fds_anterior[READ], &valor_recibido, sizeof valor_recibido);
	if (check < 0) {
		printf("Error en read hijo\n");
		exit(-1);
	}
	if (check == 0) {
		close(fds_anterior[READ]);
		return;
	}
	printf("primo %d\n", valor_recibido);
	if (pipe(nuevo_filtro_fds) < 0) {
		perror("Error en pipe\n");
		exit(-1);
	}
	pid_t nuevo_hijo_id = fork();
	if (nuevo_hijo_id < 0) {
		printf("Error en fork hijo\n");
		exit(-1);
	}
	if (nuevo_hijo_id == 0) {
		close(nuevo_filtro_fds[WRITE]);
		close(fds_anterior[READ]);
		filtrar_numero(nuevo_filtro_fds);
	} else {
		close(nuevo_filtro_fds[READ]);
		while (true) {
			int nuevo_valor;
			int check = read(fds_anterior[READ],
			                 &nuevo_valor,
			                 sizeof nuevo_valor);
			if (check < 0) {
				printf("Error en read hijo\n");
				exit(-1);
			}
			if (check == 0) {
				close(fds_anterior[READ]);
				close(nuevo_filtro_fds[WRITE]);
				break;
			}
			if (nuevo_valor % valor_recibido != 0) {
				int res = write(nuevo_filtro_fds[WRITE],
				                &nuevo_valor,
				                sizeof nuevo_valor);
				if (res < 0) {
					printf("Error en write hijo\n");
					exit(-1);
				}
			}
		}
		waitpid(nuevo_hijo_id, NULL, 0);
	}
}

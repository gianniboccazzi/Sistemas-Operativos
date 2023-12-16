#define FUSE_USE_VERSION 30
#define MAX_CONTENIDO 100
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "structs.h"
#include "persistence.h"
#include "argumentos.h"

struct FileSystem fs; 
FILE *fp;

struct pos find_pos(const char *path) {
	struct pos pos;
	pos.i = -1;
	pos.j = -1;

	for (int i = 0; i < LADO_MAPA && fs.map[i][0].cell_status != FREE_T; i++) {
		if (strcmp(fs.map[i][0].path, path) == 0) {
			pos.i = i;
			pos.j = 0;
			return pos;
		}

		for (int j = 1; j < LADO_MAPA && fs.map[i][j].cell_status != FREE_T; j++) {
			if (strcmp(fs.map[i][j].path, path) == 0) {
				pos.i = i;
				pos.j = j;
				return pos;
			}
		}
	}

	return pos;
}

void clear_cell(int i, int j) {
	fs.map[i][j].cell_status = DELETED_T;
	fs.map[i][j].type = NOTHING_T;
	for (int aux=0; aux < LARGO_PATH; aux++) {
		fs.map[i][j].path[aux]=0;
	}
	fs.map[i][j].metadata.gid = 0;
	fs.map[i][j].metadata.propietario = 0;
	fs.map[i][j].metadata.mode = 0;
	strcpy(fs.map[i][j].contenido, "");
}

void get_directory(const char *path, char *directory) {
	// Find the last occurrence of '/' in the path
	const char *lastSlash = strrchr(path, '/');

	if (lastSlash == NULL) {
		// No '/' found, it's a file in the root directory
		strcpy(directory, "/");
	} else {
		// Copy the directory part to the output string
		size_t length = lastSlash - path;
		strncpy(directory, path, length);
		// Ensure the directory string is null-terminated
		directory[length] = '\0';
	}
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (path == NULL){
		return -ENOENT;
	}

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
		return 0;
	}

	struct pos position = find_pos(path);

	if (position.i == -1 || position.j == -1) {
		return -ENOENT;
	}

	st->st_uid = fs.map[position.i][position.j].metadata.propietario;
	if (fs.map[position.i][position.j].type == FILE_T) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = strlen(fs.map[position.i][position.j].contenido);
	} else {
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	}
	st->st_atime = fs.map[position.i][position.j].metadata.fecha_acceso.tv_sec;
	st->st_mtime = fs.map[position.i][position.j].metadata.fecha_modificacion.tv_sec;
	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		for (int i = 0; i < LADO_MAPA && fs.map[i][0].cell_status != FREE_T; i++) {
			if (fs.map[i][0].cell_status == TAKEN_T){
				filler(buffer, fs.map[i][0].path+1, NULL, 0);
			}
		}
		return 0;
	} else {
		struct pos position = find_pos(path); // "/"
		
		if (position.i == -1 || position.j == -1) {
			printf("No se encontro el path: %s\n", path);
			return -1;
		}

		if (fs.map[position.i][position.j].type == FILE_T) {
			return -1;
		}
		for (int i = 1; i < LADO_MAPA && fs.map[position.i][i].cell_status != FREE_T; i++) {
			if (fs.map[position.i][i].cell_status == TAKEN_T){
				filler(buffer, fs.map[position.i][i].path+strlen(path)+1, NULL, 0);
			}
		}
	}
	printf("Se leyo el directorio: %s\n", path);
	return 0;
}
#define MAX_CONTENIDO 100
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct pos position = find_pos(path);

	if (position.i == -1 || position.j == -1) {
		return -ENOENT;
	}

	if (fs.map[position.i][position.j].type == DIR_T) {
		return -EISDIR;
	}

	if (offset + size > strlen(fs.map[position.i][position.j].contenido)){
		size = strlen(fs.map[position.i][position.j].contenido) - offset;
	}
	size = size > 0 ? size : 0;

	strncpy(buffer, fs.map[position.i][position.j].contenido + offset, size);

	return size;
}


static int fisops_mknod(const char *path, mode_t mode, dev_t dev){

	printf("[debug] fisops_mknod - path: %s, mode: %d, dev: %ld\n",
	       path,
	       mode,
	       dev);

	struct pos position = find_pos(path);
	if (position.i != -1 && position.j != -1) {
		return -EEXIST;
	}

	if (path == NULL) {
		return -ENOENT;
	}

	char directory[LARGO_NOMBRE+LARGO_NOMBRE+1];
	get_directory(path, directory);
	int t = -1;

	for (int i = 0; i < LADO_MAPA && fs.map[i][0].cell_status != FREE_T; i++) {
		if (strcmp(fs.map[i][0].path, directory) == 0) {
			t = i;
			break;
		}
	}
	printf("t: %d\n", t);

	if (t == -1) {
		for (int t=0; t < LADO_MAPA; t++) {

			if (fs.map[t][0].cell_status != TAKEN_T) { //Cambiar por != a TAKEN

				fs.map[t][0].cell_status = TAKEN_T;

				fs.map[t][0].type = FILE_T;
				strcpy(fs.map[t][0].path, path);
				fs.map[t][0].metadata.gid = getgid();
				fs.map[t][0].metadata.propietario = getuid();
				fs.map[t][0].metadata.mode = mode;
				return 0;
			}
		}
	}	

	for (int i=1; i < LADO_MAPA; i++) {
		if (fs.map[t][i].cell_status != TAKEN_T && fs.map[t][0].type == DIR_T) { //Cambiar por != a TAKEN
			printf("esta en directory: %s\n" , directory);
			printf("esta en t: %d, i: %d\n", t, i);
			printf("el cell status es: %d\n", fs.map[t][i].cell_status);
			fs.map[t][i].cell_status = TAKEN_T;
			fs.map[t][i].type = FILE_T;
			strcpy(fs.map[t][i].path, path);
			printf("path que se guarda: %s\n", path);
			fs.map[t][i].metadata.gid = getgid();
			fs.map[t][i].metadata.propietario = getuid();
			fs.map[t][i].metadata.mode = mode;
			return 0;
		}
	}

	

	return -ENOMEM;
}

static int fisopfs_utimens (const char *path, const struct timespec tv[2]){
	printf("[debug] fisopfs_utimens - path: %s, tv[0]: %lu, tv[1]: %lu\n",
	       path,
	       (unsigned long)tv[0].tv_sec,
	       (unsigned long)tv[1].tv_sec);
	
	struct pos position = find_pos(path);

	if (position.i == -1 || position.j == -1) {
		return -ENOENT;
	}

	fs.map[position.i][position.j].metadata.fecha_acceso = tv[0];
	fs.map[position.i][position.j].metadata.fecha_modificacion = tv[1]; 

	return 0;
}


static int fisopfs_write(const char *path, const char *content, size_t size, off_t offset, struct fuse_file_info *f){
	printf("[debug] fisop_write(%s)\n", path);
	struct pos position = find_pos(path);

	if (position.i == -1 || position.j == -1) {
		return -ENOENT;
	} 

	if (fs.map[position.i][position.j].type == DIR_T) {
		return -EISDIR;
	}

	if (offset > strlen(fs.map[position.i][position.j].contenido)){
		offset = strlen(fs.map[position.i][position.j].contenido);
	}

	if (size + strlen(fs.map[position.i][position.j].contenido) > LARGO_CONTENIDO){
		size = LARGO_CONTENIDO - strlen(fs.map[position.i][position.j].contenido);
	}

	for (int i = 0; i < size; i++){
		fs.map[position.i][position.j].contenido[offset + i] = content[i];
	}
	
	return size;
	
}

static int fisopfs_open(const char *path , struct fuse_file_info *f){
	printf("[debug] fisopfs_open - path: %s\n", path);
	if (strcmp(path, "\0") == 0) {
		return 0;
	}

	struct pos position = find_pos(path);
	if (position.i == -1 || position.j == -1) {
		return -ENOENT;
	}

	return 0;
}

static int fisopfs_mkdir(const char *path, mode_t mode){
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	if (path == NULL) {
		return -ENOENT;
	}
	for (int t=0; t < LADO_MAPA; t++) {
		if (fs.map[t][0].cell_status == FREE_T) {
			fs.map[t][0].cell_status = TAKEN_T;
			fs.map[t][0].type = DIR_T;
			strcpy(fs.map[t][0].path, path);
			fs.map[t][0].metadata.gid = getgid();
			fs.map[t][0].metadata.propietario = getuid();
			fs.map[t][0].metadata.mode = mode;
			return 0;
		}
	}
	return -ENOMEM;
}

static int fisopfs_rmdir(const char *path){
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	struct pos position = find_pos(path);

	printf("Pos: %d, %d\n", position.i, position.j);

	if (position.i == -1 || position.j == -1) {
		printf("[debug] fisopfs_rmdir - dir not found\n");
		return -ENOENT;
	}

	if (fs.map[position.i][position.j].type == FILE_T) {
		printf("[debug] fisopfs_rmdir - not a dir\n");
		return -ENOTDIR;
	}

	for (int j = 1; j < LADO_MAPA && fs.map[position.i][j].cell_status != FREE_T; j++) {
		if (fs.map[position.i][j].cell_status == TAKEN_T) {
			printf("[debug] fisopfs_rmdir - dir not empty\n");
			return -ENOTEMPTY;
		}
	}

	clear_cell(position.i, position.j);

	return 0;
}

static int fisopfs_unlink(const char *path){
	
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	struct pos position = find_pos(path);
	
	if (position.i == -1 || position.j == -1) {
		printf("[debug] fisopfs_unlink - file not found\n");
		return -ENOENT;
	}

	if (fs.map[position.i][position.j].type == DIR_T) {
		printf("[debug] fisopfs_rmdir - not a file\n");
		return -EISDIR;
	}

	clear_cell(position.i, position.j);
	return 0;
}


static int fisopfs_truncate(const char *path, off_t size) {
    return 0;
}

static void fisopfs_destroy(void *private_data){
	printf("[debug] fisopfs_destroy\n");
	save_fs();
	fclose(fp);
}

static int fisopfs_flush(const char *path, struct fuse_file_info *f){
	printf("[debug] fisopfs_flush - path: %s\n", path);
	save_fs();
	return 0;
}

static void *fisopfs_init(struct fuse_conn_info *conn){
	printf("[debug] fisopfs_init\n");
	load_fs();
	return 0;

}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mknod = fisops_mknod,
	.utimens = fisopfs_utimens,
	.write = fisopfs_write,
	.open = fisopfs_open,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.truncate = fisopfs_truncate,
	.destroy = fisopfs_destroy,
	.flush = fisopfs_flush,
	.init = fisopfs_init,

};

int
main(int argc, char *argv[])
{
	procesar_argumentos(&argc, argv);

	return fuse_main(argc, argv, &operations, NULL);
}



// -Manejar los errores con constantes de errno.h
// 		ENOENT, ENOTDIR, EIO, EINVAL, EBIG, ENOMEM, etc
// -Utilizar estructuras estáticas para representar archivos y directorios
// 		Se recomienda segmentar la memoria de alguna forma (e.g. usando bloques)
// -Definir límites sensibles para:
// 		Cantidad de archivos/directorios totales (i.e. similar a la cantidad de i-nodos o de archivos por directorio)
// 		Tamaño máximo por archivo/directorio
// 		Tamaño máximo del filesystem

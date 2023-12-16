#ifndef STRUCTS_H
#define STRUCTS_H

#define LARGO_NOMBRE 20
#define LARGO_FECHA 22 // ``YYYY-MM-DD HH:MM:SS.SS´´
#define LADO_MAPA 20
#define LARGO_PATH 2*LARGO_NOMBRE + 2 //   "/dir/file"
#define LARGO_METADATA 2*LARGO_NOMBRE + LARGO_FECHA + 2*sizeof(struct timespec) + 2*sizeof(__uid_t) + sizeof(gid_t)
#define LARGO_CONTENIDO 400 
#define LARGO_FILE LARGO_METADATA + LARGO_CONTENIDO + LARGO_PATH + sizeof(cell_status_t) + sizeof(file_type_t)

typedef enum {FREE_T, TAKEN_T, DELETED_T} cell_status_t;
typedef enum {NOTHING_T, FILE_T, DIR_T} file_type_t;

struct File_Metadata {
    char nombre[LARGO_NOMBRE];
    char tipo[LARGO_NOMBRE];
    char fecha_creacion[LARGO_FECHA];        
    struct timespec fecha_modificacion;
    struct timespec fecha_acceso;
    __uid_t propietario;
    __uid_t mode;    
    gid_t gid;
};

struct File {
    char path[LARGO_PATH];
    cell_status_t cell_status;
    file_type_t type;
    struct File_Metadata metadata; 
    char contenido[LARGO_CONTENIDO];
};

struct FileSystem {
    struct File map[LADO_MAPA][LADO_MAPA];
};

struct pos {
	int i;
	int j;
};

#endif
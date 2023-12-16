#include "persistence.h"

extern struct FileSystem fs; 
extern FILE *fp; 

int save_fs () {

    rewind(fp);

    for (int i =0; i < LADO_MAPA; i++) {
        for (int j= 0; j < LADO_MAPA; j++) {
            
            fwrite(fs.map[i][j].path, sizeof(char), LARGO_PATH, fp);
            fwrite(&fs.map[i][j].cell_status, sizeof(cell_status_t), 1, fp);
            fwrite(&fs.map[i][j].type, sizeof(file_type_t), 1, fp);
            write_metadata(fp, i, j);
            fwrite(fs.map[i][j].contenido, sizeof(char), LARGO_CONTENIDO, fp);
            fwrite("\n", sizeof(char), 1, fp);
       }
    }

    return 0;
}

int load_fs () {

    char buffer[LARGO_FILE];

    for (int i=0; i<LADO_MAPA; i++) {
        for (int j=0; j < LADO_MAPA; j++) {

            if(!fgets(buffer, LARGO_FILE, fp)){
                printf("Error al leer el archivo de backup\n");
                return -1;
            }

            int offset = 0;

            memcpy(fs.map[i][j].path, buffer, LARGO_PATH);

            offset += LARGO_PATH;
            memcpy(&fs.map[i][j].cell_status, &buffer[offset], sizeof(cell_status_t));
            offset += sizeof(cell_status_t);
            memcpy(&fs.map[i][j].type, &buffer[offset], sizeof(file_type_t));
            offset += sizeof(file_type_t);
            load_metadata(buffer, i, j, &offset);
            memcpy(fs.map[i][j].contenido, &buffer[offset], LARGO_CONTENIDO);
        }
    }

    return 0;
}  

void write_metadata (FILE *fp, int i, int j) {
    
    fwrite(fs.map[i][j].metadata.nombre, sizeof(char), LARGO_NOMBRE, fp);
    fwrite(fs.map[i][j].metadata.fecha_creacion, sizeof(char), LARGO_FECHA, fp);
    fwrite(&fs.map[i][j].metadata.fecha_modificacion, sizeof(struct timespec), 1, fp);
    fwrite(&fs.map[i][j].metadata.fecha_acceso, sizeof(struct timespec), 1, fp);
    fwrite(&fs.map[i][j].metadata.propietario, sizeof(__uid_t), 1, fp);
    fwrite(&fs.map[i][j].metadata.mode, sizeof(__uid_t), 1, fp);
    fwrite(&fs.map[i][j].metadata.gid, sizeof(gid_t), 1, fp);
    
    return;
}

void load_metadata (char buffer[], int i, int j, int* offset) {
    
    memcpy(fs.map[i][j].metadata.nombre, &buffer[*offset], LARGO_NOMBRE);
    *offset += LARGO_NOMBRE;
    memcpy(fs.map[i][j].metadata.fecha_creacion, &buffer[*offset], LARGO_FECHA);
    *offset += LARGO_FECHA;
    memcpy(&fs.map[i][j].metadata.fecha_modificacion, &buffer[*offset], sizeof(struct timespec));
    *offset += sizeof(struct timespec);
    memcpy(&fs.map[i][j].metadata.fecha_acceso, &buffer[*offset], sizeof(struct timespec));
    *offset += sizeof(struct timespec);
    memcpy(&fs.map[i][j].metadata.propietario, &buffer[*offset], sizeof(__uid_t));
    *offset += sizeof(__uid_t);
    memcpy(&fs.map[i][j].metadata.mode, &buffer[*offset], sizeof(__uid_t));
    *offset += sizeof(__uid_t);
    memcpy(&fs.map[i][j].metadata.gid, &buffer[*offset], sizeof(gid_t));
    *offset += sizeof(gid_t);
    return;
}

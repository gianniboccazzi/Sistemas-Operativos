# fisop-fs

Lugar para respuestas en prosa y documentación del TP.


![img](/diagrama.png?raw=true "img")

En el diagrama se muestra la estructura que elegimos para almacenar los diferentes archivos de nuestro FileSystem. Las diferentes rutas del diagrama serían: 
    
    /DIR1
        
        -> /DIR1/File 1

        ->/DIR1/File 2

    /File 3

    /DIR2

        -> /DIR2/File 4

Se eligió una estructura en forma de matriz, donde la 1era columna representa el primer nivel de recursion, donde pueden almacenarse directorios o archivos. Las columnas siguientes representan los lugares donde pueden almacenarse los archivos dentro de un directorio en especifico. 

Por comodidad se decidio que todas las estructuras fuesen archivos, y que se indique con una flag si son FILE_T o DIR_T. Todo archivo se almacena en un struct File, 

        ```c
        struct File {
            path,
            cell_status,
            type,
            metadata,
            contenido
        }
        ```

la metadata esta representada en un struct de la forma 

        ```c
        struct Metadata {
            nombre,
            tipo,
            fecha_creacion,
            fecha_modificacion,
            fecha_acceso,
            propietario, (es el UsedID)
            mode,
            gid,
        }
        ```
        
**Persistencia**
Las celdas de la matriz se pueden encontrar en 3 estados diferentes: FREE_T, TAKEN_T, DELETED_T, que indican si la celda esta libre para ser ocupada, si esta ocupada o si en algun momento estuvo ocupada y fue borrada y pueden haber mas celdas ocupadas despues de ella. La idea es optimizar la busqueda de archivos y de espacios libres.

Nuestro FileSystem se persiste mediante un archivo en el cual se guardarán todas las celdas de nuestra matriz. Siendo la matriz n * n, se guardarán n * n lineas donde cada línea será una celda de la matriz, sin importar si es una posición borrada, libre, o un File. De esta manera, la implementación asegura que siempre se volverá a obtener exactamente la misma matriz que se tenía antes de guardarse en disco, evitando posibles errores de redirección.

En cuanto a la persistencia de cada celda, al ser una implementación estática, ya se sabe a priori el tamaño de cada atributo de los structs. Entonces, se guarda cada atributo en el orden de la declaración de los structs, con su size respectivo. De esta manera, para cargar nuevamente el fs, se lee cada línea sabiendo la cantidad de bytes que debe leer para cada atributo de los structs.


**Tests**
Las salidas de los test correspondientes son:

test_1.sh
**Testing de cat, ls, mkdir, touch, cd, >, >>**

- Creamos un `archivo.txt`
- Si hago `ls` devuelve:
    ```
    archivo.txt
    ```
- Escribo "hola soy un archivo" en `archivo.txt`
- Si hago `cat archivo.txt` devuelve:
    ```
    hola soy un archivo
    ```
- Creamos un directorio, llamado `directorio`
- Si hago `ls` devuelve:
    ```
    archivo.txt  directorio
    ```
- Creamos un `archivito.txt` adentro del `directorio`
- Estando en `directorio`, si hago `ls`:
    ```
    archivito.txt
    ```
- Escribimos "algo en el archivito" en `archivito.txt` y luego lo sobreescribimos
- Si hago `cat archivito.txt` devuelve:
    ```
    algo en el archivito
    ```
- Sobreescribimos lo anterior
- Si hago `cat archivito.txt` luego de sobreescribir devuelve:
    ```
    ya no esta mas lo anterior
    ```
- Agregamos "esta lo anterior y lo que es esto" a `archivito.txt` sin sobreescribir lo anterior
- Si hago `cat archivito.txt` luego de agregar algo devuelve:
    ```
    ya no esta mas lo anterior
    esta lo anterior y lo que es esto
    ```

test_2.sh
**Testing de cat, ls, mkdir, touch, cd, >, >>, stat, rm, rmdir**

- Creamos un `archivo2.txt`
- Si hago `ls` devuelve:
    ```
    archivo2.txt
    ```
- Verifico sus stats:
    ```
    File: archivo2.txt
    Size: 0               Blocks: 0          IO Block: 4096   regular empty file
    Device: 56h/86d Inode: 2           Links: 1
    Access: (0644/-rw-r--r--)  Uid: ( 1000/  gianni)   Gid: (    0/    root)
    Access: 2023-11-27 01:20:14.000000000 -0300
    Modify: 2023-11-27 01:20:14.000000000 -0300
    Change: 1969-12-31 21:00:00.000000000 -0300
    Birth: -
    ```
- Escribo "hola soy un archivo2" en `archivo2.txt`
- Si hago `cat archivo2.txt` devuelve:
    ```
    hola soy un archivo2
    ```
- Verifico sus stats, para ver si se cambiaron las fechas y el size:
    ```
    File: archivo2.txt
    Size: 21              Blocks: 0          IO Block: 4096   regular file
    Device: 56h/86d Inode: 2           Links: 1
    Access: (0644/-rw-r--r--)  Uid: ( 1000/  gianni)   Gid: (    0/    root)
    Access: 2023-11-27 01:20:14.000000000 -0300
    Modify: 2023-11-27 01:20:14.000000000 -0300
    Change: 1969-12-31 21:00:00.000000000 -0300
    Birth: -
    ```
- Creamos un directorio, llamado `directorio2`
- Si hago `ls` devuelve:
    ```
    archivo2.txt  directorio2
    ```
- Verifico sus stats:
    ```
    File: directorio2
    Size: 0               Blocks: 0          IO Block: 4096   directory
    Device: 56h/86d Inode: 3           Links: 2
    Access: (0755/drwxr-xr-x)  Uid: ( 1000/  gianni)   Gid: (    0/    root)
    Access: 1969-12-31 21:00:00.000000000 -0300
    Modify: 1969-12-31 21:00:00.000000000 -0300
    Change: 1969-12-31 21:00:00.000000000 -0300
    Birth: -
    ```
- Creamos un `archivito3.txt` adentro del `directorio2`
- Estando en `directorio2`, si hago `ls`:
    ```
    archivito3.txt
    ```
- Borro el `archivito3.txt`
- Si hago `ls` luego de borrar el `archivito3.txt` devuelve:
- Vuelvo al path anterior
- Elimino el `directorio2`
- Si hago `ls` luego de borrar el `directorio2` devuelve:
    ```
    archivo2.txt
    ```
- Elimino el `archivo2.txt`
- Si hago `ls` luego de borrar el `archivo2.txt` devuelve:

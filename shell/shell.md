# shell

### Búsqueda en $PATH

**¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?**

exec(3) no es una syscall sino que es una familia de funciones que funcionan como wrappers y permiten iniciar un nuevo programa en un proceso. 
exec(3) presenta diferentes variaciones que permiten diferenciar las llamadas a cada función, usando diferentes argumentos, mientras que execve(2) siempre presenta la misma llamada.

**¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?**

exec(3) podría fallar en casos de un mal comando o algún error en el path. Según el man, en caso de error se setea la variable errno (number of last error) al valor que daría si la falla ocurriese en execve(2). Algunos errores son tratados diferentes: si execve(2) devuelve un EACCES error-> permission denied error, las funciones de exec(3) seguirán buscando por el path y lanzarán error al no encontrar un file. Otro ejemplo es ENOEXEC -> no se reconoce el header del file, ahí la función trataría de ejecutar /bin/sh con el path del file y lanzará error si eso no funciona. 
En los casos de error, el valor de return de la función es -1.  

---

### Procesos en segundo plano

**Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.**

La implementación de procesos en segundo plano tiene grandes similitudes con la implementación de procesos que se ejecutan de manera convencional. La principal diferencia  radica en qué ocurre luego de que se lleva a cabo la ejecución. En un proceso estándar, se utiliza el comando "fork" para crear un proceso hijo, el cual luego utiliza "exec" para ejecutar cierto comando. Mientras tanto, el proceso padre espera a que el proceso hijo complete su tarea antes de continuar.

En cambio, cuando se trata de procesos en segundo plano, el proceso padre no espera a que el proceso hijo finalice. En su lugar, se emplea el flag "WNOHANG" en la función "wait" para evitar que esta se convierta en una operación bloqueante. Esto permite que cada vez que se ejecuta un comando, se pueda verificar si hay procesos hijos que ya han concluido, evitando así que quede un “zombie". Este método es utilizado por diversas shells, como Bash y Zsh, para gestionar la ejecución de comandos en segundo plano de manera eficiente y evitar bloqueos innecesarios.

Además para su cierre se utilizaron señales, cuando un proceso en segundo plano sale, se hace wait con el flag "WNOHANG" para que el proceso deje de ser zombie apenas sale, en vez de esperar a que se ejecute un nuevo comando.

**¿Por qué es necesario el uso de señales?**

Es necesario usar señales porque es una forma de comunicación entre procesos. En este caso se usa una señal SIGCHILD, que se genera automáticamente e informa al padre de la finalización de un proceso hijo. Esto permite manejar los procesos en background cuando estos terminan. Como las señales son asincrónicas, permiten ser manejadas en un handler, sin interrumpir el resto del funcionamiento del proceso padre. 

---

### Flujo estándar

**Investigar el significado de 2>&1, explicar cómo funciona su forma general**

- **Mostrar qué sucede con la salida de cat out.txt en el ejemplo.**

- **Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).**

El comando "2>&1" es una redirección en la cual se combina la salida de dos File Descriptors en un mismo destino. Ambos números representan fds y se redirige la salida del primero hacia el mismo destino que el segundo. En este ejemplo, esto significa que los mensajes de error se redirigen al mismo destino que la salida estándar.

En el comando "ls -C /home /noexiste >out.txt 2>&1", primero se redirige la salida estándar hacia el archivo "out.txt". Luego, se realiza una redirección de la salida de error estándar (stderr) hacia el mismo destino que la salida estándar (stdout). Esto significa que cuando se ejecuta el comando "ls" en un directorio que no existe, cualquier mensaje de error que se genere se dirigirá al mismo lugar que la salida normal. Por lo tanto, en el archivo "out.txt" se encuentra tanto la salida de error como la salida estándar combinadas en un solo archivo.

Si se corre el comando "ls -C /home /noexiste  2>&1 >out.txt" en la shell implementada, los resultados son los mismos que el comando anterior. Sin embargo, en bash, stderr no se redirige al archivo, se imprime por pantalla el error y en el archivo solamente está la salida de stdout. Dado que stderr y stdout apuntan al mismo destino después de la primera redirección, cualquier mensaje de error generado por el comando ls se muestra en la pantalla antes de que stdout se redirija al archivo.

---

### Tuberías múltiples

**Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe**

- **¿Cambia en algo?**
- **¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.**

En bash, el exit code depende del último comando que se ejecute en los pipes. La única forma de que esto cambie es seteando pipefail en nuestra shell. De esta manera, si alguno de los comandos que se ejecutan en los pipes falla, el exit code se va a arrastrar hacia el último comando.
En el siguiente ejemplo, se puede ver cómo el exit code es el del segundo comando.

`~/sisop_2023b_g10/shell $ echo hola | cd noexiste
bash: cd: noexiste: No such file or directory
~/sisop_2023b_g10/shell$ echo $?
1
`

En el siguiente ejemplo se puede ver cómo, a pesar que el primer comando falló, el exit code es 0 debido a que el segundo comando se ejecutó sin errores.

`~/sisop_2023b_g10/shell$ cd noexiste | echo hola
hola
bash: cd: noexiste: No such file or directory
~/sisop_2023b_g10/shell$ echo $?
0
`

---

### Variables de entorno temporarias

**¿Por qué es necesario hacerlo luego de la llamada a fork(2)?**

Es necesario hacerlo luego de la llamada a fork porque las variables de entorno temporales solamente deben poder establecerse y utilizarse dentro del proceso que se está ejecutando y no en las variables de entorno de la shell misma. La variable nunca se asigna en la shell, sino en dicho proceso, por lo tanto cuando se termina la ejecución del comando, la variable deja de existir.

**En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).**
- **¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.**
- **Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.**

El comportamiento no sería el mismo, debido a que en el primer caso el proceso tendría acceso a las nuevas variables de entorno establecidas con setenv sumado a las de la shell. En cambio, en el segundo caso solo se tendría disponibles las variables recibidas en el tercer argumento.

Para poder lograr que el comportamiento sea el mismo, se podría pasar por parámetro a exec un arreglo que contenga las variables de entorno de la shell (extern char** environ) y a ellas sumarle las nuevas variables temporales.

---

### Pseudo-variables

**Investigar al menos otras tres variables mágicas estándar, y describir su propósito.**
- **Incluir un ejemplo de su uso en bash (u otra terminal similar).**

`$$: Su valor es el PID de la shell actual.
~/sisop_2023b_g10/shell$ echo $$
19429
~/sisop_2023b_g10/shell$ ps
	PID TTY      	TIME CMD
  15225 pts/0	00:00:00 bash
  19429 pts/0	00:00:00 bash
  19457 pts/0	00:00:00 ps`

`$0: El valor es el nombre del script actual.
~/sisop_2023b_g10/shell$ echo $0
bash`


`$!: Su valor es el PID del ultimo comando ejecutado en segundo plano.
~/sisop_2023b_g10/shell$ sleep 2 &
[1] 19477
$ echo $!
19477`


---

### Comandos built-in

**¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)**

El comando cd no podría implementarse sin ser built-in, porque que sea built-in significa que el comando no lo está ejecutando un proceso hijo de la shell, sino que lo ejecuta el proceso padre. Entonces, si se implementara el comando cd sin que sea built-in, no podríamos cambiar la dirección en donde nos ubicamos en ese momento, porque el que cambiaría de dirección sería el proceso hijo y no el proceso padre. Con pwd sería distinto, porque lo que hace es imprimir la ubicación del directorio actual, lo cual no requiere que sea en el proceso padre. 

Sin embargo, implementar comandos en built-in nos permite reducir el riesgo de que se produzca un error al crear un proceso hijo y a su vez, se vuelve más eficiente y rápido.
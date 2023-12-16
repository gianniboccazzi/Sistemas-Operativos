echo "testing de cat, ls, mkdir, touch, cd, >, >>, stat, rm, rmdir"

echo ""

cd prueba

echo "creamos un archivo2.txt"

touch archivo2.txt

echo "si hago ls devuelve:"

ls

echo "verifico sus stats"

stat archivo2.txt

echo "escribo "hola soy un archivo2" en archivo2.txt"

echo "hola soy un archivo2" > archivo2.txt

echo "si hago cat archivo2.txt devuelve:"

cat archivo2.txt

echo "verifico sus stats, para ver si se cambiaron las fechas y el size"

stat archivo2.txt

echo "creamos un directorio, llamado directorio2"

mkdir directorio2

echo "si hago ls devuelve:"

ls

echo "verifico sus stats"

stat directorio2

echo "creamos un archivito3.txt adentro del directorio2"

cd directorio2

touch archivito3.txt

echo "estando en directorio2, si hago ls:"

ls

echo "borro el archivito3.txt"

rm archivito3.txt

echo "si hago ls luego de borrar el archivito3.txt devuelve:"

ls

echo "vuelvo al path anterior"

cd ..

echo "elimino el directorio2"

rmdir directorio2

echo "si hago ls luego de borrar el directorio2 devuelve:"

ls

echo "elimino el archivo2.txt"

rm archivo2.txt

echo "si hago ls luego de borrar el archivo2.txt devuelve:"

ls
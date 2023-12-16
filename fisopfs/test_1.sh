echo "testing de cat, ls, mkdir, touch, cd, >, >>"

echo ""

cd prueba

echo "creamos un archivo.txt"

touch archivo.txt

echo "si hago ls devuelve:"

ls

echo "escribo "hola soy un archivo" en archivo.txt"

echo "hola soy un archivo" > archivo.txt

echo "si hago cat archivo.txt devuelve:"

cat archivo.txt

echo "creamos un directorio, llamado directorio"

mkdir directorio

echo "si hago ls devuelve:"

ls

echo "creamos un archivito.txt adentro del directorio"

cd directorio


touch archivito.txt

echo "estando en directorio, si hago ls:"

ls

echo "escribimos "algo en archivito" y luego lo sobreescribimos"

echo "algo en el archivito" > archivito.txt

echo "si hago cat archivito.txt devuelve:"

cat archivito.txt

echo "sobreescribimos lo anterior"

echo "ya no esta mas lo anterior" > archivito.txt

echo "si hago cat archivito.txt luego de sobreescribir devuelve:"

cat archivito.txt

echo "agregamos algo a archivito sin sobreescribir lo anterior"

echo "esta lo anterior y lo que es esto" >> archivito.txt

echo "si hago cat archivito.txt luego de agregar algo devuelve:"

cat archivito.txt


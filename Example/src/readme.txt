
En fat32.h cambiamos el END_OF_CLUSTER a 0xfffffff8 porque no llegaba a leer hasta el final (0xffffffff).

El programa crea solo el archivo "DATALOG.txt", las veces que probamos creandolo no funciono.
a
Cuando escribe arranca escribiendo desde la ultima posicion que escribio anteriormente.
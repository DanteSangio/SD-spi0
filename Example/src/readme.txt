La version esta implementada con SPI (SSP0). Tenemos que pasarla a SSP1.

En fat32.h cambiamos el END_OF_CLUSTER a 0xfffffff8 porque no llegaba a leer hasta el final (0xffffffff).

El programa crea solo el archivo "DATALOG.txt", las veces que probamos creandolo no funciono.

Cuando escribe arranca escribiendo desde la ultima posicion que escribio anteriormente.

Al alimentar con 3V3 no detecta el formato ded la SD, DEBE ALIMENTARSE CON 5V

Hasta no colocar el EOF no guarda el archivo, solución usada, se coloca el EOF tras cada "linea"
se cierra el archivo y se vuelve a abrir nuevamente.
# Proyecto 2

Del curso **Sistemas Operativos Avanzados**, de la Maestría de Ciencias de la Computación del Tecnológico de Costa Rica.

Profesor:
  Dr. Francisco Torres Rojas

Integrantes:
  - Nicole Carvajal Barboza 2017098785
  - Rubén González Villanueva 2017118764
  - Edisson López 2013103311
  - Otto Mena Kikut 2021582208
  - Cristina Soto Rojas 2021582215

# Creador

Para compilar utilizar el siguiente comando:

 ```gcc creador.c -o creador -lpthread -lrt -lm -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic```
 
 Para ejecutar el programa:

 ```./creador -n nombre -m media```

 Los parámetros disponibles son los siguientes:

 - n: Es el nombre del buffer y se pone de la siguiente manera: ```-n nombre```
 - m: Es la media de tiempo a utilizar y se pone de la siguiente manera: ```-m media```

# Productor

Para compilar utilizar el siguiente comando:

 ```gcc productor.c -o productor -lm -lpthread -lrt ```
 
 Para ejecutar el programa:

 ```./productor -n nombre -m media -p pid```

 Los parámetros disponibles son los siguientes y el único obligatorio es el p:

 - n: Es el nombre del buffer y se pone de la siguiente manera: ```-n nombre```
 - m: Es la media de tiempo a utilizar y se pone de la siguiente manera: ```-m media```
 - p: Es el pid del creador y se pone de la siguiente manera: ```-p creador```

# Consumidor

Para compilar utilizar el siguiente comando:

 ```gcc consumidor.c -o consumidor -lm -lpthread -lrt ```
 
 Para ejecutar el programa:

 ```./consumidor -n nombre -m media -p pid```

 Los parámetros disponibles son los siguientes y el único obligatorio es el p:

 - n: Es el nombre del buffer y se pone de la siguiente manera: ```-n nombre```
 - m: Es la media de tiempo a utilizar y se pone de la siguiente manera: ```-m media```
 - p: Es el pid del creador y se pone de la siguiente manera: ```-p creador```

# Finalizador

Para compilar utilizar el siguiente comando:

 ```gcc finalizador.c -o finalizador -lm -lpthread -lrt ```
 
 Para ejecutar el programa:

 ```./finalizador -n nombre  -p pid```

 Los parámetros disponibles son los siguientes y el único obligatorio es el p:

 - n: Es el nombre del buffer y se pone de la siguiente manera: ```-n nombre```
 - p: Es el pid del creador y se pone de la siguiente manera: ```-p creador```


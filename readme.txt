Intregantes: Daniela Aguilar - Fabio Calderón - Felipe Olivares - Juleisy Porras

Explicación del merge:

El código empieza con una estructura merge_list que tiene el puntero a la lista y el tamaño de la lista. 
Después hay otra estructura merge_list_aux que contiene la lista y un índice del inicio y del final de esta, y el puntero a la barrera.

Luego, hay una función merge_sort_aux que crea un arreglo del tamaño que hay entre los índices de inicio y final. 
En esa función se hace el merge del subarreglo original a un nuevo subarreglo y luego los copia al arreglo original. Después de esto espera, hace el wait.

La función principal merge_sort crea un arreglo de barreras dependiendo de cuántas iteraciones va a necesitar hacer.
Hay un caso especial de que si es impar ordene los últimos 3 números. 
Luego hay un ciclo de que mientras ocupe hacer más de un thread o subdividir la lista en más de una parte, 
pues al inicio la divide entre n/2 partes y manda a la función auxiliar cada una de esas partes y espera con el wait del Barrier a que termine.
Después divide la cantidad de partes entre 2 y ahora estas partes son más grandes y están ordenadas. 
Repite el proceso hasta que partes sea igual a 1. Luego hace un merge de todo el arreglo.

Por último, borra las barreras y ya termina.
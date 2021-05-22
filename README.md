# Estudio de una pila Alcalina AA.

Tras conversaciones con compañeros del grupo de telegram **"Wide Maker Xperiences"** coordinado por **@Llorens** surge la sugerencia de crear un sistema para medir las características de una pila Alcalina con el fin de sacar conclusiones que nos puedan llevar a realizar un sistema que pueda recargar pilas Alcalinas AA ( no recargables ) un cierto número de veces con seguridad.

Esto fue a raíz de que **@Llorens** ya ha recargado pilas Alcalinas AA no recargables con resultado satisfactorio, claro está con peor rendimiento que unas pilas recargables pero que le permitieron usar dichas pilas Alcalinas muchas veces más ( unas 20 veces o más ), cosa que me parece bastante increíble.

A raíz de todo esto, como primera parte se realiza un pequeño programa usando un ESP32 para medir la descarga de una pila y poder empezar a sacar conclusiones. Como primer punto está, conocer a tu enemigo.

Se usará uno de los puertos analógicos para tomar la tensión de descarga de la pila sobre una resistencia de carga ( 10 Ohmios/ 27 Ohmios ... ) y sacar valores de corriente, miliamperios hora, potencia ( miliWatios ), Julios...

Weather Station
===============

Per intenderci: non è proprio una weather station che fornisce previsioni!
Accumula i dati da più punti di lettura e li mostra. Per adesso da un solo punto. :-)

Ho fatto il porting di una libreria per nRF24L01 e li ho usati su un'Aria per ricevere i dati di temperatura e presssione, 
trasmessi da una Arduino. I sensori che ho usato sull'Arduino sono: BMP180, DHT11.

Il programma che legge i dati, li memorizza subito in un db creato in sqlite. 
Nel folder c'è il makefile per compilare il tutto con le librerie di sviluppo che, se non ricordo male, devono essere scaricate.

Per i grafici ho usato la libreria Chart.js

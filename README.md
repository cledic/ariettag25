ariettag25
==========

AriettaG25 è tra le ultime board nate in casa ACMES Systems [1].<br>
E' una board molto piccola e <b>senza ethernet</b>. Ma in 2,5x5,3 cm si possono avere: un modulo WiFi[4][5] ed una
strip di 40pin a passo 2,54mm x i GPIO [2].

Il processore è lo stesso della board AriaG25: un Atmel AT91SAM9G25 SoC (ARM9 @ 400Mhz)[3].<br>
L'I/O riportato sulla strip comprende:<br>
<ul>
<li><b>3 USB</b> ( 2 se si usa il WiFi)</li>
<li><b>2 I2C</b></li>
<li><b>3 SPI</b></li>
<li><b>3 UART</b></li>
</ul>
<br>
Non tutte le perifieriche si possono avere <i>disponibili</i>, ma vi assicuro che è possibile avere contemporaneamente:
<ul>
<li><b>2 I2C</b></li>
<li><b>1 SPI</b> (CS0)</li>
<li><b>2 UART</b></li>
</ul>
Non male vero? Provare per credere! [2]

[1] http://acmesystems.it<br>
[2] http://acmesystems.it/pinout_arietta<br>
[3] http://www.atmel.com/devices/SAM9G25.aspx<br>
[4] http://acmesystems.it/arietta_wifi<br>
[5] http://acmesystems.it/WIFI-2

I vari Progetti
===============
Nel repository <b>ariettag25</b> ho inserito alcune progettini che ho fatto con AriettaG25.<br>
Molte librerie non sono mie, ma ne ho fatto il porting per Arietta da librerie già esistenti e quindi i meriti devono essere attribuiti ai rispettivi creatori.<br>

Io, quasi sempre, ne ho fatto il porting dal C++, ed ho inserito il codice per gestire la I2C o la SPI in Linux.<br>
Si è trattato quindi di costruire le primitive di <i>read</i> e <i>write</i> e di impostare l'apertura del <i>device driver</i>

Il progetto più consistente riguarda un lettore di news da Internet che usa ovviamente AriettaG25 ed un LCD tipo Nokia6110; è sotto il folder <b>ariettaNews</b>. Ho usato molto la libreria ImageMagick per convertire il testo in immagini e per montare immagini scaricate da Internet con altro testo. Lo schermo non è visto come framebuffer ma pilotato come device SPI.

Nelle altre direcotry ci sono programmi che sono prove di collegamento ad alcuni device di cui: un accelerometro ADXL345, un altimetro/barometro BMP180, un LCD 320x240 MI0283QT.

Nella directory <b>weather_station</b> invece c'è un progettino che ho realizzato con Arduino ed una board Terra. Si tratta appunto di una stazione meteo composta da più unità di lettura, realizzate con Arduino il sensore BMP180 e DH11, e la board Terra a ricevere i dati. Come unità di trasmissione e ricezione ho usato i dispositivi RF: nRF24L01 di cui ho fatto il porting di una libreria per la board Terra. 

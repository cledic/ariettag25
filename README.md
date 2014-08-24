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
Io, quasi sempre, ne ho fatto ilporting dal C++, ed ho inserito il codice per gestire la I2C o la SPI in Linux.<br>



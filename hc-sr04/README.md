 HC-SR04
==========

Ho modificato il driver realizzato da Tanzilli, per il sensore HC-SR04[1]<br>
La modifica riguarda la possibilità di indicare il pin di trigger e di echo, al momento dell'esecuzione, passandoli come parametri.<br>
Inoltre, al file <b>/sys/class/hcsr04/value</b> da cui leggere i dati, ho aggiunto il file <b>/sys/class/hcsr04/config</b> per
leggere su quali pin il sensore è configurato.<br>

I parametri sono due:<br>
<b>insmod hcsr04.ko pecho=[<<kernel.ID>> ptrg=<<kernel.ID>>]</b><br>

In caso non si passi nessun parametro, a default valgono i valori:<br>
pecho=95 ptrg=91<br>

[1] http://acmesystems.it/HC-SR04

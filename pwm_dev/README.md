 PWM Library
=============

Questa libreria gestisce il PWM di Arietta da sysfs.<br>

Un utilizzo di questa libreria si può trovare nel sorgente del programma pwm_tst<br>

Ad esempio, per eseguire il posizionamento al centro dei due servi eseguire:<br>
<b>./pwm_tst 0 16650000 750000</b><br>
<b>./pwm_tst 1 16650000 750000</b><br>
<br>
Estremo orizzontale<br>
<b>./pwm_tst 1 16650000 1200000</b><br>
<b>./pwm_tst 1 16650000 250000</b><br>
<br>
Estremo verticale<br>
<b>./pwm_tst 0 16650000 1000000</b><br>
<b>./pwm_tst 0 16650000 250000</b><br>

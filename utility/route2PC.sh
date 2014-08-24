# Uso questo script per impostare la rotta di default sull'IP del PC
# collegato all'Arietta in USB. Per andare in Internet c'è bisogno di abilitare sul PC
# anche la condivisione Internet sull'interfaccia usata.
#
ip route delete
ip route add default via 192.168.10.20

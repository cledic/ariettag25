# Uso questo script per passare la rotta di default sul router WiFi
# In questo caso il router WiFi ha indirizzo IP: 192.168.1.1
#
ip route delete
ip route add default via 192.168.1.1

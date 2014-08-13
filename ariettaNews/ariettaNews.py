from time import sleep
import datetime
import string
import os

os.system("/usr/bin/convert -background lightblue  -fill blue -pointsize 21 -size 131x131 -font URW-Palladio-Bold-Italic  -gravity center caption:\"Welcome to AriettaNews. Please Wait...\" -depth 8 -flip /tmp/tmpdata/welcome.rgb")
os.system("./nokiaLCD -s /tmp/tmpdata/welcome.rgb")

os.system("./sync_time.sh")
os.system("/usr/bin/python ./weather_reader.py")
os.system("/usr/bin/python ./ansa_reader.py")

while(1):
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-1.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-2.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-3.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-4.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-5.rgb")
  sleep(60*1)

  os.system("./nokiaLCD -s /tmp/tmpdata/today_weather.rgb")
  sleep(60*2)
  os.system("./nokiaLCD -s /tmp/tmpdata/tomorrow_weather.rgb")
  sleep(60*2)
  os.system("./nokiaLCD -s /tmp/tmpdata/twodayslater_weather.rgb")
  sleep(60*2)

  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-6.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-7.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-8.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-9.rgb")
  sleep(60*1)
  os.system("./nokiaLCD -s /tmp/tmpdata/ansa_news-10.rgb")
  sleep(60*1)

  os.system("./nokiaLCD -s /tmp/tmpdata/today_weather.rgb")
  sleep(60*2)
  os.system("./nokiaLCD -s /tmp/tmpdata/tomorrow_weather.rgb")
  sleep(60*2)
  os.system("./nokiaLCD -s /tmp/tmpdata/twodayslater_weather.rgb")
  sleep(60*2)

  os.system("/usr/bin/python ./weather_reader.py")
  os.system("/usr/bin/python ./ansa_reader.py")

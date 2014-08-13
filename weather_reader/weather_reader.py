# -*- coding: utf-8 -*-
from urllib import urlencode
import urllib2
import urllib
from lxml import etree
from lxml import objectify
import string
import os

FREE_API_KEY = ""
PREMIUM_API_KEY = ""

_keytype = "free"
_key = FREE_API_KEY
_myIP = ""
_location = ""

_filepath="/tmp/tmpdata/"

def myIP():
    idx1 = idx2 = 0
    myip=""
    global _myIP

    try:
        dyndns = urllib2.urlopen("http://checkip.dyndns.org/")
        myip = dyndns.read()

        idx1 = string.find( myip, "Address: ", idx1)
        idx1 = idx1 + "Address: ".__len__()
        idx2 = string.find( myip, "</body>", idx1)
        _myIP = myip[idx1:idx2]

        return True
    except urllib2.URLError:
        return False


def internet_on():
    """fast test by trying one of google IPs"""
    try:
        #unfortunately sometimes google is unstable in China
        urllib2.urlopen('http://www.baidu.com',timeout=3)
        return True
    except urllib2.URLError:
        try:
            urllib2.urlopen('http://www.google.com',timeout=3)
            return True
        except urllib2.URLError:
            return False

def setKeyType(keytype="free"):
    """ keytype either "free" or "premium", set the key if it exists"""
    global _key, _keytype, FREE_API_KEY, PREMIUM_API_KEY

    keytype = keytype.lower()
    if keytype in ("f", "fr", "free"):
        _keytype = "free"
        if FREE_API_KEY == "":
            print "Please set FREE_API_KEY"
            return False
        else:
            _key = FREE_API_KEY
            return True
    elif keytype.startswith("prem") or keytype in ("nonfree", "non-free"):
        _keytype = "premium"
        if PREMIUM_API_KEY == "":
            print "Please set PREMIUM_API_KEY"
            return False
        else:
            _key = PREMIUM_API_KEY
            return True
    else:
        print "invalid keytype", keytype
        return False

def setKey(key, keytype):
    """ if keytype is valid, save a copy of key accordingly
        and check if the key is valid """
    global _key, _keytype, FREE_API_KEY, PREMIUM_API_KEY

    keytype = keytype.lower()
    if keytype in ("f", "fr", "free"):
        keytype = "free"
        FREE_API_KEY = key
    elif keytype.startswith("prem") or keytype in ("nonfree", "non-free"):
        keytype = "premium"
        PREMIUM_API_KEY = key
    else:
        print "invalid keytype", keytype
        return

    oldkey = _key
    oldkeytype = _keytype
    _key = key
    _keytype = keytype

    w = LocalWeather("london")
    # w.data != False rather than w.data to suppress Python 2.7 FurtureWarning:
    # "The behavior of this method will change in future versions...."
    if w is not None and hasattr(w, 'data') and w.data != False:
        return True
    else:
        print "The key is not valid."
        _key = oldkey
        _keytype = oldkeytype
        return False

class WWOAPI(object):
    """ The generic API interface """
    def __init__(self, q, **keywords):
        """ query keyword is always required for all APIs """
        if _key == "":
            print "Please set key using setKey(key, keytype)"
        else:
            if internet_on():
                self.setApiEndPoint(_keytype == "free")
                self._callAPI(q=q, key=_key, **keywords)
            else:
                print "Internet connection not available."

    def setApiEndPoint(self, freeAPI):
        if freeAPI:
            self.apiEndPoint = self.FREE_API_ENDPOINT
        else:
            self.apiEndPoint = self.PREMIUM_API_ENDPOINT

    def _callAPI(self, **keywords):
        for arg in keywords:
            if keywords[arg] != None:
                if keywords[arg] in ("No", "NO", "None"):
                    keywords[arg] = "no"
                elif keywords[arg] in ("Yes", "YES", "Yeah"):
                    keywords[arg] = "yes"
            else:
                del keywords[arg]

        url = self.apiEndPoint + "?" + urlencode(keywords)
        try:
            response = urllib2.urlopen(url).read()
        except urllib2.URLError:
            print "something wrong with the API server"
            return

        # if the key is invalid it redirects to another web page
        if response.startswith("<?xml "):
            self.data = objectify.fromstring(response)
            if self.data is not None and hasattr(self.data, 'error') and self.data.error != False:
                print self.data.error.msg
                self.data = False
        else:
            self.data = False

class LocalWeather(WWOAPI):
    FREE_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/weather.ashx"
    PREMIUM_API_ENDPOINT = "http://api.worldweatheronline.com/premium/v1/premium-weather-V2.ashx"

    def __init__(self, q, num_of_days=1, **keywords):
        """ q and num_of_days are required. max 7 days for free and 15 days for premium """
        super(LocalWeather, self).__init__(
            q, num_of_days=num_of_days, **keywords)

class LocationSearch(WWOAPI):
    FREE_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/search.ashx"
    PREMIUM_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/search.ashx"

class MarineWeather(WWOAPI):
    FREE_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/marine.ashx"
    PREMIUM_API_ENDPOINT = "http://api.worldweatheronline.com/premium/v1/marine.ashx"

class PastWeather(WWOAPI):
    FREE_API_ENDPOINT = "http://api.worldweatheronline.com/premium/v1/past-weather.ashx"
    PREMIUM_API_ENDPOINT = "http://api.worldweatheronline.com/premium/v1/past-weather.ashx"

    def __init__(self, q, date=None, **keywords):
        """ q and date are required for free API. sometimes date is optional for premium API """
        super(PastWeather, self).__init__(
            q, date=date, **keywords)

class TimeZone(WWOAPI):
    FREE_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/tz.ashx"
    PREMIUM_API_ENDPOINT = "http://api.worldweatheronline.com/free/v1/tz.ashx"

if __name__ == "__main__":

    if internet_on() :
        if myIP():
           _location = _myIP
        else:
           _location = "Rome"
        #
        if setKey("mfm43jxm94wdkw24ntkghz6s", "free"):
            # weather = LocalWeather("87.22.76.169")
            print "Location: "+_location
            weather = LocalWeather(_location)
            # Scrivo il file con le info del meteo corrente
            weather_condition = "Temp     : "+str(weather.data.current_condition.temp_C)+"C \n"
            weather_condition = weather_condition + "Descr    : "+weather.data.current_condition.weatherDesc+"\n"
            weather_condition = weather_condition + "Humidity : "+str(weather.data.current_condition.humidity)+"%\n"
            weather_condition = weather_condition + "Pressure : "+str(weather.data.current_condition.pressure)+"hP\n"
            weather_condition = weather_condition + "Precip   : "+str(weather.data.current_condition.precipMM)+"mm\n"
            output = open(_filepath+"curr_weather.txt","wb+")
            output.write( weather_condition)
            output.close()
            # Scarico l'icona del meteo corrente
            URLweather = weather.data.current_condition.weatherIconUrl
            os.system("wget -q -O "+_filepath+"curr_weather.png "+URLweather)

            #print
            #print objectify.dump(weather.data.current_condition)

            #print
            weather = LocalWeather(_location, num_of_days=3)

            #
            today = weather.data.weather[0]
            tomorrow = weather.data.weather[1]
            twodayslater = weather.data.weather[2]
            #
            weather_condition = "Date     : "+today.date+"\n"
            weather_condition = weather_condition + "TMax : "+str(today.tempMaxC)+"C "
            weather_condition = weather_condition + "TMin : "+str(today.tempMinC)+"C\n"
            weather_condition = weather_condition + "Descr : "+today.weatherDesc+"\n"
            weather_condition = weather_condition + "Precip : "+str(today.precipMM)+"mm\n"
            output = open(_filepath+"today_weather.txt","wb+")
            output.write( weather_condition)
            output.close()
            # Scarico l'icona del meteo di oggi
            URLweather = today.weatherIconUrl
            os.system("wget -q -O "+_filepath+"today_weather.png "+URLweather)
            #
            weather_condition = "Date     : "+tomorrow.date+"\n"
            weather_condition = weather_condition + "TMax : "+str(tomorrow.tempMaxC)+"C "
            weather_condition = weather_condition + "TMin : "+str(tomorrow.tempMinC)+"C\n"
            weather_condition = weather_condition + "Descr : "+tomorrow.weatherDesc+"\n"
            weather_condition = weather_condition + "Precip : "+str(tomorrow.precipMM)+"mm\n"
            output = open(_filepath+"tomorrow_weather.txt","wb+")
            output.write( weather_condition)
            output.close()
            # Scarico l'icona del meteo di domani
            URLweather = tomorrow.weatherIconUrl
            os.system("wget -q -O "+_filepath+"tomorrow_weather.png "+URLweather)
            #
            weather_condition = "Date     : "+twodayslater.date+"\n"
            weather_condition = weather_condition + "TMax : "+str(twodayslater.tempMaxC)+"C "
            weather_condition = weather_condition + "TMin : "+str(twodayslater.tempMinC)+"C\n"
            weather_condition = weather_condition + "Descr : "+twodayslater.weatherDesc+"\n"
            weather_condition = weather_condition + "Precip : "+str(twodayslater.precipMM)+"mm\n"
            output = open(_filepath+"twodayslater_weather.txt","wb+")
            output.write( weather_condition)
            output.close()
            # Scarico l'icona del meteo dopodomani
            URLweather = twodayslater.weatherIconUrl
            os.system("wget -q -O "+_filepath+"twodayslater_weather.png "+URLweather)
            
            os.system("/root/weather_reader/mk_weather_img.sh "+_filepath+"today_weather.txt "+_filepath+"today_weather.png "+_filepath+"today_weather.rgb")
            os.system("/root/weather_reader/mk_weather_img.sh "+_filepath+"tomorrow_weather.txt "+_filepath+"tomorrow_weather.png "+_filepath+"tomorrow_weather.rgb")
            os.system("/root/weather_reader/mk_weather_img.sh "+_filepath+"twodayslater_weather.txt "+_filepath+"twodayslater_weather.png "+_filepath+"twodayslater_weather.rgb")

            #print
            #print objectify.dump(tomorrow)

            #print



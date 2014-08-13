import string
import feedparser
import os

while 1:
  print'.',
  d = feedparser.parse('http://ansa.feedsportal.com/c/34225/f/621689/index.rss')
  idx = len(d['items'])
  if idx > 0:
    break

tmp_path="/tmp/tmpdata/"

if ( idx < 10):
  myidx=idx
else:
  myidx=10
  
while( myidx):
  idx1 = 0
  e = d['items'][ myidx]

  titolo=e['title']

  descrizione=e['description']
  idx1 = string.find( descrizione, "<img ", idx1)
  descrizione = descrizione[0:idx1]

  #print e['title']
  #print e['description']


  myfile=tmp_path+"ansa_news-"+str(myidx)+".txt"
  myimg=tmp_path+"ansa_news-"+str(myidx)+".rgb"

  fansa = open(myfile, 'wb+')
  titolo_out = titolo.encode('utf-8', 'replace')
  #titolo_out = '{:<112}'.format(titolo_out)
  fansa.write( " "+titolo_out+"\n")

  descrizione_out = descrizione.encode('utf-8', 'replace')
  #descrizione_out = '{:<224}'.format(descrizione_out)
  fansa.write( " "+descrizione_out+"\n")
  fansa.close()

  os.system("/usr/bin/convert -background lightblue  -fill blue -pointsize 14 -size 131x131   caption:@"+myfile+" -depth 8 -flip "+myimg)

  print titolo_out
  print " "+descrizione_out+"\n"

  myidx=myidx-1


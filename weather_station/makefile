#################################################
# MAKEFILE for nRF24L01 library			#
# (c) 20131115 Clemente di Caprio		#
#                                               #
#################################################

CC=gcc

#
# Compiler options
OPTIMIZE=-O3
GDBDEBUG=-g
CFLAGS=-c -Wall
CFLAGS+=$(OPTIMIZE)
CFLAGS+=$(GDBDEBUG)

#
# Library 
SQLITE_LIB=-lsqlite3
#
LDFLAGS=$(SQLITE_LIB)

#
# Source path
NRF24_FILE=./nRF24L01_lib/
GPIO_FILE=./gpio_lib/
#
# Include
INCPATHS=./inc	\
	  $(NRF24_FILE)/inc \
	  $(GPIO_FILE)/inc
INCLUDES     = $(addprefix -I ,$(INCPATHS))

#
SOURCES=./radio2db.c 	\
	$(NRF24_FILE)/src/nRF24L01P.c 	\
	$(GPIO_FILE)/src/gpio_lib.c
		
OBJECTS=$(SOURCES:%.c=%.o)
EXECUTABLE=radio2db

MSGECHO = echo
APP_VER = W.I.P
REMOVAL = rm
REMOVE	= $(REMOVAL) -f

all: gccversion build 
build: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	$(REMOVE) $(EXECUTABLE)
	$(REMOVE) *.o
	$(REMOVE) $(wildcard $(NRF24_FILE)/src/*.o)
	$(REMOVE) $(wildcard $(GPIO_FILE)/src/*.o)

gccversion : 
	@$(CC) --version
	@$(MSGECHO) 

.PHONY : all gccversion


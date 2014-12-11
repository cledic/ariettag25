#################################################
# MAKEFILE for PWM lib                          #
# (c) 20131115 Clemente di Caprio				#
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
MATH_LIB=-lm
#
LDFLAGS=$(MATH_LIB)

#
# Include
INCPATHS=./inc				\
         ../../inc
INCLUDES     = $(addprefix -I ,$(INCPATHS))

#
SOURCES=./src/main.c 					\
		./src/PWM_lib.c
		
OBJECTS=$(SOURCES:%.c=%.o)
EXECUTABLE=pwm_tst

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
	$(REMOVE) $(wildcard ./src/*.o)

gccversion : 
	@$(CC) --version
	@$(MSGECHO) 

.PHONY : all gccversion

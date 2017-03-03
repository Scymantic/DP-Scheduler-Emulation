###############################
# Makefile
# Author: Adian Barberis
# Original Template: Wyatt Emery
# Date: FEB 22, 2017
#
#
# 
# this will compile and generate executables from Commander.cpp ProcessManager.cpp 
# 
###############################

CC=g++ -std=c++0x
CFLAGS=-ggdb -Wall
PFUNCS=ProcessFunctions.h ProcessFunctions.cpp

.PHONY: clean

all: Commander ProcessManager

Commander: Commander.cpp
	${CC} ${CFLAGS} Commander.cpp -o commander

ProcessManager: ProcessManager.cpp ${ProcessManager}
	${CC} ${CFLAGS} ProcessManager.cpp ${ProcessManager} -o procMan

clean: 
	/bin/rm -f *.o commander processManager

CFLAGS=$(shell root-config --cflags)
CFLAGS+=-Werror -g -O0 -DDEBUG -pthread -fpermissive -DLINUX  -lrt  -lCAENVME
LIBS=$(shell root-config --libs)
CFLAGS+=-I./include 

frontend:frontend.C src/*.c
	g++ $(CFLAGS) $(LIBS) $^ -o $@ 

.PHONY: clean install

install:
	cp frontend $(HOME)/bin
	cp app.py $(HOME)/bin/jcon

clean::
	rm frontend

CFLAGS=$(shell root-config --cflags)
CFLAGS+=-Werror -g -O0 -DDEBUG -pthread -fpermissive -DLINUX  -lrt  -lCAENVME
LIBS=$(shell root-config --libs)
CFLAGS+=-I./include 

frontend:frontend.C src/*.c include/*.h
	g++ $(CFLAGS) $(LIBS) $^ -o $@ 

.PHONY: clean install

install:
	cp frontend $(HOME)/dev/bin
	cp app.py $(HOME)/dev/bin/jcon

clean::
	rm frontend

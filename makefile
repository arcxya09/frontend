CFLAGS=$(shell root-config --cflags)
CFLAGS+=-Werror -g -O0 -DDEBUG -pthread -fpermissive -DLINUX  -lrt  -lCAENVME
LIBS=$(shell root-config --libs)
CFLAGS+=-I.

frontend:frontend.c CAENV2718.c cJSON.c madc.c v977.c v830.c v1190.c cvt_V1190.c cvt_board_commons.c v1190.h v830.h madc.h
	g++ $(CFLAGS) $(LIBS) $^ -o $@ 

.PHONY: clean install

install:
	cp frontend $(HOME)/dev/bin
	cp app.py $(HOME)/dev/bin/jcon

clean::
	rm frontend

CFLAGS=$(shell root-config --cflags)
CFLAGS+=-Werror -g -O0 -DDEBUG -pthread -fpermissive -DLINUX  -lrt  -lCAENVME
LIBS=$(shell root-config --libs) -lcjson
CFLAGS+=-I.

frontend:frontend.C CAENV2718.c madc.c v977.c v830.c v1190.c cvt_V1190.c cvt_board_commons.c v1190.h v830.h madc.h
	g++ $(CFLAGS) $(LIBS) $^ -o $@ 

.PHONY: clean install

install:
	cp frontend ../bin

clean::
	rm frontend

IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib
LIBs=-lpthread libpcap-1.7.3/libpcap.a

#_DEPS = hellomake.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hashtable.o hashTableFlow.o mt_hashtable_kFlowSrc_vInt.o mt_hashtable_kFlowSrc_vFloat.o mt_hashtable_kFlow_vFloat.o mt_hashtable_kFlow_vInt.o mt_hashtable_kFlow_vLinklist.o \
	createFIFOFiles.o flow.o get_mininet_host_name.o multi_write_one_read_fifo.o senderFIFOsManager.o time_library.o debug_output.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) ${LIBs}

libpublic.a: $(OBJ)
	#gcc -o $@ $^ $(CFLAGS)
	ar  rcs $@ $^

.PHONY: clean

clean:
	rm -f libpublic.a $(ODIR)/*.o *~ core $(INCDIR)/*~ 

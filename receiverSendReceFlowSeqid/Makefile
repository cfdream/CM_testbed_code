CC=gcc

ODIR=obj
LIBs=../public_lib/libpcap-1.7.3/libpcap.a ../public_lib/libpublic.a

_OBJ = receiverSendReceFlowSeqid.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

receiverSendReceFlowSeqid: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) ${LIBs}

.PHONY: clean

clean:
	rm -f receiverSendReceFlowSeqid $(ODIR)/*

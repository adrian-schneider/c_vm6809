CFLAGS = -W\#pragma-messages
CC = cc $(CFLAGS)

vm6809:		main.c vm6809.o vmmemory.o vmtermio.o vmmonitor.o os.o
	$(CC) main.c vm6809.o vmmemory.o vmtermio.o vmmonitor.o os.o -o vm6809

os.o:		os.h os.c
	$(CC) -c os.c -o os.o

vm6809.o:	vm6809.h vm6809.c
	$(CC) -c vm6809.c -o vm6809.o

vmmemory.o:	vmmemory.h vmmemory.c
	$(CC) -c vmmemory.c -o vmmemory.o

vmtermio.o:	vmtermio.h vmtermio.c
	$(CC) -c vmtermio.c -o vmtermio.o

vmmonitor.o:	vmmonitor.h vmmonitor.c
	$(CC) -c vmmonitor.c -o vmmonitor.o

clean:
	rm -f *.o
	rm -f vm6809


vm6809:		main.c vm6809.o vmmemory.o vmtermio.o vmmonitor.o os.o
	cc main.c vm6809.o vmmemory.o vmtermio.o vmmonitor.o os.o -o vm6809

os.o:		os.h os.c
	cc -c os.c -o os.o

vm6809.o:	vm6809.h vm6809.c
	cc -c vm6809.c -o vm6809.o

vmmemory.o:	vmmemory.h vmmemory.c
	cc -c vmmemory.c -o vmmemory.o

vmtermio.o:	vmtermio.h vmtermio.c
	cc -c vmtermio.c -o vmtermio.o

vmmonitor.o:	vmmonitor.h vmmonitor.c
	cc -c vmmonitor.c -o vmmonitor.o

clean:
	rm -f *.o
	rm -f vm6809

include $(ROOTDIR)/Makefile.inc

SOURCES=pb_common.c pb_encode.c pb_decode.c interface.cc server.cc

LIBNAME=pb

all:
	$(CPP) -c $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(SOURCES) -lpthread
	rm -f $(LIBNAME).a
	$(AR) -cvq $(LIBNAME).a *.o
	cp $(LIBNAME).a $(LIB)/$(LIBNAME).a

clean:
	rm -f *.o *.a
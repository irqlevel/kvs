include $(ROOTDIR)/Makefile.inc

SOURCES=client.cc main.cc ../lbs/service.pb.c
BINARY=lbs-client

all:
	$(CPP) -I../pb $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(SOURCES) $(LIB)/common.a $(LIB)/pb.a -o $(BINARY) -lpthread
	cp $(BINARY) $(BIN)/$(BINARY)
	rm -f $(BINARY)

clean:
	rm -f $(BINARY)
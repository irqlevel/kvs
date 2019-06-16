include $(ROOTDIR)/Makefile.inc

SOURCES=main.cc server.cc
BINARY=mds

all:
	$(CPP) $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(SOURCES) $(LIB)/common.a -o $(BINARY) -lpthread
	cp $(BINARY) $(BIN)/$(BINARY)
	rm -f $(BINARY)

clean:
	rm -f $(BINARY)
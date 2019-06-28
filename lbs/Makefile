include $(ROOTDIR)/Makefile.inc

SOURCES=main.cc server.cc service.pb.c disk_manager.cc
BINARY=lbs

all:
	protoc --nanopb_out=-I.:. ./service.proto --proto_path .
	$(CPP) -I../pb $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(SOURCES) $(LIB)/common.a $(LIB)/pb.a -o $(BINARY) -lpthread
	cp $(BINARY) $(BIN)/$(BINARY)
	rm -f $(BINARY)

clean:
	rm -f $(BINARY)
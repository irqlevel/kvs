include $(ROOTDIR)/Makefile.inc

SOURCES=sync/mutex.cc sync/rwmutex.cc sync/thread.cc sync/atomic.cc sync/waitgroup.cc sync/condwait.cc	\
	sync/coroutine.cc sync/signal.cc sync/process.cc sync/eventfd.cc sync/comutex.cc sync/corwmutex.cc \
	sync/spinlock.cc net/socket.cc malloc/malloc.cc filesys/file.cc filesys/path.cc filesys/directory.cc \
	io/epoll.cc io/coepoll.cc io/random.cc io/aio.cc \
	net/tcp_server.cc net/tcp_req_server.cc stdlib/trace.cc stdlib/trace.cc stdlib/base.cc stdlib/bitmap.cc \
	stdlib/time.cc stdlib/abort.cc stdlib/list_entry.cc stdlib/hash.cc stdlib/byteorder.cc stdlib/errno.cc stdlib/utf8.cc stdlib/error.cc

LIBNAME=common

all:
	$(CPP) -c $(COMMON_CFLAGS) $(EXTRA_CFLAGS) $(SOURCES) -lpthread
	rm -f $(LIBNAME).a
	$(AR) -cvq $(LIBNAME).a *.o
	cp $(LIBNAME).a $(LIB)/$(LIBNAME).a

clean:
	rm -f *.o *.a
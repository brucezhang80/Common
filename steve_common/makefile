CXX = g++

.SUFFIXES:.d

S_LIB = ../../lib/libcomm.a
D_LIB = ../../lib/libcomm.so
BIN = ../../bin/tserv
TESTMOD = ../../lib/libtmod.so
TESTCLT = ../../bin/tclt
S_OBJS = base_socket.o baselist.o buffer.o CFileSeqRead.o CFileSeqWrite.o common.o conf.o \
         event.o lock.o lthread.o MD5.o tcp_client.o tcp_server.o tfrm.o tservfrm.o tservconf.o \
	 runable.o sp_util.o timerthread.o log.o cyclog.o hash.o shm.o hashmap.o mcache.o mbhash.o \
	 merge.o hashf.o bhash.o bsearch.o mbsearch.o chunk.o bfile.o bhashf.o mqueue.o realtime.o
D_OBJS = base_socket.d baselist.d buffer.d CFileSeqRead.d CFileSeqWrite.d common.d conf.d \
         event.d lock.d lthread.d MD5.d tcp_client.d tcp_server.d tfrm.d tservfrm.d tservconf.d \
 	 runable.d sp_util.d timerthread.d log.d cyclog.d hash.d shm.d hashmap.d mcache.d mbhash.d \
	 merge.d hashf.d bhash.d bsearch.d mbsearch.d chunk.d bfile.d bhashf.d mqueue.d realtime.d

C_FLAGS = -g -Wall

INC = -I. -I../../include

LIBPATH = -L../../lib
LIB = -lcomm -ldl -lpthread

.cpp.o:
	$(CXX) -c -o $*.o $(C_FLAGS) $(INC) $*.cpp  

.cpp.d:
	$(CXX) -c -fPIC -o $*.d $(C_FLAGS) $(INC) $*.cpp  

all: $(S_OBJS) $(D_OBJS) tserv.o tserv_test_clt.o tserv_test_mod.d
	ar -r $(S_LIB) $(S_OBJS)
	$(CXX) -shared -fPIC -o $(D_LIB) $(D_OBJS)
	$(CXX) -o $(BIN) tserv.o -Wl,-rpath,../lib $(LIBPATH) $(LIB)
	$(CXX) -shared -fPIC -o $(TESTMOD) tserv_test_mod.d
	$(CXX) -o $(TESTCLT) tserv_test_clt.o ../../lib/libcomm.a -lpthread

clean:	
	rm -f $(S_OBJS) $(D_OBJS) $(S_LIB) $(D_LIB) $(BIN) $(TESTMOD) $(TESTCLT) tserv.o tserv_test_clt.o tserv_test_mod.d

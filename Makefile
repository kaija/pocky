OBJS=pocky.o simclist.o
EXEC=pocky

STRIP=strip
EXEC_OBJ= main.o

SHARED_LIB=libpocky.so
STATIC_LIB=libpocky.a

CFLAGS = -fPIC
CFLAGS+= -Wall
CFLAGS+= -DDEBUG

LDFLAGS = $(STATIC_LIB)
LDFLAGS+= -L.
LDFLAGS+= -lpthread

.PHONY: all clean strip

all: static shared exec test

static: $(OBJS)
	$(AR) rcs $(STATIC_LIB) $(OBJS) 

shared: $(OBJS)
	$(CC) -shared -o $(SHARED_LIB) $(OBJS) $(CFLAGS)


exec: $(EXEC_OBJ)
	$(CC) -o $(EXEC) $(EXEC_OBJ) $(CFLAGS) $(LDFLAGS)

test: upunch.o
	$(CC) -o upunch upunch.o $(CFLAGS) $(LDFLAGS)

strip:
	$(STRIP) -R .note -R .comment $(SHARED_LIB)
	$(STRIP) -R .note -R .comment $(STATIC_LIB)

clean:
	rm -rf *.o $(EXEC) $(SHARED_LIB) $(STATIC_LIB) upunch

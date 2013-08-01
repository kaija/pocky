OBJS=pocky.o simclist.o
EXEC=pocky


EXEC_OBJ= main.o

SHARED_LIB=libpocky.so
STATIC_LIB=libpocky.a

CFLAGS = -fPIC
CFLAGS+= -Wall
CFLAGS+= -DDEBUG

LDFLAGS = $(STATIC_LIB)
LDFLAGS+= -L.

.PHONY: all clean

all: static shared exec

static: $(OBJS)
	$(AR) rcs $(STATIC_LIB) $(OBJS) 

shared: $(OBJS)
	$(CC) -shared -o $(SHARED_LIB) $(OBJS) $(CFLAGS)


exec: $(EXEC_OBJ)
	$(CC) -o $(EXEC) $(EXEC_OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(EXEC) $(SHARED_LIB) $(STATIC_LIB)

# Makefile for client/server example
# see 'man make' for more details
SERVER          = rdtserver
CLIENT          = rdtclient
SERVER_SOURCES  = rdtserver.cpp
CLIENT_SOURCES  = rdtclient.cpp

DEFINES         = 

CFLAGS         = -Wall -Werror -Wextra -pedantic `xml2-config --cflags`

LIBRARIES       =`xml2-config --libs`

CC              = g++
SERVER_OBJECTS  = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS  = $(CLIENT_SOURCES:.c=.o)
INCLUDES        = #-I.
LIBDIRS         = 
LDFLAGS         = $(LIBDIRS) $(LIBRARIES)

###########################################

.SUFFIXES: .cpp .o

.cpp.o:
		$(CC) $(CFLAGS) -c $<

###########################################

all:		$(SERVER) $(CLIENT)

rebuild:	clean all

$(SERVER):	$(SERVER_OBJECTS)
		$(CC) $(SERVER_OBJECTS) $(LDFLAGS) -o $@ -I/usr/include/libxml2

$(CLIENT):	$(CLIENT_OBJECTS)
		$(CC) $(CLIENT_OBJECTS) $(LDFLAGS) -o $@ -I/usr/include/libxml2

###########################################

clean:
	rm -fr core* *~ $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER) $(CLIENT) .make.state .sb

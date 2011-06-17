IDIR=include
ODIR=build
LDIR=lib
SRCDIR=src
EXDIR=examples
CPP=g++
CPPFLAGS= -O2 -Wall -W
LDFLAGS=-lpthread
UNAME= $(shell uname)
STATIC_LIB=$(LDIR)/liburemote.a
ifeq ($(UNAME), Darwin)
SHARED_LIB=$(LDIR)/liburemote.dylib
DYLIB_FLAGS=-dynamiclib
else 
SHARED_LIB=$(LDIR)/liburemote.so
DYLIB_FLAGS=-shared
endif


HEADERS = URemote.h URCommon.h URServer.h URClient.h URCommonPrivate.h URField.h
DEPS = $(patsubst %,$(IDIR)/%,$(HEADERS))

SOURCES = URServer.cpp URClient.cpp URCommon.cpp URField.cpp
_OBJ = $(SOURCES:.cpp=.o)
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: static shared examples
static: $(STATIC_LIB)
shared: $(SHARED_LIB)
examples: client-example server-example
clean-examples: clean-client clean-server
client-example: $(EXDIR)/client
server-example: $(EXDIR)/server
clean-client:
	rm -f $(EXDIR)/client
clean-server:
	rm -f $(EXDIR)/server

$(EXDIR)/client: $(EXDIR)/client.cpp $(STATIC_LIB)
	$(CPP) -I$(IDIR) $(CPPFLAGS) $(LDFLAGS) -L$(LDIR) -luremote $< -o $@

$(EXDIR)/server: $(EXDIR)/server.cpp $(STATIC_LIB)
	$(CPP) -I$(IDIR)  $(CPPFLAGS) $(LDFLAGS) -L$(LDIR) -luremote $< -o $@

$(OBJ): $(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	mkdir -p $(ODIR); $(CPP) -c $(CPPFLAGS) -I$(IDIR) -fPIC $< -o $@

$(STATIC_LIB): $(OBJ)
	ar -rcs $@ $^

$(SHARED_LIB): $(OBJ)
	$(CPP) $(DYLIB_FLAGS) -o $@ $^ $(LDFLAGS)

clean-static:
	rm -f $(STATIC_LIB)

clean-shared:
	rm -f $(SHARED_LIB)

clean: clean-static clean-shared clean-examples
	rm -rf $(ODIR) $(SRCDIR)/*~ $(INCDIR)/*~ 

.PHONY: all static shared clean-static clean-shared clean-server clean-client clean-examples clean examples

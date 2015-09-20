CXXFLAGS=-Wall -O3 -g -fno-strict-aliasing
BINARIES=led-matrix minimal-example rgbmatrix.so

# Where our library resides. It is split between includes and the binary
# library in lib
RGB_INCDIR=include
RGB_LIBDIR=lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

all : $(BINARIES) test-server

$(RGB_LIBRARY):
	$(MAKE) -C $(RGB_LIBDIR)

#led-matrix : demo-main.o $(RGB_LIBRARY)
#	$(CXX) $(CXXFLAGS) demo-main.o -o $@ $(LDFLAGS)

# minimal-example : minimal-example.o $(RGB_LIBRARY)
#	$(CXX) $(CXXFLAGS) minimal-example.o -o $@ $(LDFLAGS)

#text-example : text-example.o $(RGB_LIBRARY)
#	$(CXX) $(CXXFLAGS) text-example.o -o $@ $(LDFLAGS)

# Python module
rgbmatrix.so: rgbmatrix.o $(RGB_LIBRARY)
	$(CXX) -s -shared -lstdc++ -Wl,-soname,librgbmatrix.so -o $@ $< $(LDFLAGS)

%.o : %.cc
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -DADAFRUIT_RGBMATRIX_HAT -c -o $@ $<

clean:
	rm -f *.o $(OBJECTS) $(BINARIES)
	$(MAKE) -C lib clean

test-server: test-server.cc $(RGB_LIBRARY)
	# gcc -o test-server -DINSTALL_DATADIR= -lwebsockets -I /usr/local/include -L /usr/local/lib/ test-server.c
	gcc -o test-server -DINSTALL_DATADIR=  -lwebsockets -I /usr/local/include -I include/ -L /usr/local/lib/ -L lib -lrgbmatrix -lrt -lm -lpthread -lstdc++ -Wall -O3 -g -DADAFRUIT_RGBMATRIX_HAT -fno-strict-aliasing lib/gpio.cc lib/thread.cc lib/framebuffer.cc lib/led-matrix.cc test-server.cc

runserver: test-server
	sudo LD_LIBRARY_PATH=/usr/local/lib/ ./test-server

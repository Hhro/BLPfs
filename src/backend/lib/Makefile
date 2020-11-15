CXX = g++
LIB = libblpfs.a
OBJS = fs.o dispatcher.o

CXXFLAGS = -std=c++1z -I ../include

$(LIB): $(OBJS)
	$(AR) rv $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@  

clean:
	rm -f $(OBJS) $(LIB)

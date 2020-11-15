EXEC = fs_gen
CXX = g++

$(EXEC): fs_gen.o
		$(CXX) $< -o $(EXEC) -L ../lib -lblpfs

%.o: %.cpp
		$(CXX) -I ../include -c $< -o $@

clean:
		rm -f *.o $(EXEC) blpfs*

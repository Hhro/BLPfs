EXEC = test
CXX = g++

$(EXEC): test.o
		$(CXX) -o $(EXEC) test.o -L ../lib -lblpfs

%.o: %.cpp
		$(CXX) -c $< -o $@ -I../include

clean:
		rm -f test.o test

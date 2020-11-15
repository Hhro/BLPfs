EXEC = test
CXX = g++

$(EXEC): test.o
		$(CXX) -o $(EXEC) test.o -L ../lib -lblpfs -ggdb

%.o: %.cpp
		$(CXX) -c $< -o $@ -I../include -ggdb

clean:
		rm -f test.o test blpfs

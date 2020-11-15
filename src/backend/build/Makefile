EXEC = blpfsm
CXX = g++

$(EXEC): blpfsm.o
		$(CXX) -o $(EXEC) blpfsm.o -L ../lib -lblpfs

%.o: %.cpp
		$(CXX) -I../include -c $< -o $@

clean:
		rm -f blpfsm.o blpfsm

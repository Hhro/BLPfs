test: fs.o
	g++ -o fsm main.cpp fs.cpp -std=c++1z -ggdb
	g++ -o fs_gen fs_gen.cpp fs.cpp -std=c++1z -ggdb
	./fs_gen

clean:
	rm *.o fsm fs_gen blpfs

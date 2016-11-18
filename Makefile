FileUtils.o: FileUtils.cpp FileUtils.h
	g++ -c FileUtils.cpp
Session.o: Session.cpp Session.h
	g++ -c Session.cpp
main.o: main.cpp FileUtils.h Session.h
	g++ -c main.cpp
main: main.o FileUtils.o Session.o
	g++ -o main main.o FileUtils.o Session.o
clean:
	rm -f main
	rm -f main.o
	rm -f FileUtils.o
	rm -f Session.o

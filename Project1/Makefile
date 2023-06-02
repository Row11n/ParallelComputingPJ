target = main.o

main : $(target)
	g++ -fopenmp -o main $(target) && clear &&./main

main.o : main.cpp
	g++ -fopenmp -c main.cpp

gdb : $(target)
	g++ -fopenmp -o main -g main.cpp && main algo

excute : $(target)
	clear && ./main

clean : 
	-rm -f main.o main
EXECUTABLES = mpi_solved1 mpi_solved2 mpi_solved3 mpi_solved4 mpi_solved5 mpi_solved6 mpi_solved7 sort
COMPILER = mpicc #mpicc-openmpi-mp
FLAGS = -O3 -Wall

# make all
all: $(EXECUTABLES)

# problem 1
mpi_solved1: mpi_solved1.c
	$(COMPILER) $(FLAGS) mpi_solved1.c -o mpi_solved1

1: mpi_solved1
	clear
	mpirun -np 4 ./mpi_solved1  


# problem 2
mpi_solved2: mpi_solved2.c
	$(COMPILER) $(FLAGS) mpi_solved2.c -o mpi_solved2

2: mpi_solved2
	clear
	mpirun -np 4 ./mpi_solved2  



# problem 3
mpi_solved3: mpi_solved3.c
	$(COMPILER) $(FLAGS) mpi_solved3.c -o mpi_solved3

3: mpi_solved3
	clear
	mpirun -np 4 ./mpi_solved3  



# problem 4
mpi_solved4: mpi_solved4.c
	$(COMPILER) $(FLAGS) mpi_solved4.c -o mpi_solved4

4: mpi_solved4
	clear
	mpirun -np 4 ./mpi_solved4  




# problem 5
mpi_solved5: mpi_solved5.c
	$(COMPILER) $(FLAGS) mpi_solved5.c -o mpi_solved5

5: mpi_solved5
	clear
	mpirun -np 4 -hosts box567,box571 ./mpi_solved5  


# problem 6
mpi_solved6: mpi_solved6.c
	$(COMPILER) $(FLAGS) mpi_solved6.c -o mpi_solved6

6: mpi_solved6
	clear
	mpirun -np 4 ./mpi_solved6  


mpi_bug6: mpi_bug6.c
	$(COMPILER) $(FLAGS) mpi_bug6.c -o mpi_bug6

66: mpi_bug6
	clear
	mpirun -np 4 ./mpi_bug6  




# sort problem
sort: ssort.c
	$(COMPILER) $(FLAGS) ssort.c -o sort

run: sort
	mpirun -np 10 -hosts box567,box571 ./sort 10000

crunch: sort
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 1000
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 10000
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 100000
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 1000000
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 10000000
	mpirun -np 16 -hosts crunchy1,crunchy3 ./sort 100000000


# clean up
clean:
	rm -rf $(EXECUTABLES)
	rm -vf *~
	rm -vf #*
	rm -vf 0* 1* 2* 3* 4* 5* 6* 7* 8* 9*
	rm -vf .#*
push:
	git push https://github.com/yairdaon/HPC15-HW2.git

pull:
	git pull https://github.com/yairdaon/HPC15-HW2.git

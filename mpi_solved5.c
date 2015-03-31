/******************************************************************************
* FILE: mpi_bug5.c
* DESCRIPTION: 
*   This is an "unsafe" program. It's behavior varies depending upon the
*   platform and MPI library
* AUTHOR: Blaise Barney 
* LAST REVISED: 01/24/09
* Hint: If possible, try to run the program on two different machines,
* which are connected through a network. You should see uneven timings;
* try to understand/explain them.
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MSGSIZE 2000

int main (int argc, char *argv[]) {

  int         numtasks, rank, i, tag=111, dest, source, count=0;
  char        data[MSGSIZE];
  double      start, end, result;
  MPI_Status  status;
  MPI_Request req;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  source = 0;
  dest   = 1;
 
  if (rank == 0) {
    printf ("mpi_solved5 has started...\n");
    if (numtasks > 2) 
      printf("INFO: Number of tasks= %d. Only using 2 tasks.\n", numtasks);
  }

  /******************************* Send task **********************************/
  if (rank == source) {

    /* Initialize send data */
    for(i=0; i<MSGSIZE; i++)
      data[i] =  'x';

    start = MPI_Wtime();
    while (1) {

      //MPI_Send(data, MSGSIZE, MPI_BYTE, dest, tag, MPI_COMM_WORLD);
      MPI_Isend(data  ,MSGSIZE, MPI_BYTE, dest, tag, MPI_COMM_WORLD, &req);
      
      count++;
      if ( count %  20 == 0  ) {
	end = MPI_Wtime();
	printf("Count= %d  Time= %f sec.\n", count, end-start);
	start = MPI_Wtime();
      }

    }
  }

/****************************** Receive task ********************************/
  
  if (rank == dest) {
    count = 0;
    while (1) {

      //MPI_Recv(data, MSGSIZE, MPI_BYTE, source, tag, MPI_COMM_WORLD, &status);
      MPI_Irecv(data , MSGSIZE, MPI_BYTE, source, tag, MPI_COMM_WORLD, &req);

      /* Do some work  - at least more than the send task */
      result = 0.0;
      for (i=0; i < 1000000; i++) 
	result = result + (double)random();
      count++;

    }

  }
  
  MPI_Finalize();
}
// Made communication non blocking

/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>


static int compare(const void *a, const void *b);
void writeResults(int * arr , int n, int rank, float time );

int main( int argc, char *argv[]) {

  int rank, nTasks;
  int i,j;
  double start , end;


  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
  start = MPI_Wtime();

  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be passed in through the command line */
  int locDataSize =  atoi(argv[1]); // set originally to N = 100
  
  // number of splitters
  int nSplitters = locDataSize/10;

  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 3925539));

  /* fill vector with random integers */
  int *locData = calloc(locDataSize, sizeof(int));
  for (i = 0; i < locDataSize; ++i) {
    locData[i] =  rand();
  }

  /* sort locally */
  qsort(locData, locDataSize, sizeof(int), compare);


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 1 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* randomly sample nSplitters entries from vector or select local splitters,
   * i.e., every N/P-th entry of the sorted vector */
  
  //create array of splitters
  int *splitters = calloc(nSplitters, sizeof(int));
  
  // choose splinters
  int jump = locDataSize/(nSplitters-1);
  for( i = 0 ; i < nSplitters-1 ; i++) {
    splitters[i] = locData[i*jump];
  }

  // make sure we get all range of values
  splitters[nSplitters-1] = locData[locDataSize-1];


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 2 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* every processor communicates the selected entries
   * to the root processor; use for instance an MPI_Gather */

  int rootDataSize = nTasks*nSplitters; 
  int * rootData   = calloc(rootDataSize , sizeof(int));
  MPI_Gather(splitters, nSplitters, MPI_INT, rootData, nSplitters, MPI_INT, 0, MPI_COMM_WORLD);


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 3 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* root processor does a sort, determines splitters that
   * split the data into P buckets of approximately the same size */

  // sort your data if you're the root
  if ( rank == 0)
    qsort(rootData, rootDataSize , sizeof(int), compare);
   
  // the number of splitters in the list of the root
  int rootNumSplitters = nTasks - 1;
    
  // the jump in the list of the root
  int rootJump = rootDataSize/(rootNumSplitters+1);
  int *rootSplitters = calloc(rootNumSplitters, sizeof(int));

  for( i = 0 ; i < rootNumSplitters ; i++) {
    rootSplitters[i] = rootData[(i+1)*rootJump];
  }


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 4 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* root process broadcasts splitters */
  MPI_Bcast( rootSplitters, rootNumSplitters, MPI_INT, 0, MPI_COMM_WORLD );


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 5 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* every processor uses the obtained splitters to decide
   * which integers need to be sent to which other processor (local bins) */


  // allocate memory for an array that holds the left and right endpoints of this
  // processor's data that goes to the ith processor
  int * sdisp   = calloc( nTasks, sizeof(int));
  int * scount  = calloc( nTasks, sizeof(int));
  

  int currInd = 0;
  for( j = 0 ; j < nTasks ; j++) {

    // go over all local data, compare it to splitters and do stuff:
    for( i = currInd ; i < locDataSize ; i++) {
    
      // whenever we find ourselves above the current splitter...
      if ( locData[i] >  rootSplitters[j] ) {
	scount[j]    = i - currInd; // bucket size is this for this processor
	currInd      = i+1; 
	break;
      }
    }
  }
  // the last index j == nTasks - 1
  scount[nTasks-1] = locDataSize - currInd;


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 6 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */


  int * rdisp  = calloc(nTasks , sizeof(int) );
  int * rcount = calloc(nTasks , sizeof(int) );

  // share the number of int everyone needs to expect to get
  MPI_Alltoall( scount, 1, MPI_INT, rcount, 1, MPI_INT, MPI_COMM_WORLD);
  
  // set the displacements
  rdisp[0] = 0;
  for( i = 1 ; i < nTasks ; i++){
    rdisp[i] = rcount[i-1] + rdisp[i-1];
    sdisp[i] = scount[i-1] + sdisp[i-1];
  }

  // calculate memory to be allocated:
  int rDataSize = 0;
  for(i = 0 ; i < nTasks ; i++ ) {
    rDataSize = rDataSize + rcount[i];
  }

  // allocate the memory for the received data
  int * rData = calloc(rDataSize , sizeof(int));
 

  // share the data
  MPI_Alltoallv(locData ,  scount, sdisp, MPI_INT,
		rData   ,  rcount, rdisp, MPI_INT,
		MPI_COMM_WORLD);  
  

  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 7 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* do a local sort */
  qsort(rData, rDataSize , sizeof(int), compare);
  end = MPI_Wtime();
  float time = (float)(end-start);
  float avg;
  MPI_Reduce( &time, &avg, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
  avg = avg/nTasks;

  /* every processor writes its result to a file */
  //writeResults(rData , rDataSize , rank,  time  );




  /* some statistics about the run */
  //printf("Rank %2d. Data size =  %9d. Time = %5f \n", rank, rDataSize, time );
  if( rank == 0) 
    printf("%d in %f seconds.\n" , locDataSize, avg);

  // clean up
  free(locData);
  free(rData);
  free(rcount);
  free(scount);
  free(rdisp);
  free(sdisp);
  free(rootData);
  free(rootSplitters);
  free(splitters);
  MPI_Finalize();
  return 0;
}

static int compare(const void *a, const void *b) {
  /* 
     return 1 if the value at *a is bigger
     than the value at *b
  */

  // cast to  pointers to int
  int *da = (int *)a;
  int *db = (int *)b;

  // if a > b return 1, ow return stuff
  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

void writeResults(int * arr , int n, int rank, float time  ) {
  
  int i;
  char str[15];
  sprintf(str, "%d", rank);
  FILE *filePtr;
 
  filePtr = fopen(str,"w");

  fprintf(filePtr, "Elapsed time: %f seconds.\n", time);
  fprintf(filePtr, "Got %d numbers.\n", n);
  for (i = 0; i < n ; i++) {
    fprintf(filePtr, "%d\n", arr[i]);
  }
}
 

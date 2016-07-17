/*
 * Mesh Multiplication
 * Program for parallem multiplication of two given matrices.
 * Author: Peter Lacko, xlacko06@stud.fit.vutbr.cz
 */
#include <mpi.h>
#include <cstdio>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#define BUFSIZE 128
#define INFO 0
#define DATA 16
#define LEFT 1
#define TOP 2
#define RESULT 8
#define PRINT 10
#define TIME 64


using namespace std;


int main(int argc, char *argv[])
{
	char idstr[32];
	char buff[BUFSIZE];
	int numprocs;
	int myid;
	int i;
	MPI_Status stat;
	/* MPI programs start with MPI_Init; all 'N' processes exist thereafter */
	MPI_Init(&argc,&argv);
	/* find out how big the SPMD world is */
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	/* and this processes' rank is */
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	vector<int> rowIn;
	vector<int> colIn;
	int total;
	int nrows, ncols;
    double time_start, time_end;


	/* 
	 * Values are read by first processor and distributed to others.
     * First matrices info, then values itself to input processors.
	 */
	if (myid == 0) { // I'm reading both files
		FILE *f1 = fopen("mat1", "r");
		FILE *f2 = fopen("mat2", "r");
		fscanf(f1, "%d", &nrows);
		fscanf(f2, "%d", &ncols);
        int c;
        while (!feof(f1)) {
            fscanf(f1, "%d", &c);
            rowIn.push_back(c);
        }
        fclose(f1);
        rowIn.pop_back();
        total = rowIn.size()/nrows;
        colIn.resize(total*ncols);
        int i = 0;
        while (true) {
            fscanf(f2, "%d", &c);
            if (feof(f2))
                break;
            colIn.at((i%ncols)*total + i/ncols) = c;
            i++;
        }
        fclose(f2);
        int check = colIn.size()/ncols;
        if (total != check) {
            fprintf(stderr, "Fatal error!\n Matrices dimensions do not match!\n");
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        int info[3] = {nrows, ncols, total};

        if (ncols > 1) {
            MPI_Send(info, 3, MPI_INT, myid+1, INFO, MPI_COMM_WORLD);
            for (int i = colIn.size()-total; i != 0; i -= total) {
                MPI_Send(&colIn[i], total, MPI_INT, myid+1, DATA, MPI_COMM_WORLD);
            }
            colIn.resize(total);
        }
        if (nrows > 1) {
            MPI_Send(info, 3, MPI_INT, myid+ncols, INFO, MPI_COMM_WORLD);
            for (int i = rowIn.size()-total; i != 0; i -= total) {
                MPI_Send(&rowIn[i], total, MPI_INT, myid+ncols, DATA, MPI_COMM_WORLD);
            }
            rowIn.resize(total);
        }
	} else {
        int info[3];
        MPI_Recv(&info, 3, MPI_INT, MPI_ANY_SOURCE, INFO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        nrows = info[0];
        ncols = info[1];
        total = info[2];
        if (myid+ncols < numprocs) {
            MPI_Send(&info, 3, MPI_INT, myid+ncols, INFO, MPI_COMM_WORLD);
        }
        if ((myid+1)%ncols != 0) {
            MPI_Send(&info, 3, MPI_INT, myid+1, INFO, MPI_COMM_WORLD);
        }
        if (myid < ncols) {
            int data[total];
            for (int i = myid; i < ncols - 1; i++) {
                MPI_Recv(&data, total, MPI_INT, myid-1, DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&data, total, MPI_INT, myid+1, DATA, MPI_COMM_WORLD);
            }
            MPI_Recv(&data, total, MPI_INT, myid-1, DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> tmp (data, data+total);
            colIn = tmp;
        }
        if (myid % ncols == 0) {
            int data[total];
            for (int i = myid/ncols; i < nrows - 1; i++) {
                MPI_Recv(&data, total, MPI_INT, myid-ncols, DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&data, total, MPI_INT, myid+ncols, DATA, MPI_COMM_WORLD);
            }
            MPI_Recv(&data, total, MPI_INT, myid-ncols, DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> tmp (data, data+total);
            rowIn = tmp;
        }
	}

    time_start = MPI_Wtime();
	/* 
	 * And now algorithm itself
	 */
	bool cont = true;
	int currInput = 0;
	int currValue = 0;
	while (cont) {
        int rcvCol, rcvRow;
        // first recieve values from input/prev. processors
        if (myid < ncols || myid%ncols == 0) {
            if (myid == 0) {
                rcvCol = colIn[currInput];
                rcvRow = rowIn[currInput];
            }
            if (myid < ncols && myid != 0) {
                rcvCol = colIn[currInput];
                if (myid != 0) {
                    MPI_Recv(&rcvRow, 1, MPI_INT, myid-1,
                            LEFT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            if (myid%ncols == 0 && myid != 0) {
                rcvRow = rowIn[currInput];
                if (myid != 0) {
                    MPI_Recv(&rcvCol, 1, MPI_INT, myid-ncols,
                            TOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
        } else {
            MPI_Recv(&rcvCol, 1, MPI_INT, myid-ncols,
                    TOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&rcvRow, 1, MPI_INT, myid-1,
                    LEFT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // update current value
        currValue += rcvRow * rcvCol;
        // and send
        if (myid + ncols < numprocs) {
            MPI_Send(&rcvCol, 1, MPI_INT, myid+ncols, TOP, MPI_COMM_WORLD);
        }
        if ((myid+1) % ncols != 0) {
            MPI_Send(&rcvRow, 1, MPI_INT, myid+1, LEFT, MPI_COMM_WORLD);
        }
        currInput++;
        if (currInput == total) {
            cont = false;
        }
	}
    MPI_Barrier(MPI_COMM_WORLD);
    if (myid == 0) {
        time_end = MPI_Wtime();
    }

    /*
     * Each processor now sends it's result back to proc 0 which writes output
     */
    int *result;
    if (myid == 0) {
       result = (int*) malloc(sizeof(int) * numprocs);
    }
    MPI_Gather(&currValue, 1, MPI_INT, result, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (myid == 0) {
        printf("%d:%d\n", nrows, ncols);
        for (int i = 0; i < nrows; i++) {
            for (int j = 0; j < ncols-1; j++) {
                printf("%d ", result[i*ncols+j]);
            }
            printf("%d", result[i*ncols+ncols-1]);
            printf("\n");
        }
        free(result);
    }
    /*
    if (myid == 0) {
        fprintf(stderr, "Time: %f\n", time_end-time_start);
    }
    */

	/* MPI programs end with MPI Finalize; this is a weak synchronization point */
	MPI_Finalize();
	return 0;
}

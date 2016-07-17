#include <mpi.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
#define TAG 0

int main(int argc, char *argv[])
{
	// queues for numbers
	vector<int> q1;
	vector<int> q2;
	vector<int> q3;
	vector<int> q4;
	int recieved;
	int queueLen;
	int sorted;
	int sent;
	int rcvdValue;
	int totalRecieved;
	bool enabled;
	bool top;
	bool finished;
	int selected;

	int numprocs;
	int myid;
	MPI_Status stat;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	double startTime, endTime;
	startTime = MPI_Wtime();

	queueLen = pow(2, myid-1);
	vector<int>*q1In = &q1;
	vector<int>*q2In = &q2;
	vector<int>*q1Out = &q1;
	vector<int>*q2Out = &q2;
	vector<char>sortedValues;
	recieved = 0;
	totalRecieved = 0;
	sorted = 0;
	sent = 0;
	top = true;
	enabled = false;
	finished = false;
	// open file for reading
	fstream fin;            
	char input[]= "numbers";
	if (myid == 0) {
		fin.open(input, ios::in);                   
	}
	while (true) {
		/*
		 * first checking if I can pass value from one of queues
		 * first processor reads from file and sends to next processor
		 */
		if (myid == 0) {
			int number;            
			number = fin.get();
			if (!fin.good()) {
				fin.close();
				int l=-1;
				MPI_Send(&l, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
				cout << endl;
				cout.flush();
				break;
			}
			cout << number << " ";
			MPI_Send(&number, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
		}
		// logic for all other processors is the same
		else if (myid > 0) {
			if (!enabled && (q1Out->size() == queueLen && q2Out->size() == 1)) {
				enabled = true;
			}
			// if "ready to send", pick one datum and send to next processor
			if (enabled) {
				if (!q1Out->empty() && !q2Out->empty()) {
					if (q1Out->front() < q2Out->front()) {
						selected = q1Out->front();
						q1Out->erase(q1Out->begin());
					} else {
						selected = q2Out->front();
						q2Out->erase(q2Out->begin());
					}
				} else if (q1Out->empty()) {
					selected = q2Out->front();
					q2Out->erase(q2Out->begin());
				} else if (q2Out->empty()) {
					selected = q1Out->front();
					q1Out->erase(q1Out->begin());
				} else {
					cerr << myid << ": both OUT queues empty, why?\n";
				}
				sorted++;
				if (q1Out->empty() && q2Out->empty()) {
					// output queues change
					if (q1Out == &q1) {
						q1Out = &q3;
						q2Out = &q4;
					} else {
						q1Out = &q1;
						q2Out = &q2;
					}
				}
			}
		}
		// now decide if send to next processor or write to a file
		if (myid > 0 && myid < numprocs-1) {
			if (enabled) {
				// send next
				MPI_Send(&selected, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);
				if (sorted == totalRecieved) {
					int l=-1;
					MPI_Send(&l, 1, MPI_INT, myid+1, TAG, MPI_COMM_WORLD);
					break;
				}
			}
		} else if (myid == numprocs-1) {
			if (enabled) {
				// if last processor, save to file
				sortedValues.push_back((char) selected);
				if (sorted == totalRecieved) {
					break;
				}
			}
		}

		/*
		 * Now we are recieving values from prev. processor and puting 
		 * them to one of two input queues
		 */
		if (myid > 0 && !finished) {
			// waits for data and puts it into one of her queues
			MPI_Recv(&rcvdValue, 1, MPI_INT, myid-1, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (rcvdValue == -1) {
				finished = true;
			} else {
				totalRecieved++;
				if (top && recieved < queueLen) {
					q1In->push_back(rcvdValue);
					recieved++;
				} else if (top && recieved == queueLen) {
					q2In->push_back(rcvdValue);
					recieved = 1;
					top = false;
				} else if (!top && recieved < queueLen) {
					q2In->push_back(rcvdValue);
					recieved++;
				} else if (!top && recieved == queueLen) {
					recieved = 1;
					top = true;
					// input queues change
					if (q1In == &q1) {
						q1In = &q3;
						q2In = &q4;
					} else {
						q1In = &q1;
						q2In = &q2;
					}
					q1In->push_back(rcvdValue);
				}
			}
		}
	}
	if (myid == numprocs-1) {
		for (int i = 0; i < sortedValues.size(); i++) {
			cout << (int) (unsigned char) sortedValues[i] << endl;
		}
		endTime = MPI_Wtime();
	}
	/* MPI programs end with MPI Finalize; this is a weak synchronization point */
	MPI_Finalize();
	return 0;
}

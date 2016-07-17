/*
 * Project Name:    Job Shop Scheduling
 * Year:            2015
 * Author:          Peter Lacko, xlacko06@stud.fit.vutbr.cz
 * Description:     Solves JobShop Scheduling problem using Genetic Algorithm
 */
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ga/ga.h>
#include <algorithm>
#include <sstream>

using namespace std;


struct taskStruct {
    int machine;
    int time;
    //taskStruct(int machine, int time): machine(machine), time(time) {}
};

struct schedStruct {
    int job;
    int start;
    int end;
    schedStruct(int job, int start, int end): job(job), start(start), end(end) {}
};

struct slotStruct {
    int start;  // included
    int end;    // excluded    
    slotStruct(int start, int end): start(start), end(end) {}
};

// global variable for holding jobs
vector<vector<taskStruct> > jobs;
int genomeLength;
int machineCount;
bool buildSchedule = false;
vector<vector<schedStruct> > schedule;

bool compareVectors(vector<taskStruct> &v1, vector<taskStruct> &v2) {
    if (v1.size() < v2.size()) {
        return true;
    }
    return false;
}

float jobsMakespan(GAGenome& g) {
    GA1DArrayGenome<int> &genome = (GA1DArrayGenome<int> &)g;
    // controls which task to take from given job
    vector<int> tasksDone(jobs.size(), 0);
    // keep end time of each job's last task
    vector<int> continueTime(jobs.size(), 0);
    // keeps track of free time slots on each machine
    vector<vector<slotStruct> > machineSlots;
    machineSlots.resize(machineCount);
    // initialize each machine to empty values
    for (int i = 0; i < machineSlots.size(); i++) {
        slotStruct slot(0,1000000000); // nahodne velke cislo
        machineSlots[i].push_back(slot);
    }


    for (int i = 0; i < genomeLength; i++) {
        int currJob = genome[i] % jobs.size();
        taskStruct currTask = jobs[currJob][tasksDone[currJob]++];
        int slotsCount = machineSlots[currTask.machine].size();
        //cout << "Slots count: " << slotsCount << endl;
        for (int s = 0; s < slotsCount; s++) {
            if (machineSlots[currTask.machine][s].end <= continueTime[currJob]) {
                //cout << "Continuting\n";
                continue;
            }
            int minStartTime = continueTime[currJob] > machineSlots[currTask.machine][s].start ?
                continueTime[currJob] : machineSlots[currTask.machine][s].start;
            //cout << "Min start time: " << minStartTime << endl;
            int diff = machineSlots[currTask.machine][s].end - minStartTime;
            // task can't fit to this time slot
            if (diff < currTask.time)
                continue;
            // task can fit here, so let's put it on it's new place
            // we have to modify current time slot: delete it or create 1 or 2 new slots
            continueTime[currJob] = minStartTime + currTask.time;
            if (minStartTime == machineSlots[currTask.machine][s].start
                    && minStartTime + currTask.time == machineSlots[currTask.machine][s].end) {
                machineSlots[currTask.machine].erase(machineSlots[currTask.machine].begin()+s);
            } else if (minStartTime == machineSlots[currTask.machine][s].start
                    && minStartTime + currTask.time < machineSlots[currTask.machine][s].end) {
                machineSlots[currTask.machine][s].start = minStartTime + currTask.time;

            } else if (minStartTime > machineSlots[currTask.machine][s].start
                    &&  minStartTime + currTask.time == machineSlots[currTask.machine][s].end) {
                machineSlots[currTask.machine][s].end = minStartTime;
            } else {
                slotStruct tmp(machineSlots[currTask.machine][s].start, minStartTime);
                machineSlots[currTask.machine].insert(machineSlots[currTask.machine].begin()+s, tmp);
                machineSlots[currTask.machine][s+1].start = minStartTime + currTask.time;
            }
            if (buildSchedule == true)
                schedule[currTask.machine].push_back(
                        schedStruct(currJob, minStartTime, minStartTime + currTask.time));
        }
        //cout << "Job: " << currJob << ", Cont. time: " << continueTime[currJob] << endl;
    }
    return (*max_element(continueTime.begin(), continueTime.end()));
}

void jobsInitializer(GAGenome& g) {
    GA1DArrayGenome<int> &genome = (GA1DArrayGenome<int> &) g;
    genome.length(genomeLength);
    vector<int> ran;
    for (int i = 0; i < genomeLength; i++) {
        ran.push_back(i);
    }
    random_shuffle(ran.begin(), ran.end());
    for(int i=0; i<genomeLength; i++) {
        genome[i]=ran.at(i);
    }
}

void printSchedule(const GAGenome &g) {
    GA1DArrayGenome<int> &genome = (GA1DArrayGenome<int> &)g;
    ofstream fSchedule;
    fSchedule.open("schedule.txt");
    buildSchedule = true;
    float makespan = jobsMakespan(genome);
    for (int i = 0; i < schedule.size(); i++) {
        //cout << "M" << i+1 << ": ";
        for (int j = 0; j < schedule[i].size(); j++) {
            //cout << "(" << schedule[i][j].job << ", " << schedule[i][j].start << ":" << schedule[i][j].end << ")";
            fSchedule << i+1 << " " << schedule[i][j].job << " "
                <<  schedule[i][j].start<< " " << schedule[i][j].end << endl;
        }
        //cout << endl;
    }
    fSchedule.close();
}

//--------------------------- main function ----------------------------------//
int main(int argc, char **argv) {
    ifstream fJobs;
    fJobs.open(argv[1]);
    string line;
    getline(fJobs, line);
    // get number of machines from first line
    istringstream iss(line);
    iss >> machineCount;
    while(getline(fJobs, line)) {
        vector<taskStruct> job;
        taskStruct task;
        istringstream iss(line);
        while (iss.good()) {
            iss >> task.machine >> task.time;
            //cout << task.machine << " " << task.time << endl;
            job.push_back(task);
        }
        jobs.push_back(job);
    }
    // set machine count for schedule
    schedule.resize(machineCount);

    //-- Declaration of population with genome size and Fitness function --//
    genomeLength = (*max_element(jobs.begin(), jobs.end(), compareVectors)).size()*jobs.size();

    GA1DArrayGenome<int> genome(genomeLength, jobsMakespan);

    //-- Genetic operations --//
    genome.initializer(::jobsInitializer);
    genome.mutator(GA1DArrayGenome<int>::SwapMutator);		// randomly swap 2
    genome.crossover(GA1DArrayGenome<int>::PartialMatchCrossover);  // permuation suitable crossover

    //-- Declaration of genetic algorithm --//
    GASimpleGA ga(genome);
    // we will use elite selection as default
    // and trying to minimize lifespan
    ga.minimize();

    int geners = 1000;
    // default parameters settings
    ga.populationSize(10);
    ga.nGenerations(geners);
    ga.pMutation(0.1);
    ga.pCrossover(1.0);
    ga.scoreFrequency(50);

    //-- Parameters setting --//
    ga.parameters(argc,argv);

    //-- Start of evolution process --//
    ga.initialize();
    while(!ga.done()){
        ++ga; 

        if ((ga.statistics().generation() % ga.scoreFrequency()) == 0)
            cout << ga.population().min() << " ";

        if (ga.population().min() == 0) break;
    }
    ga.flushScores();


    //-- Print statistics --//
    //cout <<endl << "Final statistics:" << endl;
    //cout << ga.statistics() << endl;

    //cout << "The best solution: " <<  ga.statistics().bestIndividual() <<" , Fitness = " << ga.population().min() << endl;
    cerr << ga.population().min();
    printSchedule(ga.statistics().bestIndividual());

    return 0;
} //- main -//


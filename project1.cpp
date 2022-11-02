#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <list>
using namespace std;
////////////////////////////////////////////////////////////////
//events defined
#define ARRIVAL 1
#define DEPARTURE 2
#define PREEMPT 3
////////////////////////////////////////////////////////////////     
//process structure
struct process{
  int pid;
  float arrivalTime;
  float serviceTime;
  float actualStart;
  float finishTime;
  float remainingTime;
  float reTime;
};

//event  structure
struct event{
  float time;
  int   type;
  struct process p;
  // add more fields
  struct event* next;
};

//stucture for loading event priority queue
struct comp{
    bool operator()(const event& a, const event& b){
        return a.time>b.time;
    }
};
////////////////////////////////////////////////////////////////
// function definition
static void parameter_usage();
void parse_arguments(int, char *[]);
void init();
int run_sim();
void FCFS();
void SRTF();
void HRRN();
void RR();
float genexp(float);
void generate_report();
//int schedule_event(struct event*);
void schedule_dep(struct process);
void schedule_arr(struct event);
void process_arrival(struct event);
void process_departure(struct event);
void process_preempt(struct event);
////////////////////////////////////////////////////////////////
//Global variables
struct event* head; // head of event queue
float simclock; // simulation clock
int scheduler_type;
int lambda;
float completionTime;
float cpuTime;
float avgServiceTime = 0.0;
float totalTurnaroundTime;
float totalWaitingTime;
float quantumInterval;
bool cpuIdle;
int processCount = 0;
int RQprocessCount = 0;

priority_queue<event, vector<event>,comp> EventQueue;
list<process> ReadyQueue;
////////////////////////////////////////////////////////////////
void parse_arguments(int argc, char *argv[]){

    if (argc < 4 || argc > 5)
    {
        parameter_usage();
        exit(0);
    }
    if (stoi(argv[1]) == 4 && argc < 5)     
    {
        parameter_usage();
        exit(0);
    }
    scheduler_type = stoi(argv[1]);
    lambda = stoi(argv[2]);
    avgServiceTime = (float)atof(argv[3]);
    if (argc == 5){
        quantumInterval = (float)atof(argv[5]);
    } 
}

static void parameter_usage()
{
    cout << "\n 3 Arguments Required and a Fourth Argument if Round Robin selected\n\n" 
    << "[scheduler #] [average arrival rate] [average service time] [quantum interval]\n\n"
    << "list of schedulers by # :\n"
    << "1. First-Come First-Served (FCFS)\n"
    << "2. Shortest Remaining Time First (SRTF)\n"
    << "3. Highest Response Ratio Next (HRRN)\n"
    << "4. Round Robin, with different quantum values (RR)\n";
}

void init()
{
  cpuIdle = true;
  avgServiceTime = (float)1.0/avgServiceTime;


  process first_proc;
  first_proc.pid = 0;
  first_proc.arrivalTime = genexp((float)lambda);
  first_proc.serviceTime = genexp(avgServiceTime);
  first_proc.actualStart = 0.0;
  first_proc.finishTime = 0.0;
  first_proc.reTime = 0.0;
  first_proc.remainingTime = first_proc.serviceTime;


  event first_ev;
  first_ev.type = 1;
  first_ev.time = first_proc.arrivalTime;
  first_ev.p = first_proc;


  EventQueue.push(first_ev);
// initialize all varilables, states, and end conditions
// schedule first events
}
////////////////////////////////////////////////////////////////
void generate_report()
{
// output statistics
  float avgTurnaroundTime = totalTurnaroundTime / (float)processCount;
  float totalThroughput = (float)processCount / completionTime;
  float cpuUtil = cpuTime / completionTime;
  float avgNumRQProcesses = RQprocessCount / processCount;
  cout <<  "FCFS" << endl
  << "Lambda: " << lambda << endl
  << "Average Service Time: " << avgServiceTime << endl 
  << "Average Turnaround Time: " << avgTurnaroundTime << endl
  << "Total Throughput: " << totalThroughput << endl
  << "CPU Utilization: " << cpuUtil << endl
  << "Average Num of Processes in Ready Queue: " << avgNumRQProcesses;

}
////////////////////////////////////////////////////////////////
// returns a random number between 0 and 1
float urand()
{
return( (float) rand()/RAND_MAX );
}
/////////////////////////////////////////////////////////////
// returns a random number that follows an exp distribution
float genexp(float lambda)
{
float u,x;
x = 0;
while (x == 0)
{
u = urand();
x = (-1/lambda)*log(u);
}
return(x);
}

void schedule_arr(struct event eve){
  process new_proc;
  new_proc.pid = eve.p.pid + 1;
  new_proc.arrivalTime = eve.p.arrivalTime + genexp((float)lambda);
  new_proc.serviceTime = genexp(avgServiceTime);
  new_proc.actualStart = 0.0;
  new_proc.finishTime = 0.0;
  new_proc.reTime = 0.0;
  new_proc.remainingTime = new_proc.serviceTime;
  processCount = processCount + 1;

  event arrival;
  arrival.type = ARRIVAL;
  arrival.time = new_proc.arrivalTime;
  arrival.p = new_proc;
  
  EventQueue.push(arrival);

}

void schedule_dep(struct process pr){
  event dep;
  dep.type = DEPARTURE;
  dep.p = pr;
  if (scheduler_type == 2){
    if (pr.reTime == 0){
      dep.time = pr.actualStart + pr.remainingTime;
    }else{
      dep.time = pr.reTime + pr.remainingTime;
    }
    
  }else {
    dep.time = dep.p.actualStart + dep.p.remainingTime;
  }
  EventQueue.push(dep);

}

void schedule_preempt(){
  event preemption;
  preemption.type = PREEMPT;
  preemption.time = EventQueue.top().time;
  preemption.p = EventQueue.top().p;

  EventQueue.pop();
  EventQueue.push(preemption);
}

void process_departure(struct event eve){
  EventQueue.pop();
  simclock = EventQueue.top().time;
  if (eve.p.actualStart == 0){
      eve.p.actualStart = simclock;
  }
  eve.p.finishTime = simclock;
  cpuTime += eve.p.serviceTime;
  totalTurnaroundTime += (eve.p.finishTime - eve.p.arrivalTime);
  completionTime = eve.p.finishTime;
  totalWaitingTime += (eve.p.finishTime - eve.p.arrivalTime - eve.p.serviceTime);
  if (ReadyQueue.empty() == true){
    cpuIdle = true;
  }else{
    process pr = ReadyQueue.front();
    ReadyQueue.pop_front();
    schedule_dep(pr);
  }

}

void process_arrival(struct event eve){
  EventQueue.pop();
  if (cpuIdle == true){
    cpuIdle = false;
    schedule_dep(eve.p);
  }else{
    ReadyQueue.push_back(eve.p);
  } 
  RQprocessCount = RQprocessCount + ReadyQueue.size();
  schedule_arr(eve);
}

void process_preempt(struct event eve){

}

//////////////////////////////////////////////////
void FCFS(){
  struct event eve;
  while (processCount < 10000)
    {
      eve = EventQueue.top();
      switch (eve.type)
      {
      case ARRIVAL:
        process_arrival(eve);
        break;
      case DEPARTURE:
        process_departure(eve);
        break;
      default:
        //cout << "improper event type";
        exit(0);
        // error 
      }
    }
}

void SRTF(){
  struct event eve;
  while (processCount < 10000)
    {
      eve = EventQueue.top();
      switch (eve.type)
      {
      case ARRIVAL:
        process_arrival(eve);
        break;
      case DEPARTURE:
        process_departure(eve);
        break;
      case PREEMPT:
        process_preempt(eve);
      default:
        //cout << "improper event type";
        exit(0);
        // error 
      }
    }

}

////////////////////////////////////////////////////////////
int run_sim()
{
  switch (scheduler_type)
  {
  case 1:
    FCFS();
    break;
  case 2:
    SRTF();
    break;
  case 3:
    break;
  case 4:
    break;
  default:
    exit(0);
  }
  return 0;
}

////////////////////////////////////////////////////////////////
int main(int argc, char *argv[] )
{
  parse_arguments(argc, argv);
  
  init();
  run_sim(); 
  generate_report();
  return 0;
  
}
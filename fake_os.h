#include "fake_process.h"
#include "linked_list.h"
#pragma once
#define MAX_CPUS 10

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  int future_quantum;
  int current_time;
  int to_Schedule; 
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  /* FakePCB* running;*/ //sotistuisco il PCB del processo che controlla la CPU con un array
  int num_cores;
   FakePCB* running_processes[MAX_CPUS];
  ListHead ready;
  ListHead waiting;
  int timer;
  int cpu_index; //aggiunto per tenere traccia delle CPU sulle quali si agisce 
  ScheduleFn schedule_fn;
  void* schedule_args;
  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);

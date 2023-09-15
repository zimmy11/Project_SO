#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
  float decay;
} SchedSJFArgs;

void setQuantumPrediction(ListHead* ready, SchedSJFArgs* args){
    ListItem* first = ready->first;
    while(first){
       FakePCB* process = (FakePCB*)first;
       if(process->to_Schedule == 1){
       process->current_time=0;
       process->future_quantum = args -> decay *process->current_time +(1- args->decay)* process->future_quantum; // uso la formula per il SJF con quantum prediction
       process->to_Schedule=0;

    } 
    first = first->next;
    }   

}
//algortimo di scheduling
void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;
  setQuantumPrediction(&os->ready,args); //Aggiorno i quantum di ogni processo nella coda ready

  ListItem* aux= os->ready.first;
   
  FakePCB* ScheduledProcess=NULL;
  FakePCB* pcb;
  while(aux){
    
    pcb = (FakePCB*)aux;
    if(!ScheduledProcess || pcb->future_quantum < ScheduledProcess->future_quantum){
      ScheduledProcess = pcb;
    }
    aux=aux->next;
  }
   ScheduledProcess = (FakePCB*)List_detach(&os->ready, (ListItem*)ScheduledProcess);//rimuovo il processo scelto dalla coda ready
  os->running_processes[os->cpu_index]=ScheduledProcess;//lo assegno alla CPU libera
    assert(ScheduledProcess->events.first);
  ProcessEvent* e = (ProcessEvent*)ScheduledProcess->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>ScheduledProcess->future_quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=ScheduledProcess->future_quantum;
    e->duration-=ScheduledProcess->future_quantum;
    List_pushFront(&ScheduledProcess->events, (ListItem*)qe);
  }
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  srr_args.decay=0.5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d\n",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  int end_process=os.num_cores;
  while(end_process || os.ready.first
        || os.waiting.first
        || os.processes.first){
    end_process=os.num_cores;
    FakeOS_simStep(&os);
    for(int i=0; i<os.num_cores;i++){
      if(os.running_processes[i]==NULL)end_process--;
    }

  }
}

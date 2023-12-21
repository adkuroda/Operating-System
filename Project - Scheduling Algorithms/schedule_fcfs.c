/**
 * @file schedule_fcfs.c
 * @author Adilet Kuroda 
 * @brief This file contains the implementation of the FCFS scheduling algorithm
 *        for the list of tasks. It will schedule the tasks based on the order
 *        arrived. 
 * @date 2023-05-01
 * 
 */

#include "list.h"
#include "task.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Prototypes 
void add(char *name, int priority, int burst);
int str_cmp(char *a, char *b);
Task *pickNextTask();
void schedule();

// head of the list
struct node *head = NULL;

/** This add task to the utilizing insert function from list.c
 * @param name name of the task
 * @param priority priority of the task
 * @param burst burst of the task
 * 
*/
void add(char *name, int priority, int burst) {
    Task * task = malloc(sizeof(Task));
    if (task == NULL){ // if malloc fails
        printf("Error: malloc failed\n");
        return;
    }
    task->name = name;
    task->priority = priority;
    task->burst = burst;
    insert(&head, task);
}

/**
 * This function compares two strings. It was provided as 
 * part of the assignment. I made some changes to it.
 * Compares two strings and returns difference between them
 */
int str_cmp(char *a, char *b) { return strcmp(a, b); }

/**
 * This function picks the next task to run based on the FCFS
 * using str_cmp function. It will return the task with the
 * smallest name i.e the first task arrived. 
 * Note: I used the provided code from the assignment
 */
Task *pickNextTask() {
  // if list is empty, nothing to do
    if (!head){
      return NULL;
    }

    struct node *temp;
    temp = head;
    Task *best_sofar = temp->task;

    while (temp != NULL) {
      if (str_cmp(temp->task-> name, best_sofar->name) < 0)
        best_sofar = temp->task;
      temp = temp->next;
    }
    return best_sofar;
}

/**
 * This function schedules the tasks utilizing the FCFS algorithm
 * and the pickNextTask function. It will run the tasks based on the
 * order arrived.
 */
void schedule() {
    int time = 0;
    while(pickNextTask() != NULL){
        Task * next = pickNextTask();
        run(next, next->burst);
        time += next->burst;
        printf("        Time is now: %d\n", time);
        delete (&head, next);
    }
}

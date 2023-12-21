/**
 * @file schedule_sjf.c
 * @author Adilet Kuroda
 * @brief This is implementation of SJF scheduling algorithm. 
 *       It will schedule the tasks based on the shortest job first
 *      algorithm. if two tasks have the same burst time, it will
 *     schedule the task with the smallest name.
 * @date 2023-05-01
 */

#include "list.h"
#include "task.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


// Prototypes 
void add(char *name, int priority, int burst);
int cmp_burst(Task *rhs, Task *lhs);
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
 * Takes two tasks and compares their burst time. If they have the same
 * burst time, it will compare their names and return the difference
 * between them. If they have different burst time, it will return the
 * difference between their burst time.
 */
int cmp_burst(Task *rhs, Task *lhs) { 
   if (rhs->burst == lhs-> burst) {
      return str_cmp(rhs->name, lhs->name);
   } else{
      return rhs-> burst - lhs-> burst;
   }
}

/**
 * This function picks the next task to run based on the SJF
 * using cmp_burst function. It will return the task with the
 * smallest burst time. If two tasks have the same burst time,
 * it will return the task with the smallest name.
 * Note: I used the provided code from the assignment
 */
Task *pickNextTask() {
    // if list is empty, nothing to do
    if (!head)
      return NULL;

    struct node *temp;
    temp = head;
    Task *best_sofar = temp->task;

    while (temp != NULL) {
      if (cmp_burst(temp -> task, best_sofar)< 0)
        best_sofar = temp->task;
      temp = temp->next;
    }
    return best_sofar;
}

/**
 * This function schedules the tasks based on the SJF algorithm using
 * pickNextTask function. It will run the task with the smallest burst
 * time. If two tasks have the same burst time, it will run the task
 * with the smallest name.
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

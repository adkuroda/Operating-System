/**
 * @file schedule_priority.c
 * @author Adilet Kuroda
 * @brief This is implementation of Priority scheduling algorithm with 
 *        no preemption. It will schedule the tasks based on the priority
 *        of the task. If two tasks have the same priority, it will schedule
 *        the task with the smallest name.
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
int cmp_priority(Task *rhs, Task *lhs);
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
 * Takes two tasks and compares their priority. If they have the same
 * priority, it will compare their names and return the difference
 * between them. If they have different priority, it will return the
 * difference between their priority.
 */

int cmp_priority(Task *rhs, Task *lhs) { 
   if (rhs->priority == lhs-> priority) {
      return str_cmp(lhs->name, rhs->name);
   } else{
      return rhs-> priority - lhs-> priority;
   }
}

/**
 * Picks the next task based on the priority of the task. If two tasks
 * have the same priority, it will pick the task with the smallest name.
 * Note: this function provided as part of the assignment. 
 */
Task *pickNextTask() {
  // if list is empty, nothing to do
  if (!head)
    return NULL;

  struct node *temp;
  temp = head;
  Task *best_sofar = temp->task;

  while (temp != NULL) {
    if (cmp_priority(temp -> task, best_sofar) > 0)
      best_sofar = temp->task;
    temp = temp->next;
  }
  return best_sofar;
}

/**
 * Schedule the tasks based on the priority of the task using the
 * pickNextTask function. It will run the task for the duration of
 * the burst time and then delete the task from the list.
 */
void schedule() {
    int time = 0;
    while(pickNextTask() != NULL){
        Task * next = pickNextTask();
        run(next, next->burst);
        time += next->burst;
        printf("        Time is now: %d\n", time);
        delete(&head, next);
    }
    
}

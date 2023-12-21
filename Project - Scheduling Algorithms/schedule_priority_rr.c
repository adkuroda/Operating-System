/**
 * @file schedule_priority_rr.c
 * @author Adilet Kuroda
 * @brief This is implementation of Priority scheduling algorithm with
 *        Round Robin. It will schedule the tasks based on the priority
 *        and if two tasks have the same priority, it will schedule
 *       the task with the smallest name. Also, it will implement RR
 *     with time slice of 10 when two or more tasks have the same priority.
 * @date 2023-05-01
 */
#include "list.h"
#include "task.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Prototypes 
void add(char *name, int priority, int burst);
int cmp_priority(Task *rhs, Task *lhs);
int str_cmp(char *a, char *b);
Task* pickNextTaskHelper();
Task *pickNextTask();
void set_tail();

// head of the list
struct node *head = NULL;
// tail of the list helps to maintain order when implementing RR
struct node *tail = NULL;
// status of the task if there are more than one task with the same priority
int status = 0;

/**
 * add a task to the list. This will keep the list sorted
 * based on the priority of the task. This way, the task
 * with the highest prioritywill be at the front of the list
 * and will be scheduled first.
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

    struct node *ins_node = malloc(sizeof(struct node));
    if (ins_node == NULL){ // if malloc fails
        printf("Error: malloc failed\n");
        free(task);
        return;
    }
    ins_node->task = task;
    ins_node->next = NULL;

    if (head == NULL) { // if list is empty
        head = ins_node;
    } else { // if list is not empty
        struct node *cur = head;
        //check if the new node should be inserted at the beginning
        if (cmp_priority(ins_node -> task, cur-> task) > 0){
            ins_node -> next = cur;
            head = ins_node;   
        // if not, find the correct position to insert the node
        }else{
            // find the correct position
            while(cur -> next != NULL && (cmp_priority(ins_node-> task, cur -> next -> task) < 0) )
            {
                cur = cur -> next;
            }
            ins_node -> next = cur -> next;
            cur -> next = ins_node;
        }
    }
}
/**
 * @brief This is a utility function that helps to maintain the order
 * when implementing RR. It moves the task with the same priority to the
 * end of the task with the same priority. This way, the task with the
 * smallest name will be scheduled first and maintain the order when 
 * implementing RR.
 */
Task* pickNextTaskHelper(){
    struct node * cur = head;
    int priority = head -> task -> priority;
    // find the last element with the same priority
    while (cur -> next != NULL && cur -> next -> task -> priority == priority){
        cur = cur -> next;
    }
    if (cur -> next == NULL){
        struct node * temp = head;
        head = head -> next;
        tail -> next = temp;
        tail = temp;
        tail -> next = NULL;
        return temp -> task;
    }else{
        struct node * temp = head;
        head = head -> next;
        temp -> next = cur -> next;
        cur -> next = temp;
        return temp -> task;
    }
}

Task *pickNextTask() {
  // if list is empty, nothing to do
    if (head == NULL){
        return NULL;
    }
    // if the task is less than or equal to quantum
    // schedule the task and remove it from the list
    if (head -> task -> burst <= QUANTUM){
        status = 1;
        return head -> task;
    }
    // it is last element in the list and it is greater than quantum
    // schedule the task and remove it from the list when quantum is over
    if (head -> next == NULL){
        status = 1;
        return head -> task;
    }
    int priority = head -> task -> priority;
    // we do not have to do round robin if the next task has different priority
    if (head -> next -> task -> priority != priority){
        status = 1;
        return head -> task;
    }else{
        // if the next task has the same priority, we have to do round robin
        return pickNextTaskHelper();
    }
}

/**
 *  Set the tail of the list. This will help to maintain the order. 
 * 
 */
void set_tail(){
    struct node *temp = head;
    while (temp->next != NULL){
        temp = temp->next;
    }
    tail = temp;
}

/**
 * Schedule the tasks in the list. If there are more than one task with the same
 * priority, it will run task only for 10 ms and then switch to the next task.
 * Otherwise, it will run the task until it is finished.
 */
void schedule(){
    set_tail();
    int time = 0;
    Task * next;
    while ((next = pickNextTask()) != NULL){
        if(status == 1){
            status = 0;
            run(next, next -> burst);
            time += next -> burst;
            delete(&head, next);
        }else{
            if (next -> burst > QUANTUM){
                run(next, QUANTUM);
                next -> burst -= QUANTUM;
                time += QUANTUM;
            }else{
                run(next, next -> burst);
                time += next -> burst;
                delete(&head, next);
            }
        }
        printf("        Time is now: %d\n", time); 
    }
}

/**
 * This is a utility function that compares two strings.
 */
int str_cmp(char *a, char *b) { return strcmp(a, b); }

/**
 * This is a utility function that compares two tasks based on their priority.
 * If two tasks have the same priority, it will compare their names.
 */
int cmp_priority(Task *rhs, Task *lhs) { 
   if (rhs->priority == lhs-> priority) {
      return str_cmp(lhs->name, rhs->name);
   } else{
      return rhs-> priority - lhs-> priority;
   }
}


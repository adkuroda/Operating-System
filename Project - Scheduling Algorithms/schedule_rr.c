
/**
 * @file schedule_rr.c
 * @author Adilet Kuroda
 * @brief: This file contains the implementation of Round Robin 
 *        scheduling algorithm. It will schedule the tasks based
 *        alphabetically. 
 * @date 2023-04-21
 */

#include "list.h"
#include "task.h"
#include "cpu.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


// Prototypes 
int str_cmp(struct node * rhs, struct node *lhs);
void add(char *name, int priority, int burst);
Task *pickNextTask();
void schedule();
void set_tail();

// head of the list 
struct node *head = NULL;
// tail of the list helps to maintain order when implementing RR
struct node *tail = NULL;

/**
 * This function compares two strings. It was provided as 
 * part of the assignment. I made some changes to it.
 * Compares two strings and returns difference between them
 */

int str_cmp(struct node * rhs, struct node *lhs){ 
    char * a = rhs->task->name;
    char * b = lhs->task->name;
    return strcmp(a, b); 
}
/**
 *  Once task is added to the list, this function will set tail
 *  to the last node of the list. This will help to maintain order
 * when implementing RR
 */
void set_tail(){
    struct node *temp = head;
    while (temp->next != NULL){
        temp = temp->next;
    }
    tail = temp;
}
/**
 * @brief This add task to the list in alphabetical order to maintain
 *       order when implementing RR. It will utilize str_cmp function
 *       to compare two tasks.
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
    struct node *new_node = malloc(sizeof(struct node));
    if (new_node == NULL){ // if malloc fails
        free(task);
        printf("Error: malloc failed\n");
        return;
    }
    new_node->task = task;
    new_node->next = NULL;
    if (head == NULL){ // if list is empty
        head = new_node;
    }
    else{
        if (str_cmp(new_node, head) < 0){ // if new node is less than head
            new_node->next = head;
            head = new_node;
        }
        else{
            struct node *temp = head;
            while (temp->next != NULL && str_cmp(new_node, temp->next) > 0){
                temp = temp->next;
            }
            new_node->next = temp->next;
            temp->next = new_node;
        }
    }
}

/**
 * This function returns the next task to be executed which 
 * is the head of the list.
 * 
 */
Task *pickNextTask() {
  // if list is empty, nothing to do
    if (head == NULL){
        return NULL;
    }else{
        return head->task;
    }

}

/**
 * Schedule the tasks in the list. If the task is not finished
 * after QUANTUM, it will be added to the end of the list.
 * Once the task is finished, it will be deleted from the list.
 */
void schedule() {
    int time = 0;
    set_tail();
    Task * next;
    while ((next = pickNextTask()) != NULL){
        // if task is not finished and it is the only task in the list
        if (next -> burst > QUANTUM && head != tail){
            run(next, QUANTUM);
            next -> burst -= QUANTUM;
            time += QUANTUM;
            struct node *temp = head;
            head = head -> next;
            temp->next = NULL;
            tail -> next = temp;
            tail = temp;
        }
        else{ // if task is finished
            run(next, next -> burst);
            time += next -> burst;
            delete(&head, next);
        }
         printf("        Time is now: %d\n", time); 
    }
}

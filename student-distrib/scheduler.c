#include "scheduler.h"


terminal_t* tasks[MAXTERMS] = {NULL, NULL, NULL}; // our "list"
int current_task_idx = 0;

/*  void rtc_interrupt()
    function: Switch the task when irq 0 interrupt happen
     input: (all unused)
    returns: 0 once the RTC interrupts again
*/

void switch_task(){
    // find another active task to work on
    int active_found = -1;
    int new_task_idx = (current_task_idx + 1) % MAXTERMS;
    int i;
    for(i = 0; i < MAXTERMS; i++){
        if(tasks[new_task_idx] == NULL || tasks[new_task_idx]->active == 0)
            new_task_idx = (new_task_idx+1) % MAXTERMS;
        else{
            active_found = 1;
            break;
        }
    }

    // skip the rest of this function if we didn't find a new active task
    if(active_found == -1 || new_task_idx == current_task_idx){
        send_eoi(0);
        return;
    }

    // copy registers to current old task
    tasks[current_task_idx]->esp = esp_0;
    tasks[current_task_idx]->ebp = ebp_0;
    tasks[current_task_idx]->kernel_esp = tss.esp0;

    uint32_t cr3;
    asm volatile ("movl %%CR3, %0"
                     : "=b"(cr3));
    tasks[current_task_idx]->cr3 = cr3;

    current_task_idx = new_task_idx;
    //current_task_idx = (current_task_idx+1) % MAXTERMS;
    // traverse list
    /*if(tasks[current_task_idx].next != NULL)
        current_task = tasks[current_task_idx].next;
    else
        current_task = queue_head;
    */
    // copy current task regiters to saved registers TODO
    //send_eoi(0);
    tss.esp0 = MB_8 - KB_8*(tasks[current_task_idx]->program_id - 1) - PREV;//entry_point;
   //  tss.esp0 = terminals[term].kernel_esp;
    tss.ss0 = KERNEL_DS;
     // update esp, ebp, cr3, tss.esp0, tss.ss0
    asm volatile("movl %0, %%cr3" : : "r" (tasks[current_task_idx]->cr3));
    //asm volatile("movl %0, %%ebp" : : "r" (tasks[current_task_idx]->ebp));
    //asm volatile("movl %0, %%esp" : : "r" (tasks[current_task_idx]->esp));
    send_eoi(0);

    // switch page directory to current task cr3
   int_ret_helper(tasks[current_task_idx]->ebp, tasks[current_task_idx]->esp);

}

/*  add_task_to_queue(terminal_t* task_add)
    function: Add terminal on to the run queue
     input: pointer to terminal to add to the queue
    returns: 0 once the RTC interrupts again
*/


void add_task_to_queue(terminal_t* task_add){
    if(task_add == NULL)
        return;

    //task_add->next = NULL;
    //taskw_t temp = {task_add, NULL};


    // find a free spot in the scheduler, starting from the current idx
    // don't add task if no free spot
    int i;
    for(i = 0; i < MAXTERMS; i++){
        int temp_idx = (current_task_idx+i) % MAXTERMS;
        if(tasks[temp_idx] == NULL){
            tasks[temp_idx] = task_add;
            break;
        }
    }

    //if(queue_head == NULL){
    //    queue_head = &temp;
    //}
    /*
    else{
        pcb_t* task_ptr = queue_head;
        // traverse the list looking for the task before the parameter task,
        // stop right before NULL
        while(task_ptr->next != NULL){
            task_ptr = task_ptr->next;
        }
    
        task_ptr->next = &temp;
    }*/

}

/*  remove_task_from_queue(terminal_t* task_rm)
    function: Remove terminal from the run queue
     input: pointer to terminal to add to the queue
    returns: 0 once the RTC interrupts again
*/

void remove_task_from_queue(terminal_t* task_rm){
    if(task_rm == NULL)
        return;

    // find the task to remove in the tasks array
    // does not remove anything if not found
    int i;
    for(i = 0; i < MAXTERMS; i++){
        if(tasks[i] == task_rm){
            tasks[i] = NULL;
            break;
        }
    }

    /*
    if(queue_head->term == task_rm){ // on remove head task and that's it
        queue_head = queue_head->next;
        return;
    }

    pcb_t* task_ptr = queue_head;
    // traverse the list looking for the task before the parameter task,
    // stop either right before parameter task or right before NULL
    while(task_ptr->next != NULL && task_ptr->next != task_rm){
        task_ptr = task_ptr->next;
    }

    pcb_t* task_next = task_ptr->next;
    // if the next task is the parameter task (instead of NULL), point the next ptr of its predecessor
    // to the parameter task's successor
    // and also point the parameter task's next ptr to null
    if(task_next->term == task_rm){
        task_ptr->next = task_next->next;
        task_next->next = NULL;
    }
    */
}

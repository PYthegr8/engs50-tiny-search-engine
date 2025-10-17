/*
Implementation of list.c module
Engs050 Module 3
Team members: Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"

static car_t *front = NULL;
static int size = 0;

int32_t lput(car_t *cp);
car_t *lget();
void lapply(void (*fn)(car_t *cp));
car_t *lremove(char *platep); 
int32_t list_size(void);
void print_list(void);
void car_free(car_t *cp);
void list_free(void);

/*
This function inserts a car structure at the front of our list of cars.
It returns 0 if the operation was successful and non-zero otherwise.
*/
int32_t lput(car_t *cp) {
    if (cp == NULL) {
        fprintf(stderr, "Invalid car pointer passed to the function\n");
        return 1;
    }
    cp->next = front;
    front = cp;
    size++;
    return 0;
}

/*
This function gets the first car in the list of car structures.
In the case that the list is empty, we return NULL.
*/
car_t *lget() {
    if (front == NULL) {
        return NULL;
    }
    car_t *car_to_return = front;
    front = front->next;
    size--;
    return car_to_return;
}

/*
This function applies a specific function passed in as a paremeter to all 
of the cars present in the list of cars.
*/
void lapply(void (*fn)(car_t *cp)) {
    car_t *walker = front;
    while (walker != NULL) {
        fn(walker);
        walker = walker->next;
    } 
}

/*
This function removes the car whose plateID corresponds to the plateID 
passed in as a parameter. If no such car exists, we return NULL.
*/
car_t *lremove(char *platep) {
    car_t *prev = NULL;
    car_t *walker = front;
    while (walker != NULL) {
        if (!strcmp(walker->plate, platep)) {
            if (prev == NULL){
                front = front->next;
            }
            else {
                prev->next = walker->next;
            }
            walker->next = NULL;
            size--;
            return walker;
        }
        prev = walker;
        walker = walker->next;
    }
    return NULL;
}

int32_t list_size(void) {
    return size;
}

void print_list(void) {
    car_t *walker = front;
    if (!walker) {
        printf("List is empty.\n");
        return;
    }

    printf("Cars in list:\n");
    while (walker != NULL) {
        printf("Car: %s, Price: %.2f, Year: %d\n", walker->plate, walker->price, walker->year);
        walker = walker->next;
    }
}

void car_free(car_t *cp) {
    if (cp != NULL) {
        free(cp);
    }
}

void list_free(void) {
    car_t *walker = front;
    while (walker != NULL) {
        car_t *next_car = walker->next;
        car_free(walker);
        walker = next_car;
    }
    front = NULL;
    size = 0;
}

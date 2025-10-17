/*
Test file for the implementation of a singly linked list of car structures.
Engs050 Module 3
Team members: Gent Maksutaj, Papa Yaw Owusu Nti, Benjamin Rippy
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void test_lput();
void test_lget();
void test_lapply();
void test_lremove();
car_t *car_init(char *platep, double price, int year); 
void car_free(car_t *cp);
int32_t list_size(void);
void print_list(void);
void turn_ferrari(car_t *cp);
void list_free(void);

int main() {
    test_lput(1);
    test_lget();
    test_lapply();
    test_lremove();
    list_free();
    return 0;
}

void test_lput(int verbose) {
    printf("Running test_lput\n");
    printf("Initial list size is: %d\n", list_size());

    // creating 3 cars for testing
    car_t *c1 = car_init("Toyota", 20000.0, 2020);
    car_t *c2 = car_init("Honda", 22000.0, 2021);
    car_t *c3 = car_init("Ford", 18000.0, 2019);

    lput(c1);
    lput(c2);
    lput(c3);

    // checking list size after inserts
    printf("List size after 3 lput() calls: %d\n", list_size());

    //printing contents of list
    if (verbose) {
        print_list();
    }
    printf("test_lput finished.\n");
    printf("\n");
    printf("\n");
    printf("\n");
}


void test_lget(int verbose) {
    printf("Running test_lget\n");

    // Getting the k cars inside the list
    int size = list_size();
    printf("Initially the list of cars looks like: \n");
    print_list();
    while (size--) {
        printf("Getting the car from the front of the queue\n");
        car_t *front_car = lget();
        car_free(front_car);
        printf("After getting the car in the front of the list, the list looks like: \n");
        print_list();
    }

    // Trying to get an element from an empty list of cars
		printf("Calling lget on the empty list:\n");
    assert(lget() == NULL);
		print_list();
		printf("No error; lget() returned NULL\n");
    printf("\n");
    printf("\n");
    printf("\n");
}

void test_lapply(int verbose) {
    printf("Running test_lapply\n");
    printf("Lets turn all the cars in our list into ferraris\n");

    car_t *c1 = car_init("Toyota", 20000.0, 2020);
    car_t *c2 = car_init("Honda", 22000.0, 2021);
    car_t *c3 = car_init("Ford", 18000.0, 2019);

    lput(c1);
    lput(c2);
    lput(c3);

    printf("initially our list looks like this: \n");
    print_list();
    lapply(turn_ferrari);
    printf("After applying our function to all of the cars in the lists, we get: \n");
    print_list();

		printf("Apply the function to an empty list: first, clear the list.\n");
		list_free();
		print_list();
		printf("Call lapply():\n");
		lapply(turn_ferrari);
		print_list();
		printf("No effect.\n");
    printf("\n");
    printf("\n");
    printf("\n");
}

void test_lremove(int verbose) {
	  car_t *c1 = car_init("Toyota", 20000.0, 2020);                                                     
    car_t *c2 = car_init("Honda", 22000.0, 2021);                                                      
    car_t *c3 = car_init("Ford", 18000.0, 2019); 
    lput(c1);                                                                                          
    lput(c2);                                                                                          
    lput(c3);
		
		printf("Running test_lremove\n");

    printf("Lets add a lambo to our list and then remove it based on the name\n");
    car_t *lambo = car_init("Lambo", 199999.9, 2023);
    lput(lambo);

    printf("\nThe list now looks like: \n");
    print_list();
    car_t *dispose = lremove("Lambo");
    printf("After lremove we get the following: \n");
    print_list();

    car_free(dispose);

		printf("\nRemove the car in the middle:\n");
		dispose = lremove("Honda");
		printf("After removing:\n");
		print_list();
		
		car_free(dispose);

		printf("\nRemove the car at the end:\n");
		dispose = lremove("Toyota");
		printf("After removing:\n");
		print_list();
		car_free(dispose);

		printf("\nCall lremove on an empty list: First clear the list \n");
		list_free();
		print_list();
		dispose = lremove("Toyota");
		printf("After removing:\n");
		print_list();
		
    printf("\n");
    printf("\n");
    printf("\n");
}

car_t *car_init(char *platep, double price, int year) {
    car_t *cp;

    if (!(cp = (car_t*)malloc(sizeof(car_t)))) {
        fprintf(stderr, "[Error: Malloc failed allocating car!\n]");
        return NULL;
    }

    cp->next = NULL;
    strcpy(cp->plate, platep);
    cp->price = price;
    cp->year = year;
    return cp;
}

void turn_ferrari(car_t *cp) {
    if (cp != NULL) {
        strcpy(cp->plate, "Ferrari");
    }
}

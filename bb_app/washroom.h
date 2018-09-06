/*
 * 
 */
#ifndef WASHROOM_H
#define WASHROOM_H

void Washroom_start(void);
void Washroom_stop(void);

void Washroom_setTotalStalls(int value);
void Washroom_setNumOpenStalls(int value);
_Bool Washroom_verifyPassword(int password);

int Washroom_getTotalStallCount(void);
int Washroom_getOpenStallCount(void);

int Washroom_reserveStall(void);
void Washroom_setStallMaintenance(int num);
void Washroom_releaseStallMaintenance(int num);

//Pass in an array of size Washroom_getTotalStallCount()
//Postcondition: array is filled with stall states
void Washroom_getStallStates(int* array);

#endif
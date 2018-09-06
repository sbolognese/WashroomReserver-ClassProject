/*
 * 
 */
#ifndef STALL_H
#define STALL_H

void Stall_start(void);
void Stall_stop(void);

int Stall_getTotalStalls(void);

int Stall_reserveStall(void);
void Stall_getStallStates(int* array);
void Stall_setStallMaintenance(int stallNum);
void Stall_releaseStallMaintenance(int num);

#endif
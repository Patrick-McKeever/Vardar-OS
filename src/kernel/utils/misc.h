#ifndef MISC_H
#define MISC_H

#include <stdint.h>

void SetNthBit(uint64_t *num, uint16_t n);
void ClearNthBit(uint64_t *num, uint16_t n);
uint64_t RoundToNearestMultiple(uint64_t num, uint32_t multiple);
int GetNthBit(uint64_t num, uint16_t n);
unsigned long long Floor(long double n);
unsigned long long Ceil(long double n);
double DMod(double x, double y);
void Reverse(char str[], int len);
void Itoa(int n, char *buff);


#endif


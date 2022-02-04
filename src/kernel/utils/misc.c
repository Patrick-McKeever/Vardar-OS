#include <stdbool.h>
#include <limits.h>
#include "misc.h"

void SetNthBit(uint64_t *num, uint16_t n)
{
    *(num) |= (1 << n) ;
}

void ClearNthBit(uint64_t *num, uint16_t n)
{
	*(num) &= ~(1UL << n);
}

int GetNthBit(uint64_t num, uint16_t n)
{
    return num & (1 << n);
}

//unsigned long long Floor(long double num)
//{
//	if(/*num >= LLONG_MAX || num <= LLONG_MIN || */num != num) {
//		/* handle large values, infinities and nan */
//		return num;
//	}
//	long long n = (long long)num;
//	double d = (double)num;
//	if (d == num || num >= 0) {
//		return d;
//    }
//	return d - 1;
//}
//
//unsigned long long Ceil(long double num)
//{
//	return num + (DMod(num, 1) != 0);
//}
//
//double DMod(double x, double y) {
//	return x - (int)(x/y) * y;
//}

void Reverse(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
		char temp = *(str + start);
		*(str + start) = *(str + end);
		*(str + end) = temp;
        start++;
        end--;
    }
}

void Itoa(int n, char *buff)
{
	int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if (n == 0)
    {
        buff[i++] = '0';
        buff[i] = '\0';
        return;
    }
 
    // In standard itoa(), negative nbers are handled only with
    // base 10. Otherwise nbers are considered unsigned.
    if (n < 0)
    {
        isNegative = true;
        n = -n;
    }
 
    // Process individual digits
    while (n != 0)
    {
        int rem = n % 10;
        buff[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        n = n/10;
    }
 
    // If nber is negative, append '-'
    if (isNegative)
        buff[i++] = '-';
 
    buff[i] = '\0'; // Append string terminator
 
    // Reverse the string
    Reverse(buff, i);
 
    return;
}


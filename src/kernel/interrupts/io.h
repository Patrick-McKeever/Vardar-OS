#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * Read a byte from the specified port.
 * @input port The port from which to read.
 * @return The byte read.
 */
uint8_t inportb(uint16_t port);

/**
 * Write a byte to the given port.
 * @input port The port to which we will write.
 * @input byte The byte to write to the port.
 */
void outportb(uint16_t port, uint8_t byte);

/**
 * Wait some short period of time by contracting unsued
 * port and waiting for response.
 */
void io_wait();

#endif

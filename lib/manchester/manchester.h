#ifndef __S5APP6_MANCHESTER_H
#define __S5APP6_MANCHESTER_H

/**
 * Takes a bit and return the signal that needs to be send as manchester.
 *
 * @param char Must be either 1 or 0.
 * @return char[2] The 2-bits values that must be sent.
 * */
void bit_to_write(char *, char);

/**
 * Takes a 2-bits manchester representation, makes sure it is valid, and return the
 * bit value. If it is not valid, return -1.
 *
 * @param char[2] The 2-bits values of the manchester representation.
 * @return char Valid 0 or 1, or -1 if invalid.
 * */
char bit_to_read(char[2]);


void to_manchester(char[2], char);

#endif
/* 
 * 
 * 
 */

#ifndef ENCODING_H
#define ENCODING_H

#include <gmp.h>

// Defaults for encoding

#define MOD_BITS 128
#define FRAC_BITS 32

// Encoding for Paillier with single multiplication (assumes mpz already initialised)

void encode_from_dbl(mpz_t res, double x, unsigned int mod_bits, unsigned int frac_bits);
double decode_to_dbl(mpz_t e, int mult, unsigned int mod_bits, unsigned int frac_bits);

// Encoding for ORE

unsigned long int encode_dbl_cmp(double x, unsigned int mod_bits, unsigned int frac_bits);

#endif
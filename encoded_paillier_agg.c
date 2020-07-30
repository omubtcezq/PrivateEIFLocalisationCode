/*
 * 
 */

#include "encoded_paillier_agg.h"
#include "libpaillier-0.8/paillier.h"

// Local
void init_rand_agg(gmp_randstate_t rnd, paillier_get_rand_t get_rand, int bytes);


// 888    d8P
// 888   d8P
// 888  d8P
// 888d88K      .d88b.  888  888  .d88b.   .d88b.  88888b.
// 8888888b    d8P  Y8b 888  888 d88P"88b d8P  Y8b 888 "88b
// 888  Y88b   88888888 888  888 888  888 88888888 888  888
// 888   Y88b  Y8b.     Y88b 888 Y88b 888 Y8b.     888  888
// 888    Y88b  "Y8888   "Y88888  "Y88888  "Y8888  888  888
//                           888      888
//                      Y8b d88P Y8b d88P
//                       "Y88P"   "Y88P"

// Use wrapped paillier library to genenrate keys
void key_gen(int paillier_bitsize, pubkey_t **pubkey, prvkey_t **prvkey){
    paillier_keygen(paillier_bitsize, pubkey, prvkey, paillier_get_rand_devrandom);
}

// Generate aggregation keys given the paillier public key parameters
void agg_key_gen(pubkey_t *pubkey, int participants, aggkey_t *key_array){
    // Init and allocate memory for last key (used during generation of others)
    mpz_init(key_array[participants-1]);

    // Required for large random number generation. Copied from paillier library
    gmp_randstate_t rnd;
    init_rand_agg(rnd, paillier_get_rand_devrandom, (pubkey->bits)*2 / 8 + 1);

    // Generate all but one key as random values less than the Paillier modulus
    for (int s=0; s<participants-1; s++){
        // Init and allocate memory for each key
        mpz_init(key_array[s]);

        // Generate random key
        do
            mpz_urandomb(key_array[s], rnd, (pubkey->bits)*2);
        while(mpz_cmp(key_array[s], pubkey->n_squared) >= 0);

        // Keep track of sum of keys
        mpz_add(key_array[participants-1], key_array[participants-1], key_array[s]);
    }

    // Generate the final key, making them all sum to 0
    mpz_neg(key_array[participants-1], key_array[participants-1]);
    mpz_mod(key_array[participants-1], key_array[participants-1], pubkey->n_squared);
}

// 888b     d888 8888888b. 8888888                                d8b          888 d8b                   888    d8b
// 8888b   d8888 888   Y88b  888                                  Y8P          888 Y8P                   888    Y8P
// 88888b.d88888 888    888  888                                               888                       888
// 888Y88888P888 888   d88P  888        .d8888b   .d88b.  888d888 888  8888b.  888 888 .d8888b   8888b.  888888 888  .d88b.  88888b.
// 888 Y888P 888 8888888P"   888        88K      d8P  Y8b 888P"   888     "88b 888 888 88K          "88b 888    888 d88""88b 888 "88b
// 888  Y8P  888 888         888        "Y8888b. 88888888 888     888 .d888888 888 888 "Y8888b. .d888888 888    888 888  888 888  888
// 888   "   888 888         888             X88 Y8b.     888     888 888  888 888 888      X88 888  888 Y88b.  888 Y88..88P 888  888
// 888       888 888       8888888       88888P'  "Y8888  888     888 "Y888888 888 888  88888P' "Y888888  "Y888 888  "Y88P"  888  888




// Serialisation of paillier public key for MPI communication
// Does not allocate buffer
void serialise_pubkey(pubkey_t *pubkey, char *buffer){
    mpz_get_str(buffer, SERIALISATION_BASE, pubkey->n);
}

// Deserialisation of paillier public key for MPI communcation
// Allocates pubkey
pubkey_t* deserialise_pubkey(char *key_serialisation){
    return paillier_pubkey_from_hex(key_serialisation);
}

// Serialisation of aggregation private key for MPI communication
// Does not allocates buffer
void serialise_aggkey(aggkey_t aggkey, char *buffer){
    mpz_get_str(buffer, SERIALISATION_BASE, aggkey);
}

// Deserialisation of aggregation private key for MPI communication
// Allocates aggkey
void deserialise_aggkey(aggkey_t aggkey, char *aggkey_serialisation){
    mpz_init_set_str(aggkey, aggkey_serialisation, SERIALISATION_BASE);
}

// Serialisation of encryptions for MPI communication
// Does not allocates buffer
void serialise_encryption(ciphertext_t *ct, char *buffer){
    mpz_get_str(buffer, SERIALISATION_BASE, ct->c);
}

// Deserialisation of encryptions for MPI communication
// Allocates ciphertext
ciphertext_t* deserialise_encryption(char *encryption_serialisation){
    ciphertext_t *ct;
    ct = paillier_create_enc_zero();
    mpz_set_str(ct->c, encryption_serialisation, SERIALISATION_BASE);
    return ct;
}

// 8888888          d8b 888
//   888            Y8P 888
//   888                888
//   888   88888b.  888 888888
//   888   888 "88b 888 888
//   888   888  888 888 888
//   888   888  888 888 Y88b.
// 8888888 888  888 888  "Y888




ciphertext_t* init_ciphertext(){
    return paillier_create_enc_zero();
}

// 888    888                                                                         888      d8b
// 888    888                                                                         888      Y8P
// 888    888                                                                         888
// 8888888888  .d88b.  88888b.d88b.   .d88b.  88888b.d88b.   .d88b.  888d888 88888b.  88888b.  888  .d8888b       .d88b.  88888b.  .d8888b
// 888    888 d88""88b 888 "888 "88b d88""88b 888 "888 "88b d88""88b 888P"   888 "88b 888 "88b 888 d88P"         d88""88b 888 "88b 88K
// 888    888 888  888 888  888  888 888  888 888  888  888 888  888 888     888  888 888  888 888 888           888  888 888  888 "Y8888b.
// 888    888 Y88..88P 888  888  888 Y88..88P 888  888  888 Y88..88P 888     888 d88P 888  888 888 Y88b.         Y88..88P 888 d88P      X88
// 888    888  "Y88P"  888  888  888  "Y88P"  888  888  888  "Y88P"  888     88888P"  888  888 888  "Y8888P       "Y88P"  88888P"   88888P'
//                                                                           888                                          888
//                                                                           888                                          888
//                                                                           888                                          888

// Encode and encrypt a float, given public key and number of multiplications for encoding
void encode_and_enc(pubkey_t *pubkey, ciphertext_t *res, double a, unsigned int mults){
    paillier_plaintext_t *p_text = paillier_plaintext_from_ui(0);
    encode_from_dbl(p_text->m, a, mults, MOD_BITS, FRAC_BITS);
    paillier_enc(res, pubkey, p_text, paillier_get_rand_devurandom);
    paillier_freeplaintext(p_text);
}

// Decrypt and decode encryption to a float given necessary keys and the number of multiplications for decoding
double dec_and_decode(pubkey_t *pubkey, prvkey_t *prvkey, ciphertext_t *ct, unsigned int mults){
    return 0.0;
}

void encode_and_add_enc(pubkey_t *pubkey, ciphertext_t *res, ciphertext_t *ct, double a, unsigned int mults){

}

void encode_and_mult_enc(pubkey_t *pubkey, ciphertext_t *res, ciphertext_t *ct, double a, unsigned int mults){

}

void add_encs(pubkey_t *pubkey, ciphertext_t *res, ciphertext_t *ct1, ciphertext_t *ct2){

}

//        d8888                                                     888    d8b
//       d88888                                                     888    Y8P
//      d88P888                                                     888
//     d88P 888  .d88b.   .d88b.  888d888 .d88b.   .d88b.   8888b.  888888 888  .d88b.  88888b.
//    d88P  888 d88P"88b d88P"88b 888P"  d8P  Y8b d88P"88b     "88b 888    888 d88""88b 888 "88b
//   d88P   888 888  888 888  888 888    88888888 888  888 .d888888 888    888 888  888 888  888
//  d8888888888 Y88b 888 Y88b 888 888    Y8b.     Y88b 888 888  888 Y88b.  888 Y88..88P 888  888
// d88P     888  "Y88888  "Y88888 888     "Y8888   "Y88888 "Y888888  "Y888 888  "Y88P"  888  888
//                   888      888                      888
//              Y8b d88P Y8b d88P                 Y8b d88P
//               "Y88P"   "Y88P"                   "Y88P"

void add_agg_noise(pubkey_t *pubkey, ciphertext_t *res, ciphertext_t *ct, int stamp){

}

// 8888888888
// 888
// 888
// 8888888 888d888 .d88b.   .d88b.       88888b.d88b.   .d88b.  88888b.d88b.   .d88b.  888d888 888  888
// 888     888P"  d8P  Y8b d8P  Y8b      888 "888 "88b d8P  Y8b 888 "888 "88b d88""88b 888P"   888  888
// 888     888    88888888 88888888      888  888  888 88888888 888  888  888 888  888 888     888  888
// 888     888    Y8b.     Y8b.          888  888  888 Y8b.     888  888  888 Y88..88P 888     Y88b 888
// 888     888     "Y8888   "Y8888       888  888  888  "Y8888  888  888  888  "Y88P"  888      "Y88888
//                                                                                                  888
//                                                                                             Y8b d88P
//                                                                                              "Y88P"

void free_pubkey(pubkey_t *pubkey){
    paillier_freepubkey(pubkey);
}

void free_prvkey(prvkey_t *prvkey){
    paillier_freeprvkey(prvkey);
}

void free_aggkey(aggkey_t aggkey){
    mpz_clear(aggkey);
}

void free_ciphertext(ciphertext_t *ct){
    paillier_freeciphertext(ct);
}

// 888                                888
// 888                                888
// 888                                888
// 888      .d88b.   .d8888b  8888b.  888
// 888     d88""88b d88P"        "88b 888
// 888     888  888 888      .d888888 888
// 888     Y88..88P Y88b.    888  888 888
// 88888888 "Y88P"   "Y8888P "Y888888 888




// Copied from libpaillier-X.X library for getting random mpz values for aggregation
void init_rand_agg(gmp_randstate_t rnd, paillier_get_rand_t get_rand, int bytes){
	void* buf;
	mpz_t s;

	buf = malloc(bytes);
	get_rand(buf, bytes);

	gmp_randinit_default(rnd);
	mpz_init(s);
	mpz_import(s, bytes, 1, 1, 0, 0, buf);
	gmp_randseed(rnd, s);
	mpz_clear(s);

	free(buf);
}
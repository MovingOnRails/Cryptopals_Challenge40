#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

void get_RSA_prime(mpz_t p, mpz_t e, gmp_randstate_t state, unsigned long bits){
    mpz_t gcd_a, p1, a;
    mpz_inits(gcd_a, p1, a, NULL);

    mpz_set_ui(gcd_a, 0);
    while(mpz_cmp_ui(gcd_a,1) != 0){
        mpz_set_ui(a, 0);
        while(mpz_cmp_ui(a, 0) == 0){
            mpz_urandomb(a, state, bits);
        }
        mpz_nextprime(p, a);
        mpz_sub_ui(p1, p, 1);
        mpz_gcd(gcd_a,p1,e);
    }

    mpz_clears(gcd_a, p1, a, NULL);
}

void get_RSA_totient(mpz_t et, mpz_t p, mpz_t q){
    mpz_t term1, term2;
    mpz_inits(term1, term2, NULL);
    mpz_sub_ui(term1, p, 1);
    mpz_sub_ui(term2, q, 1);
    mpz_mul(et, term1, term2);
    mpz_clears(term1, term2, NULL);
}

void get_CRT_term(mpz_t term0, mpz_t c_this, mpz_t n_this, mpz_t n_other1, mpz_t n_other2){
    mpz_t ms, inv;
    mpz_inits(ms, inv, NULL);
    mpz_mul(ms, n_other1, n_other2);
    mpz_invert(inv, ms, n_this);
    mpz_mul(term0, ms, inv);
    mpz_mul(term0, term0, c_this);
}

int main(){

    mpz_t  e, p0, q0, p1, q1, p2, q2, n0, n1, n2, d, c0, c1, c2, m, m_deciphered;
    gmp_randstate_t state;

    mpz_inits(e, p0, q0, p1, q1, p2, q2, n0, n1, n2, d, c0, c1, c2, m, m_deciphered, NULL);

    gmp_randinit_default(state);
    gmp_randseed_ui(state, 12345);

    unsigned long bits = 1024;

    mpz_set_ui(e, 3);    
    
    get_RSA_prime(p0, e, state, bits);
    get_RSA_prime(q0, e, state, bits);
    mpz_mul(n0, p0, q0);
    get_RSA_prime(p1, e, state, bits);
    get_RSA_prime(q1, e, state, bits);
    mpz_mul(n1, p1, q1);
    get_RSA_prime(p2, e, state, bits);
    get_RSA_prime(q2, e, state, bits);
    mpz_mul(n2, p2, q2);

    // --------------------Encrypt the message x 3--------------------
    const char message[17] = "This is a test\n";
    char message_hex[33];
    for(int i=0;i<16;i++){
        snprintf(message_hex+i*2, 3, "%02x", message[i]);
    }
    mpz_set_str(m, message_hex, 16);
    mpz_powm(c0, m, e, n0);
    mpz_powm(c1, m, e, n1);
    mpz_powm(c2, m, e, n2);


    mpz_t result, term0, term1, term2, N012;
    mpz_inits(result, term0, term1, term2, N012, NULL);
    mpz_mul(N012, n0, n1);
    mpz_mul(N012, N012, n2);
    get_CRT_term(term0, c0, n0, n1, n2);
    get_CRT_term(term1, c1, n1, n0, n2);
    get_CRT_term(term2, c2, n2, n0, n1);
    mpz_mod(term0, term0, N012);
    mpz_mod(term1, term1, N012);
    mpz_mod(term2, term2, N012);
    mpz_add(result, term0, term1);
    mpz_add(result, result, term2);
    mpz_mod(result, result, N012);

    mpz_clears(result, term0, term1, term2, N012, NULL);


    // --------------------Decrypt the message--------------------
    mpz_root(m_deciphered, result, 3);
    size_t count;
    size_t n_bits = mpz_sizeinbase(m_deciphered, 2);
    size_t n_bytes = (n_bits + 7) / 8;

    unsigned char* buffer = malloc(n_bytes+1);

    mpz_export(buffer, &count, 1, 1, 1, 0, m_deciphered);

    buffer[count] = '\0';
    printf("message deciphered:\n%s", buffer);
    

    mpz_clears(e, p0, q0, p1, q1, p2, q2, n0, n1, n2, d, c0, c1, c2, m, m_deciphered, NULL);
    return 0;
}
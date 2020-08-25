#include <math.h>
#include <stdint.h>

double secret = 0.651;

double secret_mul(double input)
{
    return (secret * input);
}

double get_secret(void)
{
    return secret;
}

void set_secret(double s)
{
    secret = s;
}

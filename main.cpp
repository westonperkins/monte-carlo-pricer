#include <iostream>
#include "mc_pricer.h"

int main()
{
    double S0 = 100;
    double K = 105;
    double r = 0.05;
    double sigma = 0.2;
    double T = 1.0;
    int N = 1'000'000;

    double price_std = monte_carlo_call(S0, K, r, sigma, T, N);
    double price_ant = monte_carlo_call_antithetic(S0, K, r, sigma, T, N);

    std::cout << "Standard MC   : " << price_std << std::endl;
    std::cout << "Antithetic MC : " << price_ant << std::endl;

    return 0;
}

#ifndef MC_PRICER_H
#define MC_PRICER_H

double monte_carlo_call(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N
);

double monte_carlo_delta(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N,
    double h
);

double monte_carlo_call_antithetic(
    double S0,
    double K,
    double r,
    double sigma,
    double T,
    int N
);

#endif

#ifndef PAYOFFS_H
#define PAYOFFS_H

#include "payoff.h"
#include <algorithm>

class CallPayoff : public Payoff
{
public:
    explicit CallPayoff(double K) : K_(K) {}

    double operator()(double ST) const override
    {
        return std::max(ST - K_, 0.0);
    }

private:
    double K_;
};

class PutPayoff : public Payoff
{
public:
    explicit PutPayoff(double K) : K_(K) {}

    double operator()(double ST) const override
    {
        return std::max(K_ - ST, 0.0);
    }

private:
    double K_;
};

#endif

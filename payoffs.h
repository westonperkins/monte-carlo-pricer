#ifndef PAYOFFS_H
#define PAYOFFS_H

#include "payoff.h"
#include <algorithm>

// concrete payoff implementations
// defines specific payoff types used by the monte carlo engine

// call option payoff 
class CallPayoff : public Payoff
{
public:
// constructor - stores the strike price used in the payoff formula
    explicit CallPayoff(double K) : K_(K) {}

    // payoff evaluation 
    // returns the value of the call option at expiration given the final stock price ST
    double operator()(double ST) const override
    {
        return std::max(ST - K_, 0.0);
    }

private:
    double K_;
};

// put option payoff
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

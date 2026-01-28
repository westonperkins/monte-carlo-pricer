#ifndef PAYOFF_H
#define PAYOFF_H

class Payoff
{
public:
    virtual ~Payoff() = default;

    // Make Payoff callable like a function
    virtual double operator()(double ST) const = 0;
};

#endif

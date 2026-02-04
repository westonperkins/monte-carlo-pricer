#ifndef PAYOFF_H
#define PAYOFF_H

// abtract payoff interface
// represents the idea of a payoff for a financial contract
// payoff defines how much an option or derivative is worth at expiration, given the final stock price

class Payoff
{
public:
    // ensures proper cleanup when deleting derived payoff objects through a base class pointer
    virtual ~Payoff() = default;

    // payoff eveluation operator
    // returns the value of the contract at expiration given the terminal stock price
    virtual double operator()(double ST) const = 0;
};

#endif

#ifndef LVC595_h
#define LVC595_h

#include <Arduino.h>

#define SRCLK_pin 14
#define SRDA_pin 13

/* parallel output pins after cascading two shift registers */
enum parallelpin
{
    Q0 = 0,
    Q1,
    Q2,
    Q3,
    Q4,
    Q5,
    Q6,
    Q7,
    Q8,
    Q9,
    Q10,
    Q11,
    Q12,
    Q13,
    Q14,
    Q15
};

#define CS12 Q2
#define CS11 Q3
#define CS10 Q4
#define CS9 Q5
#define CS8 Q6
#define CS7 Q7

#define CS6 Q8
#define CS5 Q9
#define CS4 Q10
#define CS3 Q11
#define CS2 Q12
#define CS1 Q13



class ShiftRegister
{
private:
    bool srda_status = 0;
    /* data */
public:
    void LVC595_init();

    /* shift low bit to desired output pin*/
    void SelectCS(int CSpin);

    /* shift desired pin value (high or low) to desired output pin*/
    void SelectCS(int CSpin, bool pinState);

    /* shift one bit */
    void ShiftCS();

    /* set all output pins HIGH */
    void High_all_CS();

    void StoreOneLOWbit();

    void StoreOnebit(bool BitVal);
};

#endif

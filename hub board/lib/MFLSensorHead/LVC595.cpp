#include "LVC595.h"
/* shift low bit to desired output pin*/
void ShiftRegister::SelectCS(int CSpin)
{
    StoreOneLOWbit();
    for (int i = 0; i <= CSpin; i++) // move low bit to desired
    {
        digitalWrite(SRCLK_pin, LOW);
        delayMicroseconds(1);
        digitalWrite(SRCLK_pin, HIGH);
        delayMicroseconds(1);
    }
}

/* shift desired pin value (high or low) to desired output pin*/
void ShiftRegister::SelectCS(int CSpin, bool pinState)
{
    StoreOnebit(pinState);
    for (int i = 0; i <= CSpin; i++) // move low bit to desired
    {
        digitalWrite(SRCLK_pin, LOW);
        delayMicroseconds(1);
        digitalWrite(SRCLK_pin, HIGH);
        delayMicroseconds(1);
    }
}

/* set all output pins HIGH */
void ShiftRegister::High_all_CS()
{
    digitalWrite(SRDA_pin, HIGH);
    srda_status = 1;
    for (int i = 0; i <= Q15 + 1; i++)
    {
        ShiftCS();
    }
}
/* shift one bit */
void ShiftRegister::ShiftCS()
{
    digitalWrite(SRCLK_pin, LOW);
    delayMicroseconds(1);
    digitalWrite(SRCLK_pin, HIGH);
    delayMicroseconds(1);
}

/*to init shift register*/
void ShiftRegister::LVC595_init()
{
    /* configure shift register pins*/
    pinMode(SRDA_pin, OUTPUT);
    pinMode(SRCLK_pin, OUTPUT);

    digitalWrite(SRDA_pin, LOW);
    srda_status = 1;
    digitalWrite(SRCLK_pin, HIGH);
    delayMicroseconds(1);
    ShiftCS();
}

void ShiftRegister::StoreOneLOWbit()
{
    High_all_CS();
    digitalWrite(SRDA_pin, LOW);
    srda_status = 0;

    ShiftCS();

    digitalWrite(SRDA_pin, HIGH); // set other bits following low bit to high
    srda_status = 1;
}

void ShiftRegister::StoreOnebit(bool BitVal)
{
    High_all_CS();
    digitalWrite(SRDA_pin, BitVal);
    srda_status = BitVal;

    ShiftCS();

    digitalWrite(SRDA_pin, !BitVal); // set other bits following low bit to high
    srda_status = !BitVal;
}

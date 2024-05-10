#include "msp.h"
#include "Clock.h"

#include <stdio.h>

#define LED_RED (1 << 0)
#define LED_GREEN (1 << 1)
#define LED_BLUE (1 << 2)

void SysTick_Init(void) {
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}

void Led_Init(void) {
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;
    P2->DIR |= 0x07;
    P2->OUT &= ~0x07;
}

void TurnOn_Led(int color) {
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}

void TurnOff_Led(void) {
    P2->OUT &= ~0x07;
}

void SysTick_Wait1ms() {
    SysTick->LOAD = 48000;
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0) {};
}

void SysTick_Wait1ms_With_Delay(int delay) {
    SysTick->LOAD = 48000 * delay;
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0) {};
}

void SysTick_Wait1us(int delay) {
    SysTick->LOAD = 48 * delay;
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0) {};
}

void SysTick_Wait1s() {
    int i, count = 1000;

    for (i = 0; i < count; i++) {
        SysTick_Wait1ms();
    }
    printf("1s passed...\n");
}

void Motor_Init(void) {
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0x30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;
}

void IR_Init(void) {
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void Turn_Left(void) {
    P2->OUT &= ~(1 << 7);
    P2->OUT |= (1 << 6);
}

void Turn_Right(void) {
    P2->OUT &= ~(1 << 6);
    P2->OUT |= (1 << 7);
}

void main(void)
{
   Clock_Init48MHz();
   Motor_Init();
   SysTick_Init();
   IR_Init();
   Led_Init();

//   int delay = 1;
   int speed = 1000;
   while (1) {
       P5->OUT |= 0x08;
       P9->OUT |= 0x04;

       P7->DIR = 0xFF;
       P7->OUT = 0xFF;
       Clock_Delay1us(10);

       P7->DIR = 0x00;
       Clock_Delay1us(1000);
       
//       if (delay >= 10000) delay = 1;
//
//       TurnOn_Led(LED_RED);
//       SysTick_Wait1us(10000 - delay);
//
//       TurnOff_Led();
//       SysTick_Wait1us(delay);
//
//       delay += 100;

       int sensor;
       sensor = P7->IN & 0x18;

       if (sensor) {
           P5->OUT &= ~0x30;
           P2->OUT |= 0xC0;
           P3->OUT |= 0xC0;

           P2->OUT |= 0x01;

        //    Turn_Left();
        //    SysTick_Wait1ms();

        //    Turn_Right();
        //    SysTick_Wait1ms();
       } else {
           P2->OUT &= ~0xC0;
           P2->OUT &= ~0x07;
       }

       P5->OUT &= ~0x08;
       P9->OUT &= ~0x04;
       Clock_Delay1ms(10);
   }
}

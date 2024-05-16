#include "msp.h"
#include "Clock.h"

#include <stdio.h>

void (*TimerA2Task) (void);
void TimerA2_Init(void(*task)(void), uint16_t period) {
    TimerA2Task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = (period - 1);
    TIMER_A2->EX0 = 0x0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;
}

void TA2_0_IRQHandler(void) {
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*TimerA2Task)();
}

void task() {
    printf("interrupt occurs!\n");
}

void PWM_Init34(uint16_t period, uint16_t duty3, uint16_t duty4) {
    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;

    TIMER_A0->CCTL[0] = 0x800;
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;
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

    PWM_Init34(15000, 0, 0);
}

void Move(uint16_t leftDuty, uint16_t rightDuty) {
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}

void Left_Forward(void) {
    P5->OUT &= ~0x10;
}

void Left_Backward(void) {
    P5->OUT |= 0x10;
}

void Right_Forward(void) {
    P5->OUT &= ~0x20;
}

void Right_Backward(void) {
    P5->OUT |= 0x20;
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

void PWM_Duty3(uint16_t duty3) {
    TIMER_A0->CCR[3] = duty3;
}

void PWM_Duty4(uint16_t duty4) {
    TIMER_A0->CCR[4] = duty4;
}

void main(void)
{
    Clock_Init48MHz();
    Motor_Init();
    IR_Init();
//    TimerA2_Init(&task, 50000);
    while(1) {
        P5->OUT |= 0x08;
        P9->OUT |= 0x04;

        P7->DIR = 0xFF;
        P7->OUT = 0xFF;
        Clock_Delay1us(10);

        P7->DIR = 0x00;
        Clock_Delay1us(1000);

        int sensor;
        sensor = P7->IN & 0x18;

        if (sensor) {
            // Move Forward
            Left_Forward();
            Right_Forward();
            Move(2000, 2000);
//            Clock_Delay1ms(3000);
        }

        if (P7->IN == 0xFF) {
            printf("P7->IN = %d\n", P7->IN);
            P3->OUT &= ~0xC0;
            printf("finish!\n");
            while (1) {}
        }

//        // Move Forward
//        Left_Forward();
//        Right_Forward();
//        Move(2000, 2000);
//        Clock_Delay1ms(3000);
//
//        // Move Backward
//        Left_Backward();
//        Right_Backward();
//        Move(2000, 2000);
//        Clock_Delay1ms(3000);
//
//        // Rotate
//        Left_Forward();
//        Right_Backward();
//        Move(1500, 1500);
//        Clock_Delay1ms(3000);
    };
}

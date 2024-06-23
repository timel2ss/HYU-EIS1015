#include "msp.h"
#include "Clock.h"
#include <stdio.h>

#define ROTATION_TIME 320

int map[8][8], mask[8][8];
int hard_map[8][8];
int phase = 0;
int cur_node = 0;
int32_t Position;
int8_t Data;

int arr[16] = {0};

enum DIR
{
    W = 0,
    SW,
    S,
    SE,
    E,
    NE,
    N,
    NW
};
enum DIR cur_dir;

void led_init() {
    // Set P2 as GPIO
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0x07;

    // Input or Output
    // Current type is output
    P2->DIR |= 0x07;

    //Turn off LED
    P2->OUT &= ~0x07;
}

void turn_on_led(int color) {
    P2->OUT &= ~0x07;
    P2->OUT |= color;
}
void turn_off_led() {
    P2->OUT &= ~0x07;
}

void PMW_init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
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

void (*TimerA2Task) (void);
void TimerA2_Init(void(*task)(void), uint16_t period)
{
    TimerA2Task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = period - 1;
    TIMER_A2->EX0 = 0x0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;
}

void TA2_0_IRQHandler(void)
{
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*TimerA2Task)();
}

void IR_Init()
{
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
    P7->DIR |= 0xFF;
}

void IR_On()
{
    P5->OUT |= 0x08;
    P9->OUT |= 0x04;
}

void IR_Off()
{
    P5->OUT &= ~0x08;
    P9->OUT &= ~0x04;
}

uint8_t IRSensor_read(uint32_t time){
    uint8_t result;
    P5->OUT |= 0x08; //set 5.3 as high
    P9->OUT |= 0x04; //set 9.2 as high

    P7->DIR |= 0xFF; //set P7.7-P7.0 as output
    P7->OUT |= 0xFF; //set P7.7-P7.0 as high
    Clock_Delay1us(10);

    P7->DIR = 0x00; //set P7.7-P7.0 as input
    Clock_Delay1us(time);

    result = (P7->IN & 0xFF);
    P5->OUT &= ~0x08; //turn off IR LED
    P9->OUT &= ~0x04; //turn off IR LED
    return ~result;
}

int32_t IRSensor_position(uint8_t data) {
    int W[8] = {0, 0, -142, -47, 47, 142, 0, 0};
    uint8_t Mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    int sum, count, result, i;
    sum = 0;
    count = 0;
    result = 0;

    for(i = 0; i < 8; i++){
        if(data & Mask[i]){
            count++;
            sum = sum + W[i];
        }
    }

    if(data == 0){
        return 333;
    }
    else{
        result = sum / count;
        return result;
    }
}

void Motor_Init()
{
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

    PMW_init34(15000, 0, 0);
}

void Motor_On()
{
    P3->OUT |= 0xC0;
}

void Motor_Off()
{
    P3->OUT &= ~0xC0;
}

void PWM_Duty3(uint16_t duty3)
{
    TIMER_A0->CCR[3] = duty3;
}

void PWM_Duty4(uint16_t duty4)
{
    TIMER_A0->CCR[4] = duty4;
}

void Move(uint16_t leftDuty, uint16_t rightDuty)
{
    P3->OUT |= 0xC0;
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
}

void Motor_stop(void){
    P5->OUT &= ~0x30;
    P2->OUT &= ~0xC0;   // off
    P3->OUT |= 0xC0;   // low current sleep mode
    PMW_init34(15000, 0, 0);
    return;
}

void Motor_forward(uint16_t leftDuty, uint16_t rightDuty){
    P3 -> OUT |= 0xC0;  // nSleep = 1
    P5 -> OUT &= ~0x30; // PH = 0
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
    return;
}


void Motor_right(uint16_t leftDuty, uint16_t rightDuty){
    P3 -> OUT |= 0xC0;  // nSleep = 1
    P5 -> OUT &= ~0x10; // P5.4 PH = 0
    P5 -> OUT |= 0x20; // P5.5 PH = 1
    PWM_Duty4(leftDuty);
    PWM_Duty3(rightDuty);
}

void Motor_left(uint16_t leftDuty, uint16_t rightDuty){
    P3 -> OUT |= 0xC0;  // nSleep = 1
    P5 -> OUT |= 0x10; // P5.4 PH = 1
    P5 -> OUT &= ~0x20; // P5.5 PH = 0
    PWM_Duty4(leftDuty);
    PWM_Duty3(rightDuty);
}

void Motor_backward(uint16_t leftDuty, uint16_t rightDuty){
    P3 -> OUT |= 0xC0;  // nSleep = 1
    P5 -> OUT |= 0x30; // PH = 1
    PWM_Duty3(rightDuty);
    PWM_Duty4(leftDuty);
    return;
}
// Motor Control end
void Motor_center(int wait) {
    turn_on_led(1);
    Motor_forward(2000,2000);
    Clock_Delay1ms(wait);
    Motor_stop();
    turn_off_led();
}

void Left45(int acc) {
   turn_on_led(4);
   Motor_left(3000,3000);
   if(acc == 1) Clock_Delay1ms(ROTATION_TIME * acc - 100);
   else Clock_Delay1ms(ROTATION_TIME * acc);
   Motor_stop();
   turn_off_led();
}

void Right45(int acc) {
   turn_on_led(4);
   Motor_right(3000,3000);
   if(acc == 1) Clock_Delay1ms(ROTATION_TIME * acc);
   else Clock_Delay1ms(ROTATION_TIME * acc - 50);
   Motor_stop();
   turn_off_led();
}

void init()
{
    int i, j;
    for(i = 0; i < 8; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            map[i][j] = -1;
            mask[i][j] = 0;
            hard_map[i][j] = -1;
        }
    }
    cur_dir = S;
}

// To test Phase 02 only
void hard_init()
{
    map[0][1] = W;
    map[0][3] = SW;
    map[0][5] = S;
    map[0][7] = SE;

    map[1][0] = E;
    map[1][2] = SW;
    map[1][4] = S;
    map[1][6] = SE;

    map[2][1] = NE;
    map[2][3] = S;
    map[2][5] = SE;
    map[2][7] = E;

    map[3][0] = NE;
    map[3][2] = N;
    map[3][4] = SE;
    map[3][6] = E;

    map[4][1] = N;
    map[4][3] = NW;
    map[4][5] = E;
    map[4][7] = NE;

    map[5][0] = N;
    map[5][2] = NW;
    map[5][4] = W;
    map[5][6] = NE;

    map[6][1] = NW;
    map[6][3] = W;
    map[6][5] = SW;
    map[6][7] = N;

    map[7][0] = NW;
    map[7][2] = W;
    map[7][4] = SW;
    map[7][6] = S;
}

// To test Phase 01's matrix information checking
void hard_init_compare()
{
    hard_map[0][1] = W;
    hard_map[0][3] = SW;
    hard_map[0][5] = S;
    hard_map[0][7] = SE;

    hard_map[1][0] = E;
    hard_map[1][2] = SW;
    hard_map[1][4] = S;
    hard_map[1][6] = SE;

    hard_map[2][1] = NE;
    hard_map[2][3] = S;
    hard_map[2][5] = SE;
    hard_map[2][7] = E;

    hard_map[3][0] = NE;
    hard_map[3][2] = N;
    hard_map[3][4] = SE;
    hard_map[3][6] = E;

    hard_map[4][1] = N;
    hard_map[4][3] = NW;
    hard_map[4][5] = E;
    hard_map[4][7] = NE;

    hard_map[5][0] = N;
    hard_map[5][2] = NW;
    hard_map[5][4] = W;
    hard_map[5][6] = NE;

    hard_map[6][1] = NW;
    hard_map[6][3] = W;
    hard_map[6][5] = SW;
    hard_map[6][7] = N;

    hard_map[7][0] = NW;
    hard_map[7][2] = W;
    hard_map[7][4] = SW;
    hard_map[7][6] = S;
}

// 0 : black, 1 : white
// this function is used to detect node match pattern
int event_node(uint8_t pattern)
{
    if(pattern == 0b11000011 || pattern == 0b11000111 || pattern == 0b11100011 ||
       pattern == 0b10000111 || pattern == 0b11100001 ||
       pattern == 0b01000010 || pattern == 0b00100100 ||
       pattern == 0b10000001 || pattern == 0b11000001 || pattern == 0b10000011 || pattern == 0b11000011 ||
       pattern == 0b00100000 || pattern == 0b00000100 || pattern == 0b11000100 || pattern == 0b00100011) return 1;
    return 0;
}

// this function is used to detect pattern to ignore signals
// when multiple paths overlap (when moving between non-adjacent nodes)
int event_ignore(uint8_t pattern)
{
    if(pattern == 0b00000000) return 1;
    return 0;
}

// this function is used to detect finish pattern;
int event_finish(uint8_t pattern)
{
    return pattern == 0b00000000;
}

// Returns adjacency
int is_adjacent(int cur_node, int next_node)
{
    return (abs(cur_node - next_node) == 1) || (abs(cur_node - next_node) == 7);
}

// Arrange the instruments by cur_node and dest_dir, adjacency.
void alignment(enum DIR dest_dir, int is_adjacent)
{
    if(is_adjacent) Motor_center(300);
    else Motor_center(300);
    int rotation_time = (dest_dir + (8 - cur_dir) % 8) % 8;
    if (rotation_time <= 4) {
        Left45(rotation_time);
    }
    else {
        Right45(8-rotation_time);
    }
    cur_dir = dest_dir;
}

// Function that allows you to drive in line with the center line when no special signal handling is performed
void Motor_idle()
{
    int straight_speed = 2000, turn_speed = 1000, backup_speed = 500;

    int time1 = 30,
        time2 = 60,
        time3 = 90,
        time_backup = 100;

    Position = IRSensor_position(Data);
    // normal trace algorithm
    if (Data == 0b00000000) { // lost the way
        Motor_backward(2000,2000);
        Clock_Delay1ms(time_backup);
    }

    else if (Position > -23 && Position < 23) {  //center
       Motor_forward(straight_speed, straight_speed);
       Clock_Delay1ms(time1);
    }

    else if (Position <= -23  && Position > -47) {
        Motor_left(turn_speed, turn_speed);
        Clock_Delay1ms(time2);
    }

    else if (Position >= 23 && Position < 47) {
       Motor_right(turn_speed, turn_speed);
       Clock_Delay1ms(time2);
    }

    else if (Position <= -47 && Position > -95) {
       Motor_left(turn_speed, turn_speed);
       Clock_Delay1ms(time1);
    }

    else if (Position >= 47 && Position < 95) {
       Motor_right(turn_speed, turn_speed);
       Clock_Delay1ms(time1);
    }

    else if (Position <= -95 && Position > -142) { //slightly off to the left
        Motor_left(turn_speed, turn_speed);
        Clock_Delay1ms(time1);
    }

    else if (Position >= 95 && Position <142) { //slightly off to the right
        Motor_right(turn_speed,turn_speed);
        Clock_Delay1ms(time1);
    }

    else if (Position <= -142 && Position >-237) { //off to the left
        Motor_left(turn_speed,turn_speed);
        Clock_Delay1ms(time2);
    }

    else if (Position >= 142 && Position < 237) { // off to the right
        Motor_right(turn_speed,turn_speed);
        Clock_Delay1ms(time2);
    }

    else if (Position <= -237 && Position > -332) { // way off left
        Motor_left(turn_speed,turn_speed);
        Clock_Delay1ms(time3);
    }

    else if (Position >= 237 && Position < 332) { // way off right
        Motor_right(turn_speed,turn_speed);
        Clock_Delay1ms(time3);
    }

    else {
        Motor_backward(backup_speed,backup_speed);
        Clock_Delay1ms(time_backup);
    }
    Motor_stop();
}

// A function that contains all the actions that the instrument must perform, given the number of the start and arrival nodes
void go(int from, int to) {
    turn_on_led(2);
    alignment(map[from][to], is_adjacent(from, to));
    Motor_forward(2000,2000);
    Clock_Delay1ms(200);

    int cnt = 0, limit = is_adjacent(from, to) ? 0 : 1;

    do
    {
        Data = IRSensor_read(1000);
        Position = IRSensor_position(Data);

        if (!is_adjacent(from,to)) {
            do
            {
                Data = IRSensor_read(1000);
                Position = IRSensor_position(Data);
                if(event_ignore(Data) && cnt < limit)
                {
                    turn_on_led(4);
                    Motor_forward(3000,3000);
                    Clock_Delay1ms(1200);
                    turn_off_led();
                    cnt++;
                }
                else Motor_idle();
            }
            while(cnt < limit);
        }
        else Motor_idle();
    } while(!event_node(Data));

    turn_off_led();
}

// One-stroke algorithm
void dfs(int from) {
    int to = (from + 1) % 8;
    while (to != (from + 7) % 8) {
        if (map[from][to] != -1 && mask[from][to] != 1) {
            mask[to][from] = 1;
            mask[from][to] = 1;
            go(from, to);
            dfs(to);
        } else {
            to = (to + 1) % 8;
        }
    }
}

// turn left to 45 degree
void Scan45() {
   Motor_left(3000,3000);
   Clock_Delay1ms(ROTATION_TIME);
   Motor_stop();
   Clock_Delay1ms(500);
}

// Check the presence of a path in the viewing direction
int isThere()
{
    uint8_t mask = 0b00111100;
    Data = IRSensor_read(1000);
    mask = mask & Data;
    mask = mask >> 2;
    return mask < 15;
}

// Phase_1 wrapping function
void Phase_1(int node)
{
    int cnt = 0, idx = (node + 1) % 8;
    if(node == 0)
    {
        // turn right to 90 degree
        Right45(2);

        // change cur_dir to W
        cur_dir = W;
    }
    else
    {
        Scan45();
        cur_dir = (cur_dir + 1) % 8;
    }

    do
    {
        Motor_forward(2000,2000);
        Clock_Delay1ms(200);
        Motor_stop();

        if(isThere())
        {
            turn_on_led(2);
            cnt++;
            map[node][idx] = cur_dir;
            cur_dir = (cur_dir + 1) % 8;
            idx = (idx + 2) % 8;
            Clock_Delay1ms(200);
            turn_off_led();
        }

        Motor_backward(2000,2000);
        Clock_Delay1ms(200);
        Motor_stop();

        if(cnt < 4) Scan45();

    } while(cnt < 4);

    Right45(3);
    Motor_forward(3000,3000);
    Clock_Delay1ms(150);
    cur_dir = (cur_dir + 4) % 8;
}

// Checking Gloden answer & scan result, for debugging
int Checksum()
{
    int i, j;
    for(i = 0; i < 8; ++i)
    {
        for(j = 0; j < 8; ++j)
        {
            if(map[i][j] != hard_map[i][j]) return 0;
        }
    }
    return 1;
}

int main(void)
{
    Clock_Init48MHz();
    Motor_Init();
    IR_Init();
    led_init();

    init();

    // Initialize information of current node & current direction
    cur_node = 0;
    cur_dir = S;

    // for checking all nodes
    int nodes = 0;

    // Phase 01
    while(nodes < 8)
    {
        turn_on_led(1);
        Data = IRSensor_read(1000);
        if(event_node(Data))
        {
            Motor_center(300);
            Phase_1(cur_node++);
            nodes++;
        }
        else Motor_idle();
    }

    // Reset for Phase 02
    cur_node %= 8;
    int timer = 0;

    // Phase 02
    while(1)
    {
        turn_on_led(2);
        Data = IRSensor_read(1000);
        if(event_node(Data)) {
            Motor_stop();
            for(timer = 0; timer < 3; ++timer)
            {
                turn_on_led(1);
                Clock_Delay1ms(500);
                turn_off_led();
                Clock_Delay1ms(500);
            }
            dfs(0);
            break;
        }
        else Motor_idle();
    }

    Motor_forward(3000,3000);
    Clock_Delay1ms(100);

    turn_on_led(2);
    do
    {
        Data = IRSensor_read(1000);
        Motor_idle();
    }
    while(!event_finish(Data));
    Motor_stop();
    turn_off_led();
}

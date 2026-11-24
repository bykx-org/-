#ifndef __PID_H
#define __PID_H

#include "stm32f4xx.h"

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float target;
    float actual;
    float err;
    float err_last;
    float integral;
    float output;

    float max_output;    /* Êä³öÏÞ·ù */
} PID_t;

void  PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float max_output);
void  PID_SetTarget(PID_t *pid, float target);
float PID_Calc(PID_t *pid, float actual);
void  PID_Reset(PID_t *pid);

#endif
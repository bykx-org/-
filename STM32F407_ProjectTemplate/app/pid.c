#include "PID.h"

void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float max_output)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->target   = 0;
    pid->actual   = 0;
    pid->err      = 0;
    pid->err_last = 0;
    pid->integral = 0;
    pid->output   = 0;

    pid->max_output = max_output;
}

void PID_SetTarget(PID_t *pid, float target)
{
    pid->target = target;
}

float PID_Calc(PID_t *pid, float actual)
{
    pid->actual = actual;
    pid->err    = pid->target - pid->actual;

    pid->integral += pid->err;

    pid->output = pid->Kp * pid->err
                + pid->Ki * pid->integral
                + pid->Kd * (pid->err - pid->err_last);

    /* Êä³öÏŞ·ù */
    if (pid->output >  pid->max_output) pid->output =  pid->max_output;
    if (pid->output < -pid->max_output) pid->output = -pid->max_output;

    pid->err_last = pid->err;

    return pid->output;
}

void PID_Reset(PID_t *pid)
{
    pid->err      = 0;
    pid->err_last = 0;
    pid->integral = 0;
    pid->output   = 0;
}
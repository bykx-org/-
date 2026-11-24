#ifndef __EMM_V5_2_H
#define __EMM_V5_2_H

#include "usart2.h"

#define ABS_2(x)  ((x) > 0 ? (x) : -(x))

typedef enum {
    S_VER_2   = 0,
    S_RL_2    = 1,
    S_PID_2   = 2,
    S_VBUS_2  = 3,
    S_CPHA_2  = 5,
    S_ENCL_2  = 7,
    S_TPOS_2  = 8,
    S_VEL_2   = 9,
    S_CPOS_2  = 10,
    S_PERR_2  = 11,
    S_FLAG_2  = 13,
    S_Conf_2  = 14,
    S_State_2 = 15,
    S_ORG_2   = 16,
} SysParams_t_2;

void Emm_V5_Reset_CurPos_To_Zero_2(uint8_t addr);
void Emm_V5_Reset_Clog_Pro_2(uint8_t addr);
void Emm_V5_Read_Sys_Params_2(uint8_t addr, SysParams_t_2 s);
void Emm_V5_Modify_Ctrl_Mode_2(uint8_t addr, bool svF, uint8_t ctrl_mode);
void Emm_V5_En_Control_2(uint8_t addr, bool state, bool snF);
void Emm_V5_Vel_Control_2(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_Pos_Control_2(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF);
void Emm_V5_Stop_Now_2(uint8_t addr, bool snF);
void Emm_V5_Synchronous_motion_2(uint8_t addr);
void Emm_V5_Origin_Set_O_2(uint8_t addr, bool svF);
void Emm_V5_Origin_Modify_Params_2(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF);
void Emm_V5_Origin_Trigger_Return_2(uint8_t addr, uint8_t o_mode, bool snF);
void Emm_V5_Origin_Interrupt_2(uint8_t addr);

#endif

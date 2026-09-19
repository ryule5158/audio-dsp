#ifndef WM8960_H
#define WM8960_H

#include "xiicps.h"
#include "xil_types.h"

#define WM8960_I2C_ADDRESS 0x1AU

typedef struct {
    XIicPs *iic;
    u8 failed_register;
} Wm8960;

int Wm8960_Initialize(Wm8960 *codec, XIicPs *iic);
int Wm8960_Configure48k24bitMaster(Wm8960 *codec,
                                  u8 headphone_volume);
int Wm8960_WriteRegister(Wm8960 *codec, u8 reg, u16 value);

#endif

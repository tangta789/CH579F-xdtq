#ifndef __KXTJ9_H__
#define __KXTJ9_H__
#include "CH57x_common.h"
#ifdef __cplusplus
 extern "C" {
#endif
BOOL KXTJ9_Restart(void);
BOOL KXTJ9_HasData(void);
BOOL KXTJ9_ReadData(UINT16 *x, UINT16 *y, UINT16 *z);
#ifdef __cplusplus
 }
#endif
#endif

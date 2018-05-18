/**********************************************************************************************************************
File: comiskey.h                                                                

----------------------------------------------------------------------------------------------------------------------

6. Add/update any special configurations required in configuration.h (e.g. peripheral assignment and setup values)
TODO: Investigate the configuration.h file as stated above.
----------------------------------------------------------------------------------------------------------------------

Description:
Header file for comiskey.c

**********************************************************************************************************************/

#ifndef __COMISKEY_H
#define __COMISKEY_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* Required constants for ant channel config */
//Device is a MASTER
#define ANT_CHANNEL_TYPE_USERAPP        CHANNEL_TYPE_MASTER
//Channel 0 - 7
#define ANT_CHANNEL_USERAPP             ANT_CHANNEL_0
// LO; 0X0001 - 0X7FFF
#define ANT_CHANNEL_PERIOD_LO_USERAPP   (u8)0x00
// HI; 0x0001 - 0x7fff
#define ANT_CHANNEL_PERIOD_HI_USERAPP   (u8)0x20
// Device # Low byte (unique)
#define ANT_DEVICEID_LO_USERAPP         (u8)0x89
// Device # High byte (unique)
#define ANT_DEVICEID_HI_USERAPP         (u8)0x19
// 1 - 255
#define ANT_DEVICE_TYPE_USERAPP         (u8)1
// 1-127; MSB is pairing
#define ANT_TRANSMISSION_TYPE_USERAPP   (u8)1
// 2400MHz + 0 - 99 MHz
#define ANT_FREQUENCY_USERAPP           (u8)50
// Max tx power
#define ANT_TX_POWER_USERAPP            RADIO_TX_POWER_4DBM
/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
//void LaserTagToggler(void);
void receivingSignal(void);
void Com_38Modulate(void);
void receivingHighBit(void);
void receivingLowBit(void);
void OnBit(void);
void OffBit(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
//void LaserTagInitialize(void);
//void LaserTagRunActiveState(void);
void ComInitialize(void);
void ComRunActiveState(void);
void Com_38Modulate(void);
void EnableInterruptReg(void);
void reset(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void ComSM_ModulateOn(void);
static void ComSM_ModulateOff(void);
static void ComSM_Idle(void);
static void ComSM_Error(void);
static void ComSM_TransmitWhite(void);
static void ComSM_TransmitPurple(void);
static void ComSM_TransmitBlue(void);
static void ComSM_TransmitCyan(void);
static void ComSM_TransmitGreen(void);
static void ComSM_TransmitYellow(void);
static void ComSM_TransmitOrange(void);
static void ComSM_TransmitRed(void);

static void ComSM_ReceiverMode(void);

static void ComSM_ReceiveWhite(void);
static void ComSM_ReceivePurple(void);
static void ComSM_ReceiveBlue(void);
static void ComSM_ReceiveCyan(void);
static void ComSM_ReceiveGreen(void);
static void ComSM_ReceiveYellow(void);
static void ComSM_ReceiveOrange(void);
static void ComSM_ReceiveRed(void);

#endif /* __COMISKEY_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

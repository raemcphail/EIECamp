/**********************************************************************************************************************
File: comiskey.h                                                                

----------------------------------------------------------------------------------------------------------------------

6. Add/update any special configurations required in configuration.h (e.g. peripheral assignment and setup values)
TODO: Investigate the configuration.h file as stated above.
----------------------------------------------------------------------------------------------------------------------

Description:
Header file for comiskeySV.c

**********************************************************************************************************************/

#ifndef __COMISKEYSV_H
#define __COMISKEYSV_H

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
//void Com_38Modulate(void);
//void receivingHighBit(void);
//void receivingLowBit(void);
//void OnBit(void);
//void OffBit(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void ComSVInitialize(void);
void ComSVRunActiveState(void);
void ComSV_38Modulate(void);
void EnableInterruptReg(void);
void reset(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void ComSVSM_ModulateOn(void);
static void ComSVSM_ModulateOff(void);
static void ComSVSM_Idle(void);
static void ComSVSM_Error(void);
static void ComSVSM_TransmitWhite(void);
static void ComSVSM_TransmitPurple(void);
static void ComSVSM_TransmitBlue(void);
static void ComSVSM_TransmitCyan(void);
static void ComSVSM_TransmitGreen(void);
static void ComSVSM_TransmitYellow(void);
static void ComSVSM_TransmitOrange(void);
static void ComSVSM_TransmitRed(void);

static void ComSVSM_ReceiverMode(void);

static void ComSVSM_ReceiveWhite(void);
static void ComSVSM_ReceivePurple(void);
static void ComSVSM_ReceiveBlue(void);
static void ComSVSM_ReceiveCyan(void);
static void ComSVSM_ReceiveGreen(void);
static void ComSVSM_ReceiveYellow(void);
static void ComSVSM_ReceiveOrange(void);
static void ComSVSM_ReceiveRed(void);

#endif /* __COMISKEYSV_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

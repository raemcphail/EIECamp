/**********************************************************************************************************************
File: LaserTag.c                                                                



Description:
This is a comiskey.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void LaserTagInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void LaserTagRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_LaserTag"
***********************************************************************************************************************/
/* New variables */
//volatile u32 G_u32LaserTagFlags;                       /* Global state flags */
volatile u32 G_u32ComFlags;

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern u32 G_u32AntApiCurrentMessageTimeStamp;
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Com_" and be declared as static.
***********************************************************************************************************************/
/*static fnCode_type LaserTag_StateMachine; 
static bool LaserTag_Toggle;*/
static u16 u16Count5ms;
static u16 u16countHigh;
static u16 u16countLow;
static u16 u16Lives;
static u16 u16RecoverTime;
static u16 delimiter = 600;
static u16 u16soundCount;
static fnCode_type Com_StateMachine;
static bool Com_ModulateSwitch;

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
/*
Function: LasterTagToggler
Description:
Changes the truth value of the boolean LaserTagToggler every time the function
is called.
*/
/*void LaserTagToggler(void)
{
  u32 *pu32ToggleGPIO;
  if(LaserTag_Toggle)
  {
    pu32ToggleGPIO = (u32*)(&(AT91C_BASE_PIOA->PIO_SODR));
    LaserTag_Toggle = FALSE;
  }
  else
  {
    pu32ToggleGPIO = (u32*)(&(AT91C_BASE_PIOA->PIO_CODR));
    LaserTag_Toggle = TRUE;
  }
  *pu32ToggleGPIO = PA_10_I2C_SCL;
}*/
/* end of LaserTagToggler */
/*------------------------------------------------------------
Function: Com_38Modulate
Description:
Changes the truth value of the boolean Com_ModulateSwitch every time the function
is called. This boolean is used to determine if tranmitter should be ON or OFF.
This function gets called every 5 ticks so that the signal tranmitted can be about 38kHz
and the reciever can detect the signal.
*/
void Com_38Modulate(void)
{
  u32 *pu32ToggleGPIO;
  if(Com_ModulateSwitch)
  {
    pu32ToggleGPIO = (u32*)(&(AT91C_BASE_PIOA->PIO_SODR));
    Com_ModulateSwitch = FALSE;
  }
  else
  {
    pu32ToggleGPIO = (u32*)(&(AT91C_BASE_PIOA->PIO_CODR));
    Com_ModulateSwitch = TRUE;
  }
  *pu32ToggleGPIO = PA_10_I2C_SCL;
}
/* end of LaserTagToggler */
/*------------------------------------------------------------
Function: gotShot
Description:
Checks if the modulated signal of 38 kHz is received by the GPIO pin PA_14_BLADE_MOSI.
Requires:
PA_14_BLADE_MOSI is configured correctly as an input pin, must check if receives voltage HIGH
for 5 ms, then VOLTAGE LOW for 5ms, then register that it has been hit and turn an LED on for a bit.

Promises:
Return true if the signal has been received.
*/
void receivingSignal(void)
{
    u32 *pu32Address;
    pu32Address = (u32*)(&(AT91C_BASE_PIOA->PIO_PDSR));
    u32 u32truthValue = (*pu32Address) & 0x00004000;
    //rHigh is the boolean that indicates if the receiver detects a signal
    bool rHigh = FALSE;
    if(u32truthValue == 0x00004000)
    {
      rHigh = TRUE;
    }
    if (u16countHigh ==5 && u16countLow==5) 
    {
      u16countHigh = 0;
      u16countLow = 0;
     // LedOn(WHITE);
     // PWMAudioSetFrequency(BUZZER1, 320);
     // u16Lives--;
      //LedOff(WHITE);
      LedOn(WHITE);
     //LaserTag_StateMachine = LaserTagSM_Recover;
    }
    else if (rHigh) 
    {
      u16countHigh++;
      //LedOff(WHITE);
    }
    else if (!rHigh && u16countHigh!=5) 
    {
      u16countHigh = 0;
      u16countLow = 0;
      //LedOff(WHITE);
    }
   else if (!rHigh && u16countHigh==5) 
    {
      u16countLow++;
    }
    
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: ComInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  Com_ModulateSwitch

Promises:
  -
*/
void ComInitialize(void)
{
  /* Enable the Interrupt Reg's for MOSI */
  
  /* Set counter to 0 to start*/
    u16countHigh = 0;
  /* Set counter to 0 to start*/
    u16countLow = 0;
  /* Set recover count to 0 to start */
    u16RecoverTime = 0;
  /* Set count sound to 0 to start */
    u16soundCount = 0;
   /* Player starts with 3 lives */
    u16Lives = 3;
  /* Set 5ms counter to 0 to start*/
  u16Count5ms = 0;
  /* Set switch to false to start. */
  Com_ModulateSwitch = FALSE;
   /* Set Timer with 5 tick period before inturrupt. */
  TimerSet(TIMER_CHANNEL1, 0x0005);
   /* Sets Com_38Modulate to the timer function ptr */
  TimerAssignCallback(TIMER_CHANNEL1, Com_38Modulate);
   /* Starts the timer */
  TimerStart(TIMER_CHANNEL1);
  if( 1 )
  {
    Com_StateMachine = ComSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    Com_StateMachine = ComSM_Error;
  }

} /* end ComInitialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function ComRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void ComRunActiveState(void)
{
  Com_StateMachine();

} /* end ComRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*

*/
static void ComSM_Idle(void)
{
  receivingSignal();
  if(IsButtonPressed(BUTTON0))
  {
    Com_StateMachine = ComSM_ModulateOn;
  }
  if(IsButtonPressed(BUTTON1))
  {
    Com_StateMachine = ComSM_TransmitWhite;
  }
 
} // end ComSM_Idle() 

static void ComSM_TransmitWhite(void)
{
  LedOn(WHITE);
}

static void ComSM_ModulateOn(void)
{
  receivingSignal();
  LedOn(PURPLE);
  TimerStart(TIMER_CHANNEL1);
  if(u16Count5ms >= 4)
  {
    u16Count5ms = 0;
    Com_StateMachine = ComSM_ModulateOff;
  }
  else
  {
    u16Count5ms++;
  }
}

static void ComSM_ModulateOff(void)
{
  receivingSignal();
  LedOn(PURPLE);
  if(u16Count5ms >= 4)
  {
    u16Count5ms = 0;
    if (IsButtonPressed(BUTTON0))
    {
       Com_StateMachine = ComSM_ModulateOn;
    }
    else
    {
       Com_StateMachine = ComSM_Idle;      
    }

  }
  else
  {
    u16Count5ms++;
    TimerStop(TIMER_CHANNEL1);
    Com_ModulateSwitch = FALSE;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void ComSM_Error(void)          
{

  
} /* end ComSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

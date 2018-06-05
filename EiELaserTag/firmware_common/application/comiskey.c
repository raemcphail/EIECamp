/**********************************************************************************************************************
File: comiskey.c                                                                



Description:
This is a project to teach about IR transmitter and receivers at a summer camp. 
The project involves
a board with a transmitter and a board with a receiver.There are 3 modes, 
transmitter mode (for the board with the transmitter), 
receiver mode 1 and receiver mode 2 (both for the board with the receiver).

By default transmitter mode is selected. The transmitter board can use BUTTON1 
and BUTTON2 to scroll
through and select each colour. The selected colour will be the only LED that
is ON. Each colour transmitts a different bit pattern, hold BUTTON0 to transmit 
the selected colour's bit pattern.

Press BUTTON3 to go into receiver mode 1. In this mode the receiver 
expects the colours in the order they appear on the board: white, 
purple, blue, cyan, green, yellow, orange, red.
The recevier board will turn the LED it was expecting ON when it sees
the bit pattern of the colour it is expecting then it will expect 
the next bit pattern. The purpose of this mode is to test that the
transmitter board has each bit pattern programmed correctly. 
To reset this mode press BUTTON3 again.

From receiver mode 1 press BUTTON2 to go to receiver mode 2, receiver mode 2 
can only be reached from receiver mode 1. In receiver mode 2 the 
order of the colours is green, purple, red, white, blue, 
cyan, orange, yellow. To reset press BUTTON2
------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void ComInitialize(void)
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
volatile u16 G_u16countSentBit;
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

static u16 u16Count5ms;
static u16 u16countHigh;
static u16 u16countLow;
static u16 u16countSound;
static u16 u16countReceivedBit;
static fnCode_type Com_StateMachine;
static bool Com_ModulateSwitch;

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------
Function: Com_38Modulate
Description:
Changes the truth value of the boolean Com_ModulateSwitch every time the function
is called. This boolean is used to determine if tranmitter should be ON or OFF.
This function gets called every 5 ticks so that the signal tranmitted can be 
about 38kHz and the reciever can detect the signal.
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
/* end of Com_38Modulate*/


/*------------------------------------------------------------
Function: receivingHighBit
Description:
Checks if a modulated signal of 38 kHz is received by the GPIO pin 
PA_14_BLADE_MOSI for 5ms.
Requires:
PA_14_BLADE_MOSI is configured correctly as an input pin, 

Promises:
Increases u16countRecievedBit if there is a High bit 
(receiver detected signal for 5ms). 
Sets u16countBit to 0 if the receiver isn't detecting a signal at 
any point during the 5ms.
*/
void receivingHighBit(void)
{
   
    u32 *pu32Address;
    pu32Address = (u32*)(&(AT91C_BASE_PIOA->PIO_PDSR));
    u32 u32truthValue = (*pu32Address) & 0x00004000;
    //rHigh is the boolean that indicates if the receiver detects a signal
    bool rHigh = TRUE;
    if(u32truthValue == 0x00004000)
    {
      rHigh = FALSE;
    }
 
   if (rHigh) //if signal is HIGH but hasn't been HIGH for 5ms yet
    {
      u16countHigh++;
    }
   
    else if (!rHigh && u16countHigh!=5) //If signal is LOW and hasn't been HIGH for 5ms, reset
   {
      u16countReceivedBit = 0;
      u16countHigh = 0;
      u16countLow = 0;
   }
   
     //if there has successfully been a high bit
    if(u16countHigh ==5 && u16countLow==0) 
    {
      u16countReceivedBit++;
      u16countHigh = 0;
    }
    
}
/*------------------------------------------------------------
Function: receivingLowBit
Description:
Checks if no signal is received by the GPIO pin 
PA_14_BLADE_MOSI for 5ms.
Requires:
PA_14_BLADE_MOSI is configured correctly as an input pin

Promises:
Increases u16countRecievedBit if there is a Low bit 
(receiver detected no signal for 5ms). 
Sets u16countBit to 0 if the receiver is detecting a signal at 
any point during the 5ms.
*/
void receivingLowBit(void)
{
    u32 *pu32Address;
    pu32Address = (u32*)(&(AT91C_BASE_PIOA->PIO_PDSR));
    u32 u32truthValue = (*pu32Address) & 0x00004000;
    //rHigh is the boolean that indicates if the receiver detects a signal
    bool rHigh = TRUE;
    if(u32truthValue == 0x00004000)
    {
      rHigh = FALSE;
    }
    if (!rHigh) //if signal is LOW but hasn't been LOW for 5ms yet
    {
      u16countLow++;
    }
    else if (rHigh && u16countLow!=5) //If signal is not LOW and hasn't been LOW for 5ms, reset
   {
      u16countReceivedBit = 0;
      u16countHigh = 0;
      u16countLow = 0;
   }
   //if there has successfully been a low bit
   if (u16countHigh == 0 && u16countLow==5) 
    {
      u16countReceivedBit++;
      u16countLow = 0;
    }
    
}
/*--------------------------------------------------------------------------------------------------------------------*/
/*Function: OnBit
Description:
Sends a signal modulated and 38kHz

Requires:
Transmitter is connected to SCL and GND. GPIO pin 
PA_14_BLADE_SCL is configured correctly as an output pin

Promises:
Transmit a signal modulated a 38kHz for 5ms
*/
void OnBit(void)
{  
  TimerStart(TIMER_CHANNEL1);
  if(u16Count5ms >= 5)
  {
    u16Count5ms = 0;
    G_u16countSentBit++;  
    G_u16countSentBit%=6;
  }
  else
  {
    u16Count5ms++;
  }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*Function: OffBit
Description:
Sends no signal for 5ms

Requires:
Transmitter is connected to SCL and GND. GPIO pin 
PA_14_BLADE_SCL is configured correctly as an output pin

Promises:
Does not transmit a signal for 5ms
*/
void OffBit(void)
{
  TimerStop(TIMER_CHANNEL1);
  Com_ModulateSwitch = FALSE;
  if(u16Count5ms >= 5)
  {
    u16Count5ms = 0;
    G_u16countSentBit++;  
    G_u16countSentBit%=6;
  }
  else
  {
    u16Count5ms++;
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
   /* Set counter to 0 to start*/
    G_u16countSentBit = 0; 
  /* Set counter to 0 to start*/
    u16countReceivedBit = 0;
  /* Set counter to 0 to start*/
    u16countHigh = 0;
  /* Set counter to 0 to start*/
    u16countLow = 0;
  /* Set 5ms counter to 0 to start*/
    u16Count5ms = 0;
  /* Set sound counter to 0 to start*/
    u16countSound = 0;
  /* Set switch to false to start. */
    Com_ModulateSwitch = FALSE;
  /* sets buzzer1 frequency to 500 */
  PWMAudioSetFrequency(BUZZER1, 500);
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
    TimerStop(TIMER_CHANNEL1);
    Com_ModulateSwitch = FALSE;
    Com_StateMachine = ComSM_TransmitWhite;
} // end ComSM_Idle() 

static void ComSM_ReceiverMode(void)
{
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  u16countReceivedBit = 0;
  u16countHigh = 0;
  u16countLow = 0;
  Com_StateMachine = ComSM_ReceiveWhite;
} //end ComSM_ReceiverMode

/*Wait for the bit pattern 101010 then go to ComSM_ReceivePurple*/
static void ComSM_ReceiveWhite(void)
{
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(WHITE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceivePurple;
  }
}/*end ComSM_ReceiveWhite()*/

/*Wait for the bit pattern 110110 then go to ComSM_ReceiveBlue*/
static void ComSM_ReceivePurple(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(PURPLE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveBlue;
  }
}/*end ComSM_ReceivePurple()*/

/*Wait for the bit pattern 110000 then go to ComSM_ReceiveCyan*/
static void ComSM_ReceiveBlue(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(BLUE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveCyan;
  }
}/*end ComSM_ReceiveBlue()*/

/*Wait for the bit pattern 111110 then go to ComSM_ReceiveGreen*/
static void ComSM_ReceiveCyan(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(CYAN);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveGreen;
  }
}/*end ComSM_ReceiveCyan()*/

/*Wait for the bit pattern 111010 then go to ComSM_ReceiveYellow*/
static void ComSM_ReceiveGreen(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(GREEN);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveYellow;
  }
}/*end ComSM_ReceiveGreen()*/

/*Wait for the bit pattern 110010 then go to ComSM_ReceiveOrange*/
static void ComSM_ReceiveYellow(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(YELLOW);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveOrange;
  }
}/*end ComSM_ReceiveYellow()*/

/*Wait for the bit pattern 101000 then go to ComSM_ReceiveRed*/
static void ComSM_ReceiveOrange(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(ORANGE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveRed;
  }
}/*end ComSM_ReceiveOrange()*/

/*Wait for the bit pattern 111100*/
static void ComSM_ReceiveRed(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(RED);
    u16countReceivedBit = 0;
  }
}/*end ComSM_ReceiveRed()*/

/*-------------------------------------------------------------------------------------*/
/* Receiver Mode 2 for final challenge in Communication is Key. It works the same way as 
before where each state expects a different colour then expects the next colour.
The only difference is the order of the colors is different, even the bit patterns are the same*/


/*Wait for the bit pattern 111010 then go to ComSM_ReceivePurple2*/
static void ComSM_ReceiveGreen2(void)
{
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(GREEN);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceivePurple2;
  }
}
/*end ComSM_ReceiveGreen2()*/

/*Wait for the bit pattern 110110 then go to ComSM_ReceiveRed2*/
static void ComSM_ReceivePurple2(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(PURPLE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveRed2;
  }
}/*end ComSM_ReceivePurple2()*/

/*Wait for the bit pattern 111100 then go to ComSM_ReceiveWhite2*/
static void ComSM_ReceiveRed2(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(RED);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveWhite2;
  }
}/*end ComSM_ReceiveRed2()*/

/*Wait for the bit pattern 101010 then go to ComSM_ReceiveBlue2*/
static void ComSM_ReceiveWhite2(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(WHITE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveBlue2;
  }
}/*end ComSM_ReceiveWhite2()*/

/*Wait for the bit pattern 110000 then go to ComSM_ReceiveCyan2*/
static void ComSM_ReceiveBlue2(void)
{  
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(BLUE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveCyan2;
  }
}/*end ComSM_ReceiveBlue2()*/

/*Wait for the bit pattern 111110 then go to ComSM_ReceiveOrange2*/
static void ComSM_ReceiveCyan2(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(CYAN);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveOrange2;
  }
}/*end ComSM_ReceiveCyan2()*/

/*Wait for the bit pattern 101000 then go to ComSM_ReceiveYellow2*/
static void ComSM_ReceiveOrange2(void)
{
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOn(ORANGE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_ReceiveYellow2;
  }
}/*end ComSM_ReceiveOrange2()*/

/*Wait for the bit pattern 110010 then go to ComSM_Song*/
static void ComSM_ReceiveYellow2(void)
{  
  if(IsButtonPressed(BUTTON2))
  {
    Com_StateMachine = ComSM_ReceiveGreen2;
  }
  if(IsButtonPressed(BUTTON3))
  {
    Com_StateMachine = ComSM_ReceiveWhite;
  }
  
  if(u16countReceivedBit == 0)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 1)
  {
    receivingHighBit();
  }
  else if (u16countReceivedBit == 2)
  {
    receivingLowBit();
  }
  else if (u16countReceivedBit == 3)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 4)
  {
    receivingHighBit();
  }
  else if(u16countReceivedBit == 5)
  {
    receivingLowBit();
  }
  else if(u16countReceivedBit == 6)
  {
    LedOff(RED);
    LedOff(ORANGE);
    LedOff(YELLOW);
    LedOff(GREEN);
    LedOff(CYAN);
    LedOff(BLUE);
    LedOff(PURPLE);
    LedOff(WHITE);
    u16countReceivedBit = 0;
    Com_StateMachine = ComSM_Song;
  }
}/*end ComSM_ReceiveYellow2()*/

/*Wait for 1800ms then go to ComSM_ReceiveGreen2*/
static void ComSM_Song(void)
{
  
    u16countSound++;
    if(u16countSound < 200)
    {
      LedOn(RED);
      LedOn(BLUE);
      LedOn(WHITE);
      PWMAudioSetFrequency(BUZZER1, 400);
      PWMAudioOn(BUZZER1);
    }
    else if(u16countSound < 400)
    {
      LedOff(RED);
      LedOff(BLUE);
      LedOff(WHITE);
      LedOn(PURPLE);
      LedOn(YELLOW);
      LedOn(CYAN);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
    }
     else if(u16countSound < 600)
    {
      LedOn(GREEN);
      LedOn(ORANGE);
      LedOn(BLUE);
      LedOff(PURPLE);
      LedOff(YELLOW);
      LedOff(CYAN);
      PWMAudioSetFrequency(BUZZER1, 600);
      PWMAudioOn(BUZZER1);
    }
    else if(u16countSound < 900)
    {
      LedOn(YELLOW);
      LedOn(WHITE);
      LedOn(CYAN);
      LedOff(GREEN);
      LedOff(ORANGE);
      LedOff(BLUE);
      PWMAudioSetFrequency(BUZZER1, 700);
      PWMAudioOn(BUZZER1);
    }
    else if(u16countSound < 1300)
    {
      LedOn(PURPLE);
      LedOn(GREEN);
      LedOn(RED);
      LedOff(YELLOW);
      LedOff(WHITE);
      LedOff(CYAN);
      PWMAudioSetFrequency(BUZZER1, 500);
      PWMAudioOn(BUZZER1);
    }
    else if(u16countSound < 1800)
    {
      LedOn(GREEN);
      LedOn(ORANGE);
      LedOn(BLUE);
      LedOn(PURPLE);
      LedOn(YELLOW);
      LedOn(CYAN);
      LedOn(RED);
      LedOn(WHITE);
      PWMAudioSetFrequency(BUZZER1, 800);
      PWMAudioOn(BUZZER1);
  }
  else
  {
    u16countReceivedBit = 0;
    PWMAudioOff(BUZZER1);
    u16countSound = 0;
    Com_StateMachine = ComSM_ReceiveGreen2;
    
  }
}/*end ComSM_Song*/

/*-----------------------------------------------------------------------------------------*/
/*The Transmitting States*/

//The following transmit states let the user scroll through each different colour to select one to transmit

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitWhite(void)
{
  LedOn(WHITE);
  LedOff(PURPLE);
  LedOff(RED);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitRed;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitPurple;
  }
 if(WasButtonPressed(BUTTON3))
 {
  LedOn(BLUE);
  ButtonAcknowledge(BUTTON3);
  Com_StateMachine = ComSM_ReceiverMode;
  }
  
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 101010
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
     if(G_u16countSentBit == 1)
    {  
      OffBit();
    }
   if(G_u16countSentBit == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OffBit();    
    }
    if(G_u16countSentBit == 4)
    {  
     OnBit();
    }
    if(G_u16countSentBit == 5)
    { 
      OffBit();
    }
  }
}/*end ComSM_TransmitWhite*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitPurple(void)
{
  LedOn(PURPLE);
  LedOff(WHITE);
  LedOff(BLUE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitWhite;
  }
  if(WasButtonPressed(BUTTON2))
  {
     ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitBlue;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 110110
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 2)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitPurple*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitBlue(void)
{
  LedOn(BLUE);
  LedOff(PURPLE);
  LedOff(CYAN);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitPurple;
  }
  if(WasButtonPressed(BUTTON2))
  {
     ButtonAcknowledge(BUTTON2);
     Com_StateMachine = ComSM_TransmitCyan;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 110000
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
     if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
      if(G_u16countSentBit == 2)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitBlue*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitCyan(void)
{
  LedOn(CYAN);
  LedOff(BLUE);
  LedOff(GREEN);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitBlue;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitGreen;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 111110
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
     if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
      if(G_u16countSentBit == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitCyan*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitGreen(void)
{
  LedOn(GREEN);
  LedOff(CYAN);
  LedOff(YELLOW);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitCyan;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitYellow;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 111010
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
     if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
      if(G_u16countSentBit == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitGreen*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitYellow(void)
{
  LedOn(YELLOW);
  LedOff(GREEN);
  LedOff(ORANGE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitGreen;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitOrange;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 110010
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
     if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
      if(G_u16countSentBit == 2)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitYellow*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitOrange(void)
{
  LedOn(ORANGE);
  LedOff(YELLOW);
  LedOff(RED);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitYellow;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitRed;
  }
 if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 101000
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 1)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitOrange*/

/*Wait for button1 or button2 to be pressed*/
static void ComSM_TransmitRed(void)
{
  LedOn(RED);
  LedOff(ORANGE);
  LedOff(WHITE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    Com_StateMachine = ComSM_TransmitOrange;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    Com_StateMachine = ComSM_TransmitWhite;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 111100
    if(G_u16countSentBit == 0)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 1)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 3)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 4)
    {  
      OffBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }
}/*end ComSM_TransmitRed*/


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void ComSM_Error(void)          
{
  
} /* end ComSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

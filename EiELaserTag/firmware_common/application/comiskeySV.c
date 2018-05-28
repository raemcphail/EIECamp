/**********************************************************************************************************************
File: comiskeySV.c                                                                

Description:
This is a comiskeySV.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32ComSVFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern volatile u16 G_u16countSentBit;                 /* From comiskey.c */

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type ComSV_StateMachine;            /* The state machine function pointer */
static u16 u16SVcountSentBit;
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: ComSVInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void ComSVInitialize(void)
{
 
  /*Set counter to 0 initially*/
  u16SVcountSentBit = 0;
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    ComSV_StateMachine = ComSVSM_TransmitWhite;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    ComSV_StateMachine = ComSVSM_Error;
  }

} /* end ComSVInitialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function ComSVRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void ComSVRunActiveState(void)
{
  ComSV_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void ComSVSM_Idle(void)
{
  
} /* end UserApp1SM_Idle() */
    

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void ComSVSM_Error(void)          
{
  
} /* end UserApp1SM_Error() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit white bit pattern */
static void ComSVSM_TransmitWhite(void)          
{
  LedOn(WHITE);
  LedOff(PURPLE);
  LedOff(RED);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitRed;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitPurple;
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
     if(G_u16countSentBit  == 2)
    {  
      OnBit();
    }
    if(G_u16countSentBit  == 3)
    {  
      OffBit();
    }
    if(G_u16countSentBit  == 4)
    {  
      OnBit();
    }
    if(G_u16countSentBit == 5)
    {  
      OffBit();
    }
  }

} /* end ComVSM_TransmitWhite() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit purple bit pattern */
static void ComSVSM_TransmitPurple(void)          
{
  LedOn(PURPLE);
  LedOff(WHITE);
  LedOff(BLUE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitWhite;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitBlue;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitPurple() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit blue bit pattern */
static void ComSVSM_TransmitBlue(void)          
{  
  LedOn(BLUE);
  LedOff(PURPLE);
  LedOff(CYAN);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitPurple;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitCyan;
  }
     if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
  
} /* end ComVSM_TransmitBlue() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit cyan bit pattern */
static void ComSVSM_TransmitCyan(void)          
{
  LedOn(CYAN);
  LedOff(BLUE);
  LedOff(GREEN);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitBlue;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitGreen;
  }
     if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitCyan() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit green bit pattern */
static void ComSVSM_TransmitGreen(void)          
{
  LedOn(GREEN);
  LedOff(CYAN);
  LedOff(YELLOW);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitCyan;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitYellow;
  }
     if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitGreen() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit yellow bit pattern */
static void ComSVSM_TransmitYellow(void)          
{
  LedOn(YELLOW);
  LedOff(GREEN);
  LedOff(ORANGE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitGreen;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitOrange;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitYellow() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit orange bit pattern */
static void ComSVSM_TransmitOrange(void)          
{
  LedOn(ORANGE);
  LedOff(YELLOW);
  LedOff(RED);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitYellow;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitRed;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitOrange() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit red bit pattern */
static void ComSVSM_TransmitRed(void)          
{
  LedOn(RED);
  LedOff(ORANGE);
  LedOff(WHITE);
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    ComSV_StateMachine = ComSVSM_TransmitOrange;
  }
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    ComSV_StateMachine = ComSVSM_TransmitWhite;
  }
  if(IsButtonPressed(BUTTON0))
  {
    //bit pattern transmitted is 
    //Campers will write there code here
  }
} /* end ComVSM_TransmitRed() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct USER_DATA {
	char buffer[BUFFER_MAX];
	uint8_t fieldCount;
	uint8_t fieldPosition[MAX_FIELDS];
	char fieldType[MAX_FIELDS];
} USER_DATA;

#define BLUE_LED_BBADDR *((volatile uint32_t*)(0x42000000 + (0x400253FC-0X40000000)*32 + 2*4))	//PF2
#define RED_LED_BBADDR  *((volatile uint32_t*)(0x42000000 + (0x400253FC-0X40000000)*32 + 1*4))	//PF1

volatile bool ParityToggle = 0;

// GLOBAL VARIABLES
volatile bool irDataRdy = 0;
volatile bool pcDataRdy = 0;

volatile USER_DATA irData = {0};
volatile USER_DATA pcData = {0};

void Timer0A_Handler(void) {
	//1. turn off flag
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
	
	//2. TURN OFF LED
	RED_LED_BBADDR = 0;
}

void UART0_Interrupt_Handler(void)
{
 // 1. READ Masked Interrupt Status_R
 volatile uint32_t status = UART0_MIS_R;

 // 2. HANDLE RX Interrupt, RX FIFO(1/4 FULL) or RT(Receive Timeout)
 if (status & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
 	UART0_ICR_R = (UART_MIS_RXMIS | UART_MIS_RTMIS);	// Clear flags 

 // 3. READ RX_FIFO. while not empty
 getsUart0(&pcData);
 
 pcDataRdy = 1;
}

void UART7_Interrupt_Handler(void)
{
 // 1. READ Masked Interrupt Status_R
 volatile uint32_t status = UART7_MIS_R;

 // 2. RX INTERRUPT
 // 2. HANDLE RX Interrupt, RX FIFO(1/4 FULL) or RT(Receive Timeout)
 if (status & (UART_MIS_RXMIS | UART_MIS_RTMIS)) {
 	UART7_ICR_R = (UART_MIS_RXMIS | UART_MIS_RTMIS);	// Clear flags

 // 3. READ RX_FIFO. while not empty

 BLUE_LED_BBADDR = 1;
 getsUart7(&irData);
 BLUE_LED_BBADDR = 0;
 irDataRdy = 1;

// 4. TX INTERRUPT
 if (status & UART_MIS_TXMIS) {
	// 1. CLEAR FLAG
	UART7_ICR_R = UART_MIS_TXMIS;
	
	// 2. WRITE ON FIFO
	char* str = getFieldString(&pcData, 2);
	putsUart7(str);

	// 3. TURN OFF interrupt
	UART7_IM_R &= ~UART_IM_TXIM;
 } 
// 5. PARITY ERROR
 else if (UART7_MIS_R & UART_DR_PE) {
	//1. TURN OFF FLAG
  UART7_ICR_R = UART_ICR_PEIC;

  // 2. TURN ON RED_LED
  RED_LED_BBADDR = 1;
  
  // 3. START TIMER0A
  // 3.1 TURN OFF INITIALLY
 TIMER0_CTL_R &= ~TIMER_CTL_TAEN;

  // 3.2 LOAD VALUE
 TIMER0_TAILR_R = 40000000 - 1;

  // 3.3 TURN BACK-ON
 TIMER0_CTL_R |= TIMER_CTL_TAEN;
 }
 
}

void main (void)
{
 initSystemClockTo40Mhz();
 
 initUart0();		// pc <> uart0
 initUart7();		// data serial
 initM0PWM0(); 		// 36k carrier
 initGPIOPFLED();	// GPIO PORTF ON-BOARD LEDs
 
 while (1) {
  ///////////////
  // A. PC -> IR
  if (pcDataRdy) {
    parseFields(&pcData);

    if (isCommand(&pcData, "change", 2)) {
            char* str = getFieldString(&pcData, 2);

            if (myStrCmp(str, "parity")) {
		// 1. TURN OFF FOR SAFETY
                UART7_CTL_R = 0;
		
		if (ParityToggle) {
		 UART7_LCRH_R &= ~UART_LCRH_PEN;	// PARITY DISABLE
		} else {
		 UART7_LCRH_R |= UART_LCRH_PEN;		// PARITY ENABLE	
		}
		
		// 3. TURN BACK ON AT THE END
		UART7_CTL_R = (UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN);

                putsUart0("IR serial format change completed/n");
            }
    } // change_end
    else if (isCommand(&pcData, "send", 10)) {
	UART7_IM_R |= UART_IM_TXIM;	// ENABLE INTERRUPT
    } // send_end
	
    pcDataRdy = 0;
  } // pcDataRdy_end
	
  /////////////////
  // B. IR -> PC
  if (irDataRdy) {
	char* str = getFieldString(&irData, 2);
	putsUart0("\nmsg read from IR: ");
	putsUart0(str);

	irDataRdy = 0;
  } //irDataRdy_end

 } // while end

}
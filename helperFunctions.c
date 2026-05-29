#define PB6_MASK 0x00000040	// m0pwm0

#define PF2_MASK 0X00000004	// BLUE_LED
#define PF1_MASK 0X00000002	// RED_LED

helper functions
void initTimer0OneShot(void) {
 // 1.ENABLE CLOCK
 SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R0;
 _delay_cycle(3);
 
 // 2.TURN OFF INITIALLY
 TIMER0_CTL_R &= ~TIMER_CTL_TAEN;

 // 3. 32bit mode (TIMER A+B)
 TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;
 
 // 4. One-shot mode
 TIMER0_TAMR_R = TIMER_TAMR_TAMR_1_SHOT; 
 
 // 5. Load 
 TIMER0_TAILR_R = 40000000 - 1;
 
 // 6. CONFIG INTERRUPT
 TIMER0_IM_R |= TIMER_IMR_TATOIM;

 // 7. NVIC (Timer0A - VEC35, IRQ19)
 NVIC_EN0_R = (1 << 19);
}

///////////////////////// BLUE_LED PF3 1000
void initGPIOPFLED()
{
 // ENABLE CLOCK
 SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
 _delay_cycle(3);
 
 GPIO_PORTF_DEN_R |= (PF1_MASK | PF2_MASK);
 GPIO_PORTF_DIR_R |=(PF1_MASK | PF2_MASK);
 GPIO_PORTF_DR2R_R |= (PF1_MASK | PF2_MASK);
}

////////////////////////////M0PWM0-PB6
void initM0PWM0(void)
{
 // 1.ENABLE CLOCK
 SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
 SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
 _delay_cycle(3);

 // 2.CONFIG GPIO
 GPIO_PORTB_DEN_R |= PB6_MASK;
 GPIO_PORTB_AFSEL_R |= PB6_MASK;
 GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB6_M0PWM0;
 
 // 3.COFIG RCC
 // pwm module will be configured using the system clock directly(without a pre-divider) 
 // to maximize the resolution of the 36khz carrier signal 
 // sysclk, pwmdivider -> Resoultion
 // Load16bit, make divider small or none
 // PWM Load value = pwm clock freq/ target freq
 // load = 40M /36K = 1,111
 // 1111 ticks in a period, 50% duty makes 555 ticks. 
 // more ticks more resolution
 SYSCTL_RCC_R &= ~SYSCTL_RCC_USEPWMDIV;

 // 4.CONFIG PWM GEN
 PWM0_0_CTL_R = 0;			// RESET
 PWM0_0_CTL_R &= ~PWM_0_CTL_MODE;	// COUNT DWN(0), UP(1)
 PWM0_0_GENA_R = (PWM_0_GENA_ACTLOAD_ONE | PWM_0_GENA_ACTCMPAD_ZERO);	//load->0 High //cmpa Low

 PWM0_0_LOAD_R = 1111 - 1;
 PWM0_0_CMPA_R = 0.5 * (1111 - 1);	// 50%

 // 5.START TIMER & ENABLE PWM OUTPUT
 PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
 PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;
}


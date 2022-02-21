/*
 * Hybrid Start
 * Main.c
 *
 *  Created on: Sep 29, 2021
 *      Author: janet
 */

#include "F28x_Project.h" // Includes device headerfile and examples

#define CPU_TIMER0_PRD 1999999 // Timer period = (10 ms / 5 ns) - 1
#define EPWM_TIMER_TBPRD 1664 // Switching selection parameter (60 kHz)

/* Calculating BRR: 7-bit baud rate register value */
// SPI CLK freq = 500 kHz
// LSPCLK freq = CPU freq / 4  (by default)
// BRR = (LSPCLK freq / SPI CLK freq) - 1

#if CPU_FRQ_200MHZ
#define SPI_BRR        ((200E6 / 4) / 500E3) - 1
#endif

#if CPU_FRQ_150MHZ
#define SPI_BRR        ((150E6 / 4) / 500E3) - 1
#endif

#if CPU_FRQ_120MHZ
#define SPI_BRR        ((120E6 / 4) / 500E3) - 1
#endif

/* -------------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* -------------------------------------------------------------------------------- */

/* Delays */
Uint32 startDelay = 35000000; // Variable for delaying 35 seconds (max.) for motor start-up
Uint32 Delay = 1000000; // Variable for 1 sec delay between steps
Uint16 dutyCycle = 1000; // Variable for duty cycle variations

/* Values to store sensed values for voltage/current */
Uint16 invOutCurrent = 0; // Result of ADC conversion for DC-AC inverter output current
Uint16 invOutVoltage = 0; // Result of ADC conversion for DC-AC inverter output voltage
Uint16 invOutVoltage1 = 0; // Result of ADC conversion for DC-AC inverter output voltagekl
Uint16 invOutVoltage2 = 0; // Result of ADC conversion for DC-AC inverter output voltage
Uint16 batteryVoltage = 0; // Result of ADC conversion for battery DC bus/DC-AC inverter input voltage
Uint16 gridCurrent = 0; // Result of ADC conversion for grid branch current
Uint16 gridVoltage = 0; // Result of ADC conversion for grid branch voltage
Uint16 gridVoltage1 = 0; // Result of ADC conversion for grid branch voltage
Uint16 gridVoltage2 = 0; // Result of ADC conversion for grid branch voltage

/* Voltage/current limits */
// ADCINA2
Uint16 invOutCurrentMin = 3445; // Min. tolerance error of -2% (Iin = 22.26 A - 2% = 21.8148 -> Vout = 2.52356 V -> 3445)
Uint16 LRA = 3460; // Value of locked rotor amperage current at inverter output (LRA = 22.26 A -> Vout = 2.53424 V -> 3460)
Uint16 invOutCurrentMax = 3474; // Max. tolerance error of +2% (Iin = 22.7052 A -> Vout = 2.545 V -> 3474)
Uint16 invOutCurrentFault = 3642; // Overcurrent limit (Iin = 27.825 A -> Vout = 2.6678 V -> 3642)
// ADCINB2 and ADCINC2
Uint16 invOutVoltageMin = 1611; // Min. tolerance error of -2% (Vin = 115 V - 2% = 112.7 V -> Vout = 1.18 V -> 1611)
Uint16 invOutVoltageExp = 1638; // Expected value of inverter output voltage (Vin = 115 V - > Vout = 1.2 V -> 1638)
Uint16 invOutVoltageMax = 1665; // Max. tolerance error of +2% (Vin = 115 V + 2% = 117.3 V -> Vout = 1.22 V -> 1665)
Uint16 invOutVoltageFault = 1911; // Overvoltage limit (Vin = 135 V -> Vout = 1.4 V -> 1911)
// ADCINB3
Uint16 batteryVoltageMin = 2641; // Normal operation charging mode (Vin = 12 V -> Vout = 1.935 V -> 2641)
Uint16 batteryVoltageFloat = 2999; // Float charging mode (Vin = 13.8 V -> Vout = 2.197 V -> 2999)
Uint16 batteryVoltageBoost = 3175; // Boost charging mode (Vin = 14.7 V -> Vout = 2.326 V -> 3175)
Uint16 batteryVoltageFault = 4096; // Overvoltage limit (Vin = 15 V -> Vout = 3 V -> 4096)
// ADCINA4
Uint16 gridCurrentMin = 2794; // Max. tolerance error of -2% (Iin = 2 A - 2% = 1.96 A -> Vout = 2.047 V -> 2794)
Uint16 FLA = 2796; //  Value of full-load amperage current at grid branch (FLA = 2 A -> Vout = 2.048 V -> 2796)
Uint16 gridCurrentMax = 2861; // Max. tolerance error of +2% (Iin = 4 A -> Vout = 2.096 V -> 2861)
Uint16 gridCurrentFault = 2894; // Overcurrent limit (Iin = 5 A -> Vout = 2.096 V -> 2894)
// ADCINB4 and ADCINC2
Uint16 gridVoltageMin = 1611; // Min. tolerance error of -2% (Vin = 115 V - 2% = 112.7 V -> Vout = 1.18 V -> 1611)
Uint16 gridVoltageExp = 1638; // Expected value of grid branch voltage (Vin = 115 V - > Vout = 1.2 V -> 1638)
Uint16 gridVoltageMax = 1665; // Max. tolerance error of +2% (Vin = 115 V + 2% = 117.3 V -> Vout = 1.22 V -> 1665)
Uint16 gridVoltageFault = 1911; // Overvoltage limit (Vin = 135 V -> Vout = 1.4 V -> 1911)

/* Variables for motor status detection */
Uint16 motorStatus = 0; // Indication of motor status (0: Off, 1: On)
Uint16 i = 0; // Counter for 1 ms of iterations
Uint16 initialValue = 0; // Initial value read for motor status

/* Variables for serial peripheral interface */
Uint16 sdata = 0; // Send data
Uint16 rdata = 0; // Receive data

/* -------------------------------------------------------------------------------- */
/* DECLARATION OF FUNCTIONS AND INTERRUPT SERVICE ROUTINES */
/* -------------------------------------------------------------------------------- */

void configurePWM2(void); // PWM output for DC-AC inverter output parameters
void configurePWM3(void); // PWM output for battery DC bus/DC-AC inverter input parameter
void configurePWM4(void); // PWM output for grid branch parameters
void configurePPB2Limits(void); // Configuration of PPB limits for DC-AC inverter output parameters
void configurePPB3Limits(void); // Configuration of PPB limits for battery DC bus/DC-AC inverter input parameter
void configurePPB4Limits(void); // Configuration of PPB limits for grid branch parameters
void detectMotorStatus(void); // Detection of motor status (0: OFF, 1: ON)
void displayMotorStatus(void); // Displaying of motor status via blue and red LEDs
void configureSpiaGPIO(void); // Configuration of SPI GPIO settings
void configureSpi(void); // Configuration of SPI to known state
void configureSpiFifo(void); // Configuration of SPI FIFO
void spi_xmit(Uint16 a); // Transmitting of data from SPI

interrupt void adcint2_isr(void); // ADC for DC-AC inverter output conversion triggered by CPU1 Timer0
interrupt void adcint3_isr(void); // ADC for battery DC bus/DC-AC inverter input conversion triggered by EOC2
interrupt void adcint4_isr(void); // ADC for grid branch conversion triggered by EOC3
interrupt void adca_ppb_isr(void); // Current fault PPB ISR triggered by ADCA events
interrupt void adcb_ppb_isr(void); // Voltage fault PPB ISR triggered by ADCB events
interrupt void adcc_ppb_isr(void); // Voltage fault PPB ISR triggered by ADCC events
interrupt void startupCycle_isr(void); // Overcurrent protection subroutine during start-up cycle using SPST switch
interrupt void emergencyDeactivation_isr(void); // Emergency deactivation system when using SPST switch
interrupt void powerSrcSwitch_isr(void); // Subroutine for switching between grid and battery using SPDT switch

/* -------------------------------------------------------------------------------- */
/* MAIN CODE */
/* -------------------------------------------------------------------------------- */

void main(void) {

    /* Initializing system control */
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2837xD_SysCtrl.c file.
    InitSysCtrl();

    /* Clearing all interrupts and initializing PIE vector table */
    DINT; // Disabling CPU interrupts

    /* Initializing PIE control registers to their default state */
    // The default state is all PIE interrupts disabled and flags are cleared.
    // This function is found in the F2837xD_PieCtrl.c file.
    InitPieCtrl();

    /* Disabling CPU interrupts and clearing all CPU interrupt flags */
    IER = 0x0000;
    IFR = 0x0000;

    /* Initializing the PIE vector table with pointers to the shell
       Interrupt Service Routines (ISR) */
    // This will populate the entire table, even if the interrupt
    // is not used in this example. This is useful for debug purposes.
    // The shell ISR routines are found in F2837xD_DefaultIsr.c.
    // This function is found in F2837xD_PieVect.c.
    InitPieVectTable();

    /* Enabling global interrupts and higher priority real-time debug events */
    // EINT;  // Enable Global interrupt INTM
    // ERTM;  // Enable Global realtime interrupt DBGM

    /* -------------------------------------------------------------------------------- */
    /* GPIO CONFIGURATION */
    /* -------------------------------------------------------------------------------- */

    /* Enabling register access */
    // GPIO Control registers are EALLOW protected
    EALLOW;

    /* Configuring SPI-A GPIO pins */
    configureSpiaGPIO();

    /* Configuring pins as GPIO */
    // GPAGMUX1, GPAMUX1: Front half (bits 0 - 15) of bits 0 - 31
    // GPAGMUX2, GPAMUX2: Back half (bits 16 - 31) of bits 0 - 31
    // GPBGMUX1, GPBMUX1: Front half (bits 32 - 47) of bits 32 - 63
    // GPBGMUX2, GPBMUX2: Back half (bits 48 - 63) of bits 32 - 63
    // GPCGMUX1, GPCMUX1: Front half (bits 64 - 79) of bits 64 - 95
    // GPCGMUX2, GPCMUX2: Back half (bits 80 - 95) of bits 64 - 95
    // GPDGMUX1, GPDMUX1: Front half (bits 96 - 111) of bits 96 - 127
    // GPDGMUX2, GPDMUX2: Back half (bits 112 - 127) of bits 96 - 127
    // GPEGMUX1, GPEMUX1: Front half (bits 128 - 143) of bits 128 - 159
    // GPEGMUX2, GPEMUX2: Back half (bits 144 - 159) of bits 128 - 159

    // GPIO2 <-> EPWM2A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;

    // GPIO3 <-> EPWM2B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;

    // GPIO4 <-> EPWM3A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;

    // GPIO5 <-> EPWM3B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;

    // GPIO6 <-> EPWM4A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;

    // GPIO7 <-> EPWM4B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;

    // GPIO8 <-> SPST switch for start-up cycle
    GpioCtrlRegs.GPAGMUX1.bit.GPIO8 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;

    // GPIO9 <-> SPST switch for emergency deactivation
    GpioCtrlRegs.GPAGMUX1.bit.GPIO9 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;

    // GPIO10 <-> SPDT switch for power source switching
    GpioCtrlRegs.GPAGMUX1.bit.GPIO10 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;

    // GPIO11 <-> Hall sensor
    GpioCtrlRegs.GPAGMUX1.bit.GPIO11 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0;

    // GPIO14 <-> PB1 for start/stop
    GpioCtrlRegs.GPAGMUX1.bit.GPIO14 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;

    // GPIO15 <-> PB2 for emergency deactivation
    GpioCtrlRegs.GPAGMUX1.bit.GPIO15 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;

    // GPIO31 <-> Blue LED (LD10)
    GpioCtrlRegs.GPAGMUX2.bit.GPIO31 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;

    // GPIO34 <-> Red LED (LD9)
    GpioCtrlRegs.GPBGMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;

    /* Defining direction (0: Input, 1: Output) */
    // Inputs
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 0; // Hall sensor
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 0; // PB1 for start/stop
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 0; // PB2 for emergency deactivation
    // Outputs
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; // Blue LED (LD10)
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // Red LED (LD9)
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1; // SPST switch for start-up cycle
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1; // SPST switch for emergency deactivation
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1; // SPDT switch for power source switching

    /* Inverting input logic automatically for active low signals */
    GpioCtrlRegs.GPAINV.bit.GPIO14 = 0; // PB1 for start/stop
    GpioCtrlRegs.GPAINV.bit.GPIO15 = 0; // PB2 for emergency deactivation
    GpioCtrlRegs.GPAINV.bit.GPIO11 = 1; // Hall sensor

    /* Synchronizing input qualification and sampling rate for external interrupts */
    // XINT1 (PB1 -> start-up cycle)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 1; // XINT1 qualification set to 3 samples
    GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 255; // Sampling period = 510 * SYSCLKOUT
    // XINT2 (PB2 -> emergency deactivation)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 1; // XINT2 qualification set to 3 samples
    GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 255; // Sampling period = 510 * SYSCLKOUT

    /* Disabling register access */
    EDIS;

    /* Entering initial value in GPIO output latches (0: Off, 1: On) */
    GpioDataRegs.GPASET.bit.GPIO8 = 0; // SPST OFF -> Open
    GpioDataRegs.GPASET.bit.GPIO9 = 0; // SPST OFF -> System deactivated
    GpioDataRegs.GPASET.bit.GPIO10 = 0; // SPDT OFF -> Grid (open)
    GpioDataRegs.GPASET.bit.GPIO31 = 0; // Blue LED (LD10) OFF
    GpioDataRegs.GPBSET.bit.GPIO34 = 0; // Red LED (LD9) OFF

    /* -------------------------------------------------------------------------------- */
    /* ADC CONFIGURATION */
    /* -------------------------------------------------------------------------------- */

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* ADC INTERRUPT CONFIGURATION */
    // Powering up and resetting ADC module
    // ADC clock speed = 50 MHz -> Prescale ratio = 4
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6;
    AdccRegs.ADCCTL2.bit.PRESCALE = 6;
    // Powering up ADC
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    // Setting interrupt pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    // 12-bit resolution, single-ended mode
    AdcSetMode(ADC_ADCA, 0, 0);
    AdcSetMode(ADC_ADCB, 0, 0);
    AdcSetMode(ADC_ADCC, 0, 0);

    /* Disabling register access */
    EDIS;

    /* Delaying at least 500 uS before using ADC */
    DELAY_US(500);

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* Setting up SOCA2 <-> ADCINA2 (inverter output current) */
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2; // Configuring SOCA2 to select channel ADCINA2
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 7; // Triggering on ePWM2, ADCSOCA

    /* Setting up SOCB2 <-> ADCINB2 (inverter output voltage 1) */
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2; // Configuring SOCB2 to select channel ADCINB2
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 8; // Triggering on ePWM2, ADCSOCB

    /* Setting up SOCB2 <-> ADCINC2 (inverter output voltage 2) */
    AdccRegs.ADCSOC2CTL.bit.CHSEL = 2; // Configuring SOCC2 to select channel ADCINC2
    AdccRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdccRegs.ADCSOC2CTL.bit.TRIGSEL = 8; // Triggering on ePWM2, ADCSOCC

    /* Setting up SOCB3 <-> ADCINB3 (battery DC bus voltage) */
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3; // Configuring SOCB3 to select channel ADCINB3
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = 10; // Triggering on ePWM3, ADCSOCB

    /* Setting up SOCA4 <-> ADCINA4 (grid branch current) */
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCA4 to select channel ADCINA4
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 11; // Triggering on ePWM4, ADCSOCA

    /* Setting up SOCB4 <-> ADCINB4 (grid branch voltage 1) */
    AdcbRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCB4 to select channel ADCINB4
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = 12; // Triggering on ePWM4, ADCSOCB

    /* Setting up SOCC4 <-> ADCINC4 (grid branch voltage 2) */
    AdccRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCC4 to select channel ADCINC4
    AdccRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdccRegs.ADCSOC4CTL.bit.TRIGSEL = 12; // Triggering on ePWM4, ADCSOCC

    /* Setting up EOC2 generated by SOC2 to trigger adcint2_isr */
    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 2; // EOC2 setting INT2 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = 2; // EOC2 setting INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag
    AdccRegs.ADCINTSEL1N2.bit.INT2SEL = 2; // EOC2 setting INT2 flag
    AdccRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdccRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag

    /* Setting up EOC3 generated by SOC3 to trigger adcint3_isr */
    AdcbRegs.ADCINTSEL3N4.bit.INT3SEL = 2; // End of SOC2 setting INT3 flag
    AdcbRegs.ADCINTSEL3N4.bit.INT3CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL3N4.bit.INT3E = 1; // Enabling INT3 flag

    /* Setting up EOC4 generated by SOC4 to trigger adcint4_isr */
    AdcaRegs.ADCINTSEL3N4.bit.INT4SEL = 4; // EOC4 setting INT4 flag
    AdcaRegs.ADCINTSEL3N4.bit.INT4CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcaRegs.ADCINTSEL3N4.bit.INT4E = 1; // Enabling INT4 flag
    AdcbRegs.ADCINTSEL3N4.bit.INT4SEL = 4; // EOC4 setting INT4 flag
    AdcbRegs.ADCINTSEL3N4.bit.INT4CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL3N4.bit.INT4E = 1; // Enabling INT4 flag
    AdccRegs.ADCINTSEL3N4.bit.INT4SEL = 4; // EOC4 setting INT4 flag
    AdccRegs.ADCINTSEL3N4.bit.INT4CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdccRegs.ADCINTSEL3N4.bit.INT4E = 1; // Enabling INT4 flag

    /* Disabling register access */
    EDIS;

    /* -------------------------------------------------------------------------------- */
    /* INTERRUPT CONFIGURATION */
    /* -------------------------------------------------------------------------------- */

    /* Enabling register access */
    // PWM clocks are EALLOW protected
    EALLOW;

    /* Redirecting address of ISRs stored in PIE Vector Table */
    PieVectTable.ADCA2_INT = &adcint2_isr;
    PieVectTable.ADCA3_INT = &adcint3_isr;
    PieVectTable.ADCA4_INT = &adcint4_isr;
    PieVectTable.ADCA_EVT_INT = &adca_ppb_isr;
    PieVectTable.ADCB_EVT_INT = &adcb_ppb_isr;
    PieVectTable.ADCC_EVT_INT = &adcc_ppb_isr;
    PieVectTable.XINT1_INT = &startupCycle_isr;
    PieVectTable.XINT2_INT = &emergencyDeactivation_isr;
    PieVectTable.TIMER0_INT = &powerSrcSwitch_isr;

    /* Disabling register access */
    EDIS;

    /* Enabling register access */
    // PWM clocks are EALLOW protected
    EALLOW;

    /* Disabling PWM clock to synchronize PWM signals */
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    /* Disabling register access */
    EDIS;

    /* Calling PWM functions */
    configurePWM2();
    configurePWM3();
    configurePWM4();

    /* Configuring PPB limits */
    configurePPB2Limits();
    configurePPB3Limits();
    configurePPB4Limits();

    /* CPU1 TIMER0 INTERRUPT CONFIGURATION */
    // Creating an interrupt every 10 ms
    CpuTimer0Regs.TCR.bit.TSS = 1; // Stopping timer
    CpuTimer0Regs.PRD.all = CPU_TIMER0_PRD; // Timer0 period = 10 ms
    CpuTimer0Regs.TCR.bit.TRB = 1; // Loading period value into timer
    CpuTimer0Regs.TCR.bit.TIE = 1; // Enabling Timer0 interrupt
    CpuTimer0Regs.TCR.bit.TSS = 0; // Decrementing timer value

    /* Enabling register access */
    // PWM clocks are EALLOW protected
    EALLOW;

    /* Enabling PWM clock to have PWM signals synchronized */
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; // Enabling SOCA
    EPwm2Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm2Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCA/B and enabling up-count mode
    EPwm3Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm3Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCB and enabling up-count mode
    EPwm4Regs.ETSEL.bit.SOCAEN = 1; // Enabling SOCA
    EPwm4Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm4Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCA/B and enabling up-count mode

    /* Selecting GPIO14 as source of XINT1 */
    GPIO_SetupXINT1Gpio(14); // GPIO14 (PB1) <-> XINT1

    /* Selecting GPIO15 as source of XINT2 */
    GPIO_SetupXINT2Gpio(15); // GPIO15 (PB2) <-> XINT2

    /* Disabling register access */
    EDIS;

    /* Configuring external interrupt register to trigger interrupts
       with rising clock edge and enabling interrupts XINT1 and XINT2 */
    XintRegs.XINT1CR.bit.POLARITY = 1; // Rising edge interrupt
    XintRegs.XINT1CR.bit.ENABLE = 1; // Enabling XINT1
    XintRegs.XINT2CR.bit.POLARITY = 1; // Rising edge interrupt
    XintRegs.XINT2CR.bit.ENABLE = 1; // Enabling XINT2

    /* Enabling interrupts in PIE block and at CPU level */
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enabling PIE block
    PieCtrlRegs.PIEIER10.bit.INTx2 = 1; // Enabling PIE Group 10, Interrupt 2 (for ADCA2)
    PieCtrlRegs.PIEIER10.bit.INTx6 = 1; // Enabling PIE Group 10, Interrupt 6 (for ADCB2)
    PieCtrlRegs.PIEIER10.bit.INTx10 = 1; // Enabling PIE Group 10, Interrupt 10 (for ADCC2)
    PieCtrlRegs.PIEIER10.bit.INTx7 = 1; // Enabling PIE Group 10, Interrupt 7 (for ADCB3)
    PieCtrlRegs.PIEIER10.bit.INTx4 = 1; // Enabling PIE Group 10, Interrupt 4 (for ADCA4)
    PieCtrlRegs.PIEIER10.bit.INTx8 = 1; // Enabling PIE Group 10, Interrupt 8 (for ADCB4)
    PieCtrlRegs.PIEIER10.bit.INTx12 = 1; // Enabling PIE Group 10, Interrupt 12 (for ADCC4)
    PieCtrlRegs.PIEIER10.bit.INTx1 = 1; // Enabling PIE Group 10, Interrupt 1 (for ADCA events)
    PieCtrlRegs.PIEIER10.bit.INTx5 = 1; // Enabling PIE Group 10, Interrupt 5 (for ADCB events)
    PieCtrlRegs.PIEIER10.bit.INTx9 = 1; // Enabling PIE Group 10, Interrupt 9 (for ADCC events)
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1; // Enabling PIE Group 1, Interrupt 4 (for XINT1)
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1; // Enabling PIE Group 1, Interrupt 5 (for XINT2)
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // Enabling PIE Group 1, Interrupt 7 (for Timer0)
    IER |= M_INT1; // Enabling CPU IER Group 1
    IER |= M_INT10; // Enabling CPU IER Group 10

    /* Enabling global interrupt mask INTM */
    EINT;

    /* Enabling global real-time interrupt DBGM */
    ERTM;

    /* Allowing for 35 seconds (max.) of delay for motor start-up */
//    DELAY_US(startDelay);

    /* Calling SPI functions */
    configureSpiFifo();

    for (;;) {

        /* Calling function DELAY_US(Delay) defined in F2837xD_Examples.h */
        // Takes input in microseconds
        DELAY_US(Delay);

        /* Calling motor status functions */
        detectMotorStatus();
        displayMotorStatus();
        DELAY_US(Delay);

        /* Transmitting data */
        spi_xmit(sdata);

        /* Waiting until data is received */
        while(SpiaRegs.SPIFFRX.bit.RXFFST != 1) {}
        
    }

}

/* -------------------------------------------------------------------------------- */
/* FUNCTIONS */
/* -------------------------------------------------------------------------------- */

/* CONFIGURING EPWM2A AND EPWM2B */
void configurePWM2(void) {

    EPwm2Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC2 on A group
    EPwm2Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC2 on B group
    EPwm2Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC2 on up-count
    EPwm2Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC2 on up-count
    EPwm2Regs.ETPS.bit.SOCAPRD = 1; // Generating pulse on first event
    EPwm2Regs.ETPS.bit.SOCBPRD = 1; // Generating pulse on first event
    EPwm2Regs.CMPA.bit.CMPA = invOutCurrent; // Setting Compare A value
    EPwm2Regs.CMPB.bit.CMPB = invOutVoltage1; // Setting Compare B value
    EPwm2Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm2Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING EPWM3A AND EPWM3B */
void configurePWM3(void) {

    EPwm3Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC3 on A group
    EPwm3Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC3 on B grouop
    EPwm3Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC3 on up-count
    EPwm3Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC3 on up-count
    EPwm3Regs.ETPS.bit.SOCAPRD = 1; // Generating pulse on first event
    EPwm3Regs.ETPS.bit.SOCBPRD = 1; // Generating pulse on first event
    EPwm3Regs.CMPB.bit.CMPB = batteryVoltage; // Setting Compare B value
    EPwm3Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm3Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING EPWM4A AND EPWM4B */
void configurePWM4(void) {

    EPwm4Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC4 on A group
    EPwm4Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC4 on B grouop
    EPwm4Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC4 on up-count
    EPwm4Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC4 on up-count
    EPwm4Regs.ETPS.bit.SOCAPRD = 1; // Generating pulse on first event
    EPwm4Regs.ETPS.bit.SOCBPRD = 1; // Generating pulse on first event
    EPwm4Regs.CMPA.bit.CMPA = gridCurrent; // Setting Compare A value
    EPwm4Regs.CMPB.bit.CMPB = gridVoltage1; // Setting Compare B value
    EPwm4Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm4Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING HIGH AND LOW LIMITS FOR ADC PPB2 */
void configurePPB2Limits(void) {

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* Associating SOC2 with PPB2 */
    AdcaRegs.ADCPPB2CONFIG.bit.CONFIG = 2;
    AdcbRegs.ADCPPB2CONFIG.bit.CONFIG = 2;
    AdccRegs.ADCPPB2CONFIG.bit.CONFIG = 2;

    /* Setting low and high limits */
//    AdcaRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutCurrentLow;
    AdcaRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutCurrentFault;
//    AdcbRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutVoltageMin;
    AdcbRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutVoltageFault;
//    AdccRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutVoltageMin;
    AdccRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutVoltageFault;

    /* Enabling low and high limit events to generate interrupts */
//    AdcaRegs.ADCEVTINTSEL.bit.PPB2TRIPLO = 1;
    AdcaRegs.ADCEVTINTSEL.bit.PPB2TRIPHI = 1;
//    AdcbRegs.ADCEVTINTSEL.bit.PPB2TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB2TRIPHI = 1;
//    AdccRegs.ADCEVTINTSEL.bit.PPB2TRIPLO = 1;
    AdccRegs.ADCEVTINTSEL.bit.PPB2TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* CONFIGURING HIGH AND LOW LIMITS FOR ADC PPB3 */
void configurePPB3Limits(void) {

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* Associating SOC3 with PPB3 */
    AdcaRegs.ADCPPB3CONFIG.bit.CONFIG = 3;
    AdcbRegs.ADCPPB3CONFIG.bit.CONFIG = 3;

    /* Setting low and high limits */
//    AdcbRegs.ADCPPB3TRIPLO.bit.LIMITLO = batteryVoltageMin;
    AdcbRegs.ADCPPB3TRIPHI.bit.LIMITHI = batteryVoltageFault;

    /* Enabling low and high limit events to generate interrupts */
//    AdcbRegs.ADCEVTINTSEL.bit.PPB3TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB3TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* CONFIGURING HIGH AND LOW LIMITS FOR ADC PPB4 */
void configurePPB4Limits(void) {

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* Associating SOC4 with PPB4 */
    AdcaRegs.ADCPPB4CONFIG.bit.CONFIG = 4;
    AdcbRegs.ADCPPB4CONFIG.bit.CONFIG = 4;
    AdccRegs.ADCPPB4CONFIG.bit.CONFIG = 4;

    /* Setting low and high limits */
//    AdcaRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridCurrentMin;
    AdcaRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridCurrentFault;
//    AdcbRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridVoltageMin;
    AdcbRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridVoltageFault;
    //    AdccRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridVoltageMin;
    AdccRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridVoltageFault;

    /* Enabling low and high limit events to generate interrupts */
//    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;
//    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;
//    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdccRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* DETECTING MOTOR STATUS (0: OFF, 1: ON) */
void detectMotorStatus(void) {

    initialValue = GpioDataRegs.GPADAT.bit.GPIO11;

    for (i = 1; i < 1000; i++) {

        /* Delaying at least 1 ms */
        DELAY_US(1000);

        /* Checking if status has changed */
        if (GpioDataRegs.GPADAT.bit.GPIO11 != initialValue) {

            motorStatus = 1;
            return;

        }

    }

    motorStatus = 0;
}

/* DISPLAYING MOTOR STATUS VIA BLUE AND RED LEDS */
void displayMotorStatus(void) {

    if (motorStatus) {

        /* Blue LED (LD10) ON */
        GpioDataRegs.GPASET.bit.GPIO31 = 1;

        /* Red LED (LD9) OFF */
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;

    }

    else {

        /* Blue LED (LD10) OFF */
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;

        /* Red LED (LD9) ON */
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;

    }

}

/* CONFIGURING SPI-A GPIO SETTINGS */
void configureSpiaGPIO(void) {
    
    /* Enabling register access */
    EALLOW;

    /* Enabling pull-up for selected GPIO pins */
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;  // Enabling pull-up on GPIO58 (SPISIMOA)
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0;  // Enabling pull-up on GPIO59 (SPISOMIA)
    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 0;  // Enabling pull-up on GPIO60 (SPICLKA)
    GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;  // Enabling pull-up on GPIO61 (SPISTEA)

    /* Setting asynchronous qualification for selected GPIO pins */
    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3; // Asynchronous input GPIO58 (SPISIMOA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 3; // Asynchronous input GPIO59 (SPISOMIA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3; // Asynchronous input GPIO60 (SPICLKA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 3; // Asynchronous input GPIO61 (SPISTEA)

    /* Selecting SPI functional pins using GPIO registers */
    // GPIO58 <-> SPISIMOA
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 1;
    // GPIO59 <-> SPISOMIA
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 1;
    // GPIO60 <-> SPICLKA
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 1;
    // GPIO61 <-> SPISTEA
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 1;

    /* Disabling register access */
    EDIS;

}

/* INITIALIZING SPI TO KNOWN STATE */
void configureSPI(void) {

    /* Initializing SPI-A */
    SpiaRegs.SPICCR.bit.SPISWRESET = 0; // Setting reset low before changing configuration
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0; // Clock polarity (0: Rising, 1: Falling)
    SpiaRegs.SPICCR.bit.SPICHAR = 15; // 16-bit character
    SpiaRegs.SPICCR.bit.SPILBK = 1; // Enabling loopback
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1; // Enabling master (0: Slave, 1: Master)
    SpiaRegs.SPICTL.bit.TALK = 1; // Enabling transmission
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0; // Setting clock phase to normal (0: Normal, 1: Delayed)
    SpiaRegs.SPICTL.bit.SPIINTENA = 0; // Disabling SPI interrupts

    /* Setting baud rate */
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = SPI_BRR;

    /* Setting emulation mode to free run */
    SpiaRegs.SPIPRI.bit.FREE = 1;

    /* Releasing SPI from reset */
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
    
}

/* INITIALIZING SPI FIFO */
void configureSpiFifo(void) {

    /* Initializing SPI FIFO registers */
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x2044;
    SpiaRegs.SPIFFCT.all = 0x0;

//    /* Initializing core SPI registers */
//    InitSpi();

}

/* TRANSMITTING DATA FROM SPI */
void spi_xmit(Uint16 a) {

    SpiaRegs.SPITXBUF = a;

}

/* -------------------------------------------------------------------------------- */
/* INTERRUPT SERVICE ROUTINES */
/* -------------------------------------------------------------------------------- */

/* AC-DC INVERTER OUTPUT ADC CONVERSION ISR TRIGGERED BY CPU1 TIMER0 */
interrupt void adcint2_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC2 conversion */
    invOutCurrent = AdcaResultRegs.ADCRESULT2;
    invOutVoltage1 = AdcbResultRegs.ADCRESULT2;
    invOutVoltage2 = AdccResultRegs.ADCRESULT2;
    invOutVoltage = invOutVoltage1 + invOutVoltage2;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* BATTERY DC BUS/DC-AC INVERTER INPUT ADC CONVERSION ISR TRIGGERED BY EOC2 */
interrupt void adcint3_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC3 conversion */
    batteryVoltage = AdcbResultRegs.ADCRESULT3;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* GRID BRANCH ADC CONVERSION ISR TRIGGERED BY EOC3 */
interrupt void adcint4_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC4 conversion */
    gridCurrent = AdcaResultRegs.ADCRESULT4;
    gridVoltage1 = AdcbResultRegs.ADCRESULT4;
    gridVoltage2 = AdccResultRegs.ADCRESULT4;
    gridVoltage = gridVoltage1 + gridVoltage2;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* CURRENT FAULT (OVERCURRENT) PPB ISR TRIGGERED BY ADCA EVENTS */
interrupt void adca_ppb_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Overcurrent conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    else if (AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Overcurrent conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* VOLTAGE FAULT (OVERVOLTAGE) PPB ISR TRIGERED BY ADCB EVENTS */
interrupt void adcb_ppb_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Overvoltage conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    else if (AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPHI) {

        /* Overvoltage conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB3TRIPHI = 1;

    }

    else if (AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Overvoltage conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* VOLTAGE FAULT (OVERVOLTAGE) PPB ISR TRIGERED BY ADCC EVENTS */
interrupt void adcc_ppb_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    if (AdccRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Overvoltage conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdccRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    else if (AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Overvoltage conditions */
        asm("     ESTOP0"); // Stopping DSP

        /* Clearing trip flags and continuing */
        AdccRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* OVERCURRENT PROTECTION ISR DURING START-UP CYCLE USING SPST SWITCH */
interrupt void startupCycle_isr(void) {

    /* Delaying during first 35 seconds of start-up cycle */
    DELAY_US(startDelay);

    /* Closing SPST switch after first 35 seconds of start-up cycle */
    GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

/* EMERGENCY SYSTEM DEACTIVATION ISR TRIGGERED BY PB2 */
interrupt void emergencyDeactivation_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Emergency deactivation */
    asm("     ESTOP0"); // Stopping DSP after PB2 pressed

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

/* POWER SOURCE SWITCHING (GRID <-> BATTERY) ISR TRIGGERED BY CPU1 TIMER0 */
// Using SPDT switch when appropriate conditions detected
interrupt void powerSrcSwitch_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* When motor ON, switch to battery until reaches grid branch current sensor
       reaches FLA, then switch back to grid and stay until next start-up */
    while (motorStatus) {

        GpioDataRegs.GPASET.bit.GPIO10 = 1; // SPDT On -> Battery

        if (gridCurrent == FLA) {

            GpioDataRegs.GPASET.bit.GPIO10 = 0; // SPDT Off -> Grid

        }

    }

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

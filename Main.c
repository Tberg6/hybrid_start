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

#define SPI_BRR 99

// SPI CLK freq = 500 kHz
// LSPCLK freq = CPU freq / 4  (by default)
// BRR = (LSPCLK freq / SPI CLK freq) - 1

#define SPI_BRR ((100E3 / 4) / 500E3) - 1 // LSPCLK freq = 100 kHz
#define SPI_BRR ((200000000 / 4) / 500000) - 1 // LSPCLK freq = 100 kHz

#if CPU_FRQ_200MHZ
#define SPI_BRR ((200E6 / 4) / 500E3) - 1
#endif

#if CPU_FRQ_150MHZ
#define SPI_BRR ((150E6 / 4) / 500E3) - 1
#endif

/* -------------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* -------------------------------------------------------------------------------- */

/* Delays */
Uint32 Delay = 1000000; // Variable for 1 sec delay between steps

/* Variables for storing values for sensed voltage/current */
Uint16 invOutCurrent = 0; // Result of ADC conversion for DC-AC inverter output current
Uint16 invOutVoltage = 0; // Result of ADC conversion for DC-AC inverter output voltage
Uint16 batteryVoltage = 0; // Result of ADC conversion for battery DC bus/DC-AC inverter input voltage
Uint16 gridCurrent = 0; // Result of ADC conversion for grid branch current
Uint16 gridVoltage = 0; // Result of ADC conversion for grid branch voltage

/* Voltage/current limits */
// ADCINA2
Uint16 invOutCurrentMin = 3222; // Undercurrent limit (Iin = 17.808 A -> Vin = 2.875 V -> Vout = 2.36 V -> 3222)
Uint16 LRA = 3460; // Value of locked rotor amperage current at inverter output (LRA = 22.26 A -> Vin = 3.08 V -> Vout = 2.534 V -> 3460)
Uint16 invOutCurrentMax = 3642; // Overcurrent limit (Iin = 27.825 A -> Vin = 3.243 V -> Vout = 2.66 V -> 3642)
// ADCINB2
Uint16 invOutVoltageMin = 2580; // Undervoltage limit (Vin = 176 V -> Vout = 1.89 V -> 2580)
Uint16 invOutVoltageExp = 3276; // Expected value of inverter output voltage (Vin = 220 V - > Vout = 2.4 V -> 3276)
Uint16 invOutVoltageMax = 4041; // Overvoltage limit (Vin = 270 V -> Vout = 2.96 V -> 4041)
// ADCINB3
Uint16 batteryVoltageMin = 1870; // Undervoltage limit (Vin = 9.6 V -> Vout = 1.37 V -> 1870)
Uint16 batteryVoltageNormal = 1870; // Normal operation charging mode (Vin = 12 V -> Vout = 1.71 V -> 2334)
Uint16 batteryVoltageFloat = 2689; // Float charging mode (Vin = 13.8 V -> Vout = 1.97 V -> 2689)
Uint16 batteryVoltageBoost = 2867; // Boost charging mode (Vin = 14.7 V -> Vout = 2.1 V -> 2867)
Uint16 batteryVoltageMax = 2921; // Overvoltage limit (Vin = 15 V -> Vout = 2.14 V -> 2921)
// ADCINA4
Uint16 gridCurrentMin = 2771; // Undercurrent limit (Iin = 1.6 A -> Vin = 2.47 V -> Vout = 2.03 V -> 2771)
Uint16 FLA = 2796; //  Value of full-load amperage current at grid branch (FLA = 2 A -> Vin = 2.489 V -> Vout = 2.048 V -> 2796)
Uint16 gridCurrentMax = 2894; // Overcurrent limit (Iin = 5 A -> Vin = 2.577 V -> Vout = 2.12 V -> 2894)
// ADCINB4
Uint16 gridVoltageMin = 2580; // Undervoltage limit (Vin = 176 V -> Vout = 1.89 V -> 2580)
Uint16 gridVoltageExp = 3276; // Expected value of inverter output voltage (Vin = 220 V - > Vout = 2.4 V -> 3276)
Uint16 gridVoltageMax = 4041; // Overvoltage limit (Vin = 270 V -> Vout = 2.96 V -> 4041)

/* Fault detection flags */
Uint16 invOut_OcFlg = 0; // Flag for overcurrent detection at inverter output
Uint16 invOut_UcFlg = 0; // Flag for undercurrent detection at inverter output
Uint16 invOut_OvFlg = 0; // Flag for overvoltage detection at inverter output
Uint16 invOut_UvFlg = 0; // Flag for undervoltage detection at inverter output
Uint16 battery_OvFlg = 0; // Flag for overvoltage detection at battery DC bus
Uint16 battery_UvFlg = 0; // Flag for undervoltage detection at battery DC bus
Uint16 grid_OcFlg = 0; // Flag for overcurrent detection at grid branch
Uint16 grid_UcFlg = 0; // Flag for undercurrent detection at grid branch
Uint16 grid_OvFlg = 0; // Flag for overvoltage detection at grid branch
Uint16 grid_UvFlg = 0; // Flag for undervoltage detection at grid branch

/* Variables for motor status detection */
Uint16 i = 0; // Counter for 1 ms of iterations
Uint16 motorStatus = 0; // Indication of motor status (0: Off, 1: On)
Uint16 initialStatus = 0; // Initial value read for motor status
Uint16 test = 0;
/* Variables for serial peripheral interface (SPI) protocol */
Uint16 sdata = 0;

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
void configureSpia(void); // Configuration of SPI-A to known state
void configureSpiFifo(void); // Configuration of SPI FIFO
void spi_xmit(Uint16 send); // Transmitting of data from SPI

interrupt void adcintA2_isr(void); // ADC for DC-AC inverter output current conversion triggered by CPU1 Timer0
interrupt void adcintB2_isr(void); // ADC for DC-AC inverter output voltage conversion triggered by CPU1 Timer0
interrupt void adcintB3_isr(void); // ADC for battery DC bus output/DC-AC inverter input voltage conversion triggered by EOC2
interrupt void adcintA4_isr(void); // ADC for grid branch current conversion triggered by EOC3
interrupt void adcintB4_isr(void); // ADC for grid branch voltage conversion triggered by EOC3
interrupt void adca_ppb_isr(void); // Current fault PPB ISR triggered by ADCA events
interrupt void adcb_ppb_isr(void); // Voltage fault PPB ISR triggered by ADCB events
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
    // GPIO control registers are EALLOW protected
    EALLOW;

    /* Enabling pull-up for selected GPIO pins */
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;  // Enabling pull-up on GPIO58 (SPISIMOA)
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0;  // Enabling pull-up on GPIO59 (SPISOMIA)
    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 0;  // Enabling pull-up on GPIO60 (SPICLKA)
    GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;  // Enabling pull-up on GPIO61 (SPISTEA)

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

     GPIO3 <-> EPWM2B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;

    // GPIO5 <-> EPWM3B
    GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;

    // GPIO6 <-> EPWM4A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;

     GPIO7 <-> EPWM4B
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

    // GPIO26 <-> Current fault flag
    GpioCtrlRegs.GPAGMUX2.bit.GPIO26 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;

    // GPIO27 <-> Voltage fault flag
    GpioCtrlRegs.GPAGMUX2.bit.GPIO27 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;

    // GPIO31 <-> Blue LED (LD10)
    GpioCtrlRegs.GPAGMUX2.bit.GPIO31 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;

    // GPIO34 <-> Red LED (LD9)
    GpioCtrlRegs.GPBGMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;

    // GPIO58 <-> SPISIMOA
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3;
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3;

    // GPIO59 <-> SPISOMIA
    GpioCtrlRegs.GPBGMUX2.bit.GPIO59 = 3;
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 3;

    // GPIO60 <-> SPICLKA
    GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 3;
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 3;

    // GPIO61 <-> SPISTEA
    GpioCtrlRegs.GPBGMUX2.bit.GPIO61 = 3;
    GpioCtrlRegs.GPBMUX2.bit.GPIO61 = 3;

    /* Defining direction (0: Input, 1: Output) */
    // Inputs
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 0; // Hall sensor
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 0; // PB1 for start/stop
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 0; // PB2 for emergency deactivation
    // Outputs
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1; // EPWM2A
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1; // EPWM2B
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 1; // EPWM3B
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1; // EPWM4A
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1; // EPWM4B
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1; // SPST switch for start-up cycle
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1; // SPST switch for emergency deactivation
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1; // SPDT switch for power source switching
    GpioCtrlRegs.GPADIR.bit.GPIO26 = 1; // Current fault flag
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1; // Voltage fault flag
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; // Blue LED (LD10)
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // Red LED (LD9)

    /* Inverting input logic automatically for active low signals */
    GpioCtrlRegs.GPAINV.bit.GPIO9 = 1; // SPST switch for emergency deactivation

    /* Setting asynchronous qualification for selected GPIO pins */
    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3; // Asynchronous input GPIO58 (SPISIMOA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 3; // Asynchronous input GPIO59 (SPISOMIA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3; // Asynchronous input GPIO60 (SPICLKA)
    GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 3; // Asynchronous input GPIO61 (SPISTEA)

    /* Synchronizing input qualification and sampling rate for external interrupts */
    // XINT1 (PB1 -> start-up cycle)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 1; // XINT1 qualification set to 3 samples
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 255; // Sampling period = 510 * SYSCLKOUT
    // XINT2 (PB2 -> emergency deactivation)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 1; // XINT2 qualification set to 3 samples
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 255; // Sampling period = 510 * SYSCLKOUT

    /* Disabling register access */
    EDIS;

    /* Entering initial value in GPIO output latches (0: Off, 1: On) */
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1; // EPWM2A Low
    GpioDataRegs.GPADCLEAR.bit.GPIO3 = 1; // EPWM2B Low
    GpioDataRegs.GPACLEAR.bit.GPIO5 = 1; // EPWM3B Low
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1; // EPWM4A Low
    GpioDataRegs.GPADAT.bit.GPIO7 = 1; // EPWM4B Low
    GpioDataRegs.GPASET.bit.GPIO8 = 1; // SPST ON -> Normal closed, start-up cycle open
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1; // SPST OFF -> System activated (open)
    GpioDataRegs.GPASET.bit.GPIO10 = 1; // SPDT ON -> Grid (open)
    GpioDataRegs.GPACLEAR.bit.GPIO26 = 1; // Current fault flag OFF
    GpioDataRegs.GPACLEAR.bit.GPIO27 = 1; // Voltage fault flag OFF
    GpioDataRegs.GPASET.bit.GPIO31 = 1; // Blue LED (LD10) OFF (active low)
    GpioDataRegs.GPBSET.bit.GPIO34 = 1; // Red LED (LD9) OFF (active low)

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
    // Powering up ADC
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    // Setting interrupt pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    // 12-bit resolution, single-ended mode
    AdcSetMode(ADC_ADCA, 0, 0);
    AdcSetMode(ADC_ADCB, 0, 0);

    /* Disabling register access */
    EDIS;

    /* Delaying at least 500 uS before using ADC */
    DELAY_US(500);

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* ADC SOC REGISTER CONFIGURATION */
    // SOCA2 <-> ADCINA2 conversion (inverter output current)
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2; // Configuring SOCA2 to select channel ADCINA2
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 1; // Triggering on CPU1 Timer0
    // SOCB2 <-> ADCINB2 conversion (inverter output voltage)
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2; // Configuring SOCB2 to select channel ADCINB2
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 1; // Triggering on CPU1 Timer0
    // SOCB3 <-> ADCINB3 conversion (battery DC bus voltage)
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3; // Configuring SOCB3 to select channel ADCINB3
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = 10; // Triggering on ePWM3, ADCSOCB
    // SOCA4 <-> ADCINA4 conversion (grid branch current)
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCA4 to select channel ADCINA4
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 11; // Triggering on ePWM4, ADCSOCA
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 1; // Triggering on CPU1 Timer0
    // SOCB4 <-> ADCINB4 conversion (grid branch voltage)
    AdcbRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCB4 to select channel ADCINB4
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = 12; // Triggering on ePWM4, ADCSOCB

    /* ADC EOC REGISTER CONFIGURATION */
    // EOC2 generated by SOC2 <-> adcintA2_isr trigger
    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 2; // EOC2 setting INT2 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag
    // EOC2 generated by SOC2 <-> adcintB2_isr trigger
    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = 2; // EOC2 setting INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag
    // EOC3 generated by SOC3 <-> adcintB3_isr trigger
    AdcbRegs.ADCINTSEL3N4.bit.INT3SEL = 3; // End of SOC3 setting INT3 flag
    AdcbRegs.ADCINTSEL3N4.bit.INT3CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL3N4.bit.INT3E = 1; // Enabling INT3 flag
    // EOC4 generated by SOC4 <-> adcintA4_isr trigger
    AdcaRegs.ADCINTSEL3N4.bit.INT4SEL = 4; // EOC4 setting INT4 flag
    AdcaRegs.ADCINTSEL3N4.bit.INT4CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcaRegs.ADCINTSEL3N4.bit.INT4E = 1; // Enabling INT4 flag
    // EOC4 generated by SOC4 <-> adcintB4_isr trigger
    AdcbRegs.ADCINTSEL3N4.bit.INT4SEL = 4; // EOC4 setting INT4 flag
    AdcbRegs.ADCINTSEL3N4.bit.INT4CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL3N4.bit.INT4E = 1; // Enabling INT4 flag

    /* Disabling register access */
    EDIS;

    /* -------------------------------------------------------------------------------- */
    /* INTERRUPT CONFIGURATION */
    /* -------------------------------------------------------------------------------- */

    /* Enabling register access */
    // PIE Vector Table is EALLOW protected
    EALLOW;

    /* REDIRECTING ISR ADDRESSES STORED IN PIE VECTOR TABLE */
    PieVectTable.ADCA2_INT = &adcintA2_isr;
    PieVectTable.ADCB2_INT = &adcintB2_isr;
    PieVectTable.ADCB3_INT = &adcintB3_isr;
    PieVectTable.ADCA4_INT = &adcintA4_isr;
    PieVectTable.ADCB4_INT = &adcintB4_isr;
    PieVectTable.ADCA_EVT_INT = &adca_ppb_isr;
    PieVectTable.ADCB_EVT_INT = &adcb_ppb_isr;
    PieVectTable.XINT1_INT = &startupCycle_isr;
    PieVectTable.XINT2_INT = &emergencyDeactivation_isr;
    PieVectTable.TIMER0_INT = &powerSrcSwitch_isr;

    /* Disabling register access */
    EDIS;

    /* Enabling register access */
    // CPU clocks are EALLOW protected
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

    /* Calling SPI functions */
    configureSpia();
    configureSpiFifo();

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

    /* EPWM EVENT-TRIGGER (ET) SUBMODULE CONFIGURATION */
    // Inverter output EPWM ET submodule
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; // Enabling SOCA
    EPwm2Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm2Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCA/B and enabling up-count mode
    // Battery DC bus EPWM ET submodule
    EPwm3Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm3Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCB and enabling up-count mode
    // Grid branch EPWM ET submodule
    EPwm4Regs.ETSEL.bit.SOCAEN = 1; // Enabling SOCA
    EPwm4Regs.ETSEL.bit.SOCBEN = 1; // Enabling SOCB
    EPwm4Regs.TBCTL.bit.CTRMODE = 0; // Unfreezing SOCA/B and enabling up-count mode

    /* Disabling register access */
    EDIS;

    /* Enabling register access */
    // GPIO control registers are EALLOW protected
    EALLOW;

    /* EXTERNAL INTERRUPT CONFIGURATION */
    // Selecting GPIO14 as source of XINT1
    GPIO_SetupXINT1Gpio(14); // GPIO14 (PB1) <-> XINT1
    // Selecting GPIO15 as source of XINT2
    GPIO_SetupXINT2Gpio(15); // GPIO15 (PB2) <-> XINT2

    /* Disabling register access */
    EDIS;

    /* Configuring external interrupt register to trigger interrupts
       with rising clock edge and enabling interrupts XINT1 and XINT2 */
    XintRegs.XINT1CR.bit.POLARITY = 1; // Rising edge interrupt
    XintRegs.XINT1CR.bit.ENABLE = 1; // Enabling XINT1
    XintRegs.XINT2CR.bit.POLARITY = 1; // Rising edge interrupt
    XintRegs.XINT2CR.bit.ENABLE = 1; // Enabling XINT2

    /* ENABLING INTERRUPTS IN PIE BLOCK AND AT CPU LEVEL */
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enabling PIE block
    PieCtrlRegs.PIEIER10.bit.INTx2 = 1; // Enabling PIE Group 10, Interrupt 2 (for ADCA2)
    PieCtrlRegs.PIEIER10.bit.INTx6 = 1; // Enabling PIE Group 10, Interrupt 6 (for ADCB2)
    PieCtrlRegs.PIEIER10.bit.INTx7 = 1; // Enabling PIE Group 10, Interrupt 7 (for ADCB3)
    PieCtrlRegs.PIEIER10.bit.INTx4 = 1; // Enabling PIE Group 10, Interrupt 4 (for ADCA4)
    PieCtrlRegs.PIEIER10.bit.INTx8 = 1; // Enabling PIE Group 10, Interrupt 8 (for ADCB4)
    PieCtrlRegs.PIEIER10.bit.INTx1 = 1; // Enabling PIE Group 10, Interrupt 1 (for ADCA events)
    PieCtrlRegs.PIEIER10.bit.INTx5 = 1; // Enabling PIE Group 10, Interrupt 5 (for ADCB events)
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1; // Enabling PIE Group 1, Interrupt 4 (for XINT1)
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1; // Enabling PIE Group 1, Interrupt 5 (for XINT2)
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // Enabling PIE Group 1, Interrupt 7 (for Timer0)
    IER |= M_INT1; // Enabling CPU IER Group 1
    IER |= M_INT10; // Enabling CPU IER Group 10

    /* Enabling global interrupt mask INTM */
    EINT;

    /* Enabling global real-time interrupt DBGM */
    ERTM;

    /* Allowing for 35 sec (max.) of delay for motor start-up */
    DELAY_US(startDelay);

    for (;;) {

        /* Delaying at least 1 sec between steps */
        DELAY_US(Delay);

        /* Calling motor status functions */
        detectMotorStatus();
        displayMotorStatus();
        DELAY_US(Delay);

        /* Transmitting voltage and current sensor readings to digital interface */
        // Inverter output voltage and current sensor readings
        spi_xmit(invOutCurrent);
        spi_xmit(invOutVoltage);
        DELAY_US(Delay);
        // Battery DC bus voltage sensor reading
        spi_xmit(batteryVoltage);
        DELAY_US(Delay);
        // Grid branch voltage and current sensor readings
        spi_xmit(gridCurrent);
        spi_xmit(gridVoltage);
        DELAY_US(Delay);

        /* Transmitting fault detection flags to digital interface */
        // Overcurrent at inverter output and grid branch
        if ((invOutCurrent > invOutCurrentMax) || (gridCurrent > gridCurrentMax)) {

            /* Setting and sending overcurrent flag to digital interface via SPI */
            invOut_OcFlg = 1;
            grid_OcFlg = 1;
            spi_xmit(invOut_OcFlg);
            spi_xmit(grid_OcFlg);

            /* Setting overcurrent flag */
            GpioDataRegs.GPASET.bit.GPIO26 = 1;

        }

        // Overvoltage at battery DC bus
        if (batteryVoltage > batteryVoltageMax) {

            /* Setting and sending overvoltage flag to digital interface via SPI */
            battery_OvFlg = 1;
            spi_xmit(battery_OvFlg);

            /* Setting overvoltage flag */
            GpioDataRegs.GPASET.bit.GPIO27 = 1;

        }

        /* Transmitting fault detection flags to digital interface */
        // Overcurrent at inverter output and grid branch
        if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPHI || AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

            /* Setting and sending overcurrent flag to digital interface via SPI */
            spi_xmit(invOut_OcFlg);
            spi_xmit(grid_OcFlg);

            /* Setting overcurrent flag */
            GpioDataRegs.GPASET.bit.GPIO26 = 1;

        }

        // Undercurrent at inverter output and grid branch
        if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPLO || AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPLO) {

            /* Setting and sending undercurrent flag to digital interface via SPI */
            spi_xmit(invOut_UcFlg);
            spi_xmit(grid_UcFlg);

        }

        // Overvoltage at inverter output, battery DC bus, and grid branch
        if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPHI || AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPHI || AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

            /* Setting and sending overvoltage flag to digital interface via SPI */
            spi_xmit(invOut_OvFlg);
            spi_xmit(battery_OvFlg);
            spi_xmit(grid_OvFlg);

            /* Setting overvoltage flag */
            GpioDataRegs.GPASET.bit.GPIO27 = 1;

        }

        // Overvoltage at battery DC bus
        if (AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPHI) {

            /* Setting and sending overvoltage flag to digital interface via SPI */
            spi_xmit(battery_OvFlg);

            /* Setting overvoltage flag */
            GpioDataRegs.GPASET.bit.GPIO27 = 1;

        }

        // Undervoltage at inverter output, battery DC bus, and grid branch
        if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPLO || AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPLO || AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPLO) {

            /* Setting and sending undervoltage flag to digital interface via SPI */
            spi_xmit(invOut_UvFlg);
            spi_xmit(battery_UvFlg);
            spi_xmit(grid_UvFlg);


        }

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
    EPwm2Regs.CMPB.bit.CMPB = invOutVoltage; // Setting Compare B value
    EPwm2Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm2Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING EPWM3B */
void configurePWM3(void) {

    EPwm3Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC3 on B grouop
    EPwm3Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC3 on up-count
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
    EPwm4Regs.CMPB.bit.CMPB = gridVoltage; // Setting Compare B value
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

    /* Setting low and high limits */
    AdcaRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutCurrentMin;
    AdcaRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutCurrentMax;
    AdcbRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutVoltageMin;
    AdcbRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutVoltageMax;

    /* Enabling low and high limit events to generate interrupts */
    AdcaRegs.ADCEVTINTSEL.bit.PPB2TRIPLO = 1;
    AdcaRegs.ADCEVTINTSEL.bit.PPB2TRIPHI = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB2TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB2TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* CONFIGURING HIGH AND LOW LIMITS FOR ADC PPB3 */
void configurePPB3Limits(void) {

    /* Enabling register access */
    // ADC Control registers are EALLOW protected
    EALLOW;

    /* Associating SOC3 with PPB3 */
    AdcbRegs.ADCPPB3CONFIG.bit.CONFIG = 3;

    /* Setting low and high limits */
    AdcbRegs.ADCPPB3TRIPLO.bit.LIMITLO = batteryVoltageMin;
    AdcbRegs.ADCPPB3TRIPHI.bit.LIMITHI = batteryVoltageMax;

    /* Enabling low and high limit events to generate interrupts */
    AdcbRegs.ADCEVTINTSEL.bit.PPB3TRIPLO = 1;
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

    /* Setting low and high limits */
    AdcaRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridCurrentMin;
    AdcaRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridCurrentMax;
    AdcbRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridVoltageMin;
    AdcbRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridVoltageMax;

    /* Enabling low and high limit events to generate interrupts */
    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* DETECTING MOTOR STATUS (0: OFF, 1: ON) */
void detectMotorStatus(void) {

    /* Reading initial status of motor */
    initialStatus = GpioDataRegs.GPADAT.bit.GPIO11;

    for (i = 1; i < 1000; i++) {
        test = 1;
        /* Delaying at least 1 ms */
        DELAY_US(1000);

        /* Detecting changes in motor position */
        if (GpioDataRegs.GPADAT.bit.GPIO11 != initialStatus) {

            /* Motor ON */
            motorStatus = 1;
            return; // Exiting out of function

        }

    }

    motorStatus = 0;

}

/* DISPLAYING MOTOR STATUS VIA BLUE AND RED LEDS */
void displayMotorStatus(void) {

    if (motorStatus) {

        /* Blue LED (LD10) ON */
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;

        /* Red LED (LD9) OFF */
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;

    }

    else {

        /* Blue LED (LD10) OFF */
        GpioDataRegs.GPASET.bit.GPIO31 = 1;

        /* Red LED (LD9) ON */
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;

    }

}

/* INITIALIZING SPI TO KNOWN STATE */
void configureSpia(void) {

    /* Enabling register access */
    // SPI Control registers are EALLOW protected
    EALLOW;

    /* Initializing SPI-A */
    SpiaRegs.SPIFFTX.bit.SPIFFENA = 1; // Enabling FIFO enhancements
    SpiaRegs.SPICCR.bit.SPISWRESET = 0; // Setting reset low before changing configuration
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 0; // Clock polarity (0: Rising, 1: Falling)
    SpiaRegs.SPICCR.bit.SPICHAR = 7; // 8-bit character
    SpiaRegs.SPICCR.bit.SPILBK = 0; // Disabling loopback
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 0; // Enabling slave mode (0: Slave, 1: Master)
    SpiaRegs.SPICTL.bit.TALK = 1; // Enabling transmission
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0; // Setting clock phase to normal (0: Normal, 1: Delayed)
    SpiaRegs.SPICTL.bit.SPIINTENA = 0; // Disabling SPI interrupts

    /* Setting baud rate */
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 2; // LSPCLK = / 4 (default on reset)
    SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = 3;

    /* Setting emulation mode to free run */
    SpiaRegs.SPIPRI.bit.FREE = 1;

    /* Releasing SPI from reset */
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;

    /* Disabling register access */
    EDIS;

}

/* INITIALIZING SPI FIFO */
void configureSpiFifo(void) {

    /* Initializing SPI FIFO registers */
    SpiaRegs.SPIFFTX.all = 57408; // 0xE040
    SpiaRegs.SPIFFRX.all = 8260; // 0x2044
    SpiaRegs.SPIFFCT.all = 0;

}

/* TRANSMITTING DATA FROM SPI */
void spi_xmit(Uint16 send) {

    /* Storing data into transmit buffer */
    SpiaRegs.SPITXBUF = send << 8; // Shifting by 8 bits for left-justification

}

/* -------------------------------------------------------------------------------- */
/* INTERRUPT SERVICE ROUTINES */
/* -------------------------------------------------------------------------------- */

/* AC-DC INVERTER OUTPUT CURRENT ADC CONVERSION ISR TRIGGERED BY CPU1 TIMER0 (ADCA2_INT) */
interrupt void adcintA2_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC2 conversion */
    invOutCurrent = AdcaResultRegs.ADCRESULT2;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

///* AC-DC INVERTER OUTPUT VOLTAGE ADC CONVERSION ISR TRIGGERED BY CPU1 TIMER0 (ADCB2_INT) */
//interrupt void adcintB2_isr(void) {
//
//    /* Delaying at least 1 sec */
//    DELAY_US(Delay);
//
//    /* Storing output of SOC2 conversion */
//    invOutVoltage = AdcbResultRegs.ADCRESULT2;
//
//    /* Acknowledging PIE Group 10 to reenable future interrupts */
//    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
//
//}

/* BATTERY DC BUS OUTPUT/DC-AC INVERTER INPUT VOLTAGE ADC CONVERSION ISR TRIGGERED BY EOC2 (ADCB3_INT) */
interrupt void adcintB3_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC3 conversion */
    batteryVoltage = AdcbResultRegs.ADCRESULT3;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* GRID BRANCH CURRENT ADC CONVERSION ISR TRIGGERED BY EOC3 (ADCA4_INT) */
interrupt void adcintA4_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC4 conversion */
    gridCurrent = AdcaResultRegs.ADCRESULT4;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* GRID BRANCH VOLTAGE ADC CONVERSION ISR TRIGGERED BY EOC3 (ADCB4_INT) */
interrupt void adcintB4_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Storing output of SOC4 conversion */
    gridVoltage = AdcbResultRegs.ADCRESULT4;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* CURRENT FAULT PPB ISR TRIGGERED BY ADCA EVENTS (ADCA_EVT_INT)*/
interrupt void adca_ppb_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Overcurrent fault detected at inverter output */
    if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Setting and sending fault detection flag to digital interface */
        invOut_OcFlg = 1; // Setting flag for overcurrent at inverter output
        spi_xmit(invOut_OcFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO26 = 1; // Setting current fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    /* Overcurrent fault detected at grid branch */
    else if (AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Setting and sending fault detection flag to digital interface */
        grid_OcFlg = 1; // Setting flag for overcurrent at grid branch
        spi_xmit(grid_OcFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO26 = 1; // Setting current fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Undercurrent fault detected at inverter output */
    if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPLO) {

        /* Setting and sending fault detection flag to digital interface */
        invOut_UcFlg = 1; // Setting flag for undercurrent at inverter output
        spi_xmit(invOut_UcFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO26 = 1; // Setting current fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB2TRIPLO = 1;

    }

    /* Undercurrent fault detected at grid branch */
    else if (AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPLO) {

        /* Setting and sending fault detection flag to digital interface */
        grid_UcFlg = 1; // Setting flag for undercurrent at grid branch
        spi_xmit(invOut_UcFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO26 = 1; // Setting current fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB4TRIPLO = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* VOLTAGE FAULT PPB ISR TRIGERED BY ADCB EVENTS (ADCB_EVT_INT)*/
interrupt void adcb_ppb_isr(void) {

    /* Delaying at least 1 sec */
    DELAY_US(Delay);

    /* Overvoltage fault detected at inverter output */
    if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Setting and sending fault detection flag to digital interface */
        invOut_OvFlg = 1; // Setting flag for overvoltage at inverter output
        spi_xmit(invOut_OvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

   /* Overvoltage fault detected at battery DC bus */
    else if (AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPHI) {

        /* Setting and sending fault detection flag to digital interface */
        battery_OvFlg = 1; // Setting flag for overvoltage at battery DC bus
        spi_xmit(battery_OvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB3TRIPHI = 1;

    }

    /* Overvoltage fault detected at grid branch */
    else if (AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Setting and sending fault detection flag to digital interface */
        grid_OvFlg = 1; // Setting flag for overvoltage at grid branch
        spi_xmit(grid_OvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Undervoltage fault detected at inverter output */
    if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPLO) {

        /* Setting and sending fault detection flag to digital interface */
        invOut_UvFlg = 1; // Setting flag for undervoltage at inverter output
        spi_xmit(invOut_UvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB2TRIPLO = 1;

    }

    /* Undervoltage fault detected at battery DC bus */
    else if (AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPLO) {

        /* Setting and sending fault detection flag to digital interface */
        battery_UvFlg = 1; // Setting flag for undervoltage at battery DC bus
        spi_xmit(battery_UvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB3TRIPLO = 1;

    }

    /* Undervoltage fault detected at branch */
    else if (AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPLO) {

        /* Setting and sending fault detection flag to digital interface */
        grid_UvFlg = 1; // Setting flag for undervoltage at grid branch
        spi_xmit(grid_UvFlg); // Sending flag status via SPI
        GpioDataRegs.GPASET.bit.GPIO27 = 1; // Setting voltage fault flag

        /* Closing SPST switch to deactivate system */
        GpioDataRegs.GPASET.bit.GPIO9 = 1;

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB4TRIPLO = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* OVERCURRENT PROTECTION ISR DURING START-UP CYCLE TRIGGERED BY PB1 (XINT1_INT) */
// Using SPST charging switch to open during start-up cycle and close
interrupt void startupCycle_isr(void) {
 test = 1;
    /* Closing SPST switch for start-up cycle */
    GpioDataRegs.GPATOGGLE.bit.GPIO8 = 1;

    /* Closing SPST switch for emergency deactivation */
    GpioDataRegs.GPATOGGLE.bit.GPIO9 = 1;

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

/* EMERGENCY SYSTEM DEACTIVATION ISR TRIGGERED BY PB2 (XINT2_INT) */
interrupt void emergencyDeactivation_isr(void) {

    /* Closing SPST switch for emergency deactivation */
    GpioDataRegs.GPATOGGLE.bit.GPIO9 = 1;

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

/* POWER SOURCE SWITCHING (GRID <-> BATTERY) ISR TRIGGERED BY CPU1 TIMER0 (TIMER0_INT) */
// Using SPDT switch to switch between battery and grid power when appropriate conditions detected
interrupt void powerSrcSwitch_isr(void) {

    /* Waiting 35 seconds for start-up cycle to complete */
    DELAY_US(35000000);

    /* When motor ON, switch to battery until grid branch current sensor
       reaches FLA, then switch back to grid and stay until next start-up */
    while (motorStatus) {

        GpioDataRegs.GPASET.bit.GPIO10 = 1; // SPDT ON -> Battery

        if (gridCurrent == FLA) {

            GpioDataRegs.GPACLEAR.bit.GPIO10 = 1; // SPDT OFF -> Grid

        }

        return; // Exiting out of function

    }

    /* Acknowledging PIE Group 1 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}

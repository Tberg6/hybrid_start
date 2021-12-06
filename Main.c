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

/* -------------------------------------------------------------------------------- */
/* GLOBAL VARIABLES */
/* -------------------------------------------------------------------------------- */

/* Delays */
Uint32 startDelay = 35000000; // Variable for delaying 35 seconds (max.) for motor start-up
Uint32 Delay = 1000000; // Variable for 1 sec delay between steps
Uint16 duty_cycle = 1000; // Variable for duty cycle variations

/* Values to store monitored values for voltage/current */
Uint16 invOutCurrent = 0; // Result of ADC conversion for DC-AC inverter output current
Uint16 invOutVoltage = 0; // Result of ADC conversion for DC-AC inverter output voltage
Uint16 batteryVoltage = 0; // Result of ADC conversion for battery DC bus/DC-AC inverter input voltage
Uint16 gridCurrent = 0; // Result of ADC conversion for grid branch current
Uint16 gridVoltage = 0; // Result of ADC conversion for grid branch voltage

/* Voltage/current limits */
// ADCINA2
Uint16 LRA = 3713; // Value of locked rotor amperage current at inverter output (LRA = 30 A -> Vout = 2.72 V -> 3713)
Uint16 invOutCurrentMin = 3694; // Min. tolerance error of -2% (Iin = 30 A - 2% = 29.4 -> Vout = 2.7056 -> 3694)
Uint16 invOutCurrentMax = 3733; // Max. tolerance error of +2% (Iin = 30.6 A -> Vout = 2.7344 V -> 3733)
Uint16 invOutCurrentFault = 3943; // Overcurrent limit (Iin = 37.5 A -> Vout = 2.888 V -> 3943)
// ADCINB2
Uint16 invOutVoltageExp = 3426; // Expected value of inverter output voltage (Vin = 230 V - > Vout = 2.51 V -> 3426)
Uint16 invOutVoltageMin = 3358; // Min. tolerance error of -2% (Vin = 230 V - 2% = 225.4 V -> Vout = 2.61 V -> 3358)
Uint16 invOutVoltageMax = 3495; // Max. tolerance error of +2% (Vin = 230 V + 2% = 234.6 V -> Vout = 2.72 V -> 3495)
Uint16 invOutVoltageFault = 4055; // Overvoltage limit (Vin = 260 V -> Vout = 3 V -> 4055)
// ADCINB3
Uint16 batteryVoltageMin = 3276; // Normal operation charging mode (Vin = 12 V -> Vout = 2.4 V -> 3276)
Uint16 batteryVoltageFloat = 3768; // Float charging mode (Vin = 13.8 V -> Vout = 2.76 V -> 3768)
Uint16 batteryVoltageBoost = 4014; // Boost charging mode (Vin = 14.7 V -> Vout = 2.94 V -> 4014)
Uint16 batteryVoltageFault = 4096; // Overvoltage limit (Vin = 15 V -> Vout = 3 V -> 4096)
// ADCINA4
Uint16 FLA = 2796; //  Value of full-load amperage current at grid branch (FLA = 2 A -> Vout = 2.048 V -> 2796)
Uint16 gridCurrentMin = 2794; // Max. tolerance error of -2% (Iin = 2 A - 2% -> Vout = 2.047 V -> 2794)
Uint16 gridCurrentMax = 2861; // Max. tolerance error of +2% (Iin = 4 A -> Vout = 2.096 V -> 2861)
Uint16 gridCurrentFault = 2894; // Overcurrent limit (Iin = 5 A -> Vout = 2.096 V -> 2894)
// ADCINB4
Uint16 gridVoltageExp = 3426; // Expected value of grid branch voltage (Vin = 230 V - > Vout = 2.51 V -> 3426)
Uint16 gridVoltageMin = 3358; // Min. tolerance error of +2% (Vin = 230 V - 2% = 225.4 V -> Vout = 2.61 V -> 3358)
Uint16 gridVoltageMax = 3495; // Max. tolerance error of +2% (Vin = 230 V + 2% = 234.6 V -> Vout = 2.72 V -> 3495)
Uint16 gridVoltageFault = 4055; // Overvoltage limit (Vin = 260 V -> Vout = 2.97 V -> 4055)

Uint16 motorStatus = 0; // Indication of motor status (0: Off, 1: On)

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

interrupt void adcint2_isr(void); // ADC for DC-AC inverter output conversion triggered by CPU1 Timer0
interrupt void adcint3_isr(void); // ADC for battery DC bus/DC-AC inverter input conversion triggered by EOC2
interrupt void adcint4_isr(void); // ADC for grid branch conversion triggered by EOC3
interrupt void adca_ppb_isr(void); // Current fault PPB ISR triggered by ADCA events
interrupt void adcb_ppb_isr(void); // Voltage fault PPB ISR triggered by ADCB events
//interrupt void powerSrcSwitch_isr(void); // Subroutine for switching between grid and battery using SPDT switch // ECEN 404

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

    /* Configuring pins as GPIO */
    // GPAGMUX1, GPAMUX1: Front half (bits 0 - 15) of bits 0 - 31
    // GPAGMUX2, GPAMUX2: Back half (bits 16 - 31) of bits 0 - 31
    // GPBGMUX1, GPBMUX1: Front half (bits 32 - 47) of bits 32 - 63
    // GPBGMUX2, GPBMUX2: Back half (bits 48 - 63) of bits 32 - 63

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

    // GPIO31 <-> Blue LED (LD10)
    GpioCtrlRegs.GPAGMUX2.bit.GPIO31 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;

    // GPIO34 <-> Red LED (LD9)
    GpioCtrlRegs.GPBGMUX1.bit.GPIO34 = 0;
    GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;

    // GPIO58 <-> Hall sensor
    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 0;

//    // GPIO40 <-> SPST switch
//    GpioCtrlRegs.GPBGMUX1.bit.GPIO40 = 0;
//    GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;
//
//    // GPIO41 <-> SPDT switch
//    GpioCtrlRegs.GPBGMUX1.bit.GPIO41 = 0;
//    GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;

    /* Defining direction (0: Input, 1: Output) */
    // Inputs
    GpioCtrlRegs.GPBDIR.bit.GPIO58 = 0; // Hall sensor
    // Outputs
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; // Blue LED (LD10)
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // Red LED (LD9)
//    GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1; // SPST switch
//    GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1; // SPDT switch

    /* Inverting input logic automatically for active low signals */
    GpioCtrlRegs.GPAINV.bit.GPIO31 = 1; // Blue LED (LD10)
    GpioCtrlRegs.GPBINV.bit.GPIO34 = 1; // Red LED (LD9)
    GpioCtrlRegs.GPBINV.bit.GPIO58 = 1; // Hall sensor

    /* Disabling register access */
    EDIS;

    /* Entering initial value in GPIO output latches (0: Off, 1: On) */
    GpioDataRegs.GPASET.bit.GPIO31 = 0; // Blue LED (LD10) OFF
    GpioDataRegs.GPBSET.bit.GPIO34 = 0; // Red LED (LD9) OFF
//    GpioDataRegs.GPBSET.bit.GPIO40 = 0; // SPST Off -> System deactivated
//    GpioDataRegs.GPBSET.bit.GPIO41 = 0; // SPDT Off -> Grid

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

    /* Setting up SOCA2 <-> ADCINA2 (inverter output current) */
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 1; // Configuring SOCA2 to select channel ADCINA2
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 7; // Triggering on ePWM2, ADCSOCA

    /* Setting up SOCB2 <-> ADCINB2 (inverter output voltage) */
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 1; // Configuring SOCB2 to select channel ADCINB2
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 8; // Triggering on ePWM2, ADCSOCB

    /* Setting up SOCB3 <-> ADCINB3 (battery DC bus voltage) */
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3; // Configuring SOCB3 to select channel ADCINB3
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = 10; // Triggering on ePWM3, ADCSOCB

    /* Setting up SOCA4 <-> ADCINA4 (grid branch current) */
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCA4 to select channel ADCINA4
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 11; // Triggering on ePWM4, ADCSOCA

    /* Setting up SOCB4 <-> ADCINB4 (grid branch voltage) */
    AdcbRegs.ADCSOC4CTL.bit.CHSEL = 4; // Configuring SOCB4 to select channel ADCINB4
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = 39; // Sampling window = (0.2 us / 5 ns) = 40 SYSCLK cycles
    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = 12; // Triggering on ePWM4, ADCSOCB

    /* Setting up EOC2 generated by SOC2 to trigger adcint2_isr */
    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 1; // EOC2 setting INT2 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = 1; // EOC2 setting INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2CONT = 1; // Generating interrupts whether ADCINTFLG cleared or not
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1; // Enabling INT2 flag

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
//    PieVectTable.ADCA3_INT = &powerSrcSwitch_isr;

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
    CpuTimer0Regs.TCR.bit.TSS = 0; // Restarting timer

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

    /* Disabling register access */
    EDIS;

    /* Enabling interrupts in PIE block and at CPU level */
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enabling PIE block
    PieCtrlRegs.PIEIER10.bit.INTx2 = 1; // Enabling PIE Group 10, Interrupt 2 (for ADCA2)
    PieCtrlRegs.PIEIER10.bit.INTx6 = 1; // Enabling PIE Group 10, Interrupt 6 (for ADCB2)
    PieCtrlRegs.PIEIER10.bit.INTx7 = 1; // Enabling PIE Group 10, Interrupt 7 (for ADCB3)
    PieCtrlRegs.PIEIER10.bit.INTx4 = 1; // Enabling PIE Group 10, Interrupt 4 (for ADCA4)
    PieCtrlRegs.PIEIER10.bit.INTx8 = 1; // Enabling PIE Group 10, Interrupt 8 (for ADCB4)
    PieCtrlRegs.PIEIER10.bit.INTx1 = 1; // Enabling PIE Group 10, Interrupt 1 (for ADCA events)
    PieCtrlRegs.PIEIER10.bit.INTx5 = 1; // Enabling PIE Group 10, Interrupt 5 (for ADCB events)
    IER |= M_INT10; // Enabling CPU IER Group 10

    /* Enabling global interrupt mask INTM */
    EINT;

    /* Enabling global real-time interrupt DBGM */
    ERTM;

    /* Allowing for 35 seconds (max.) of delay for motor start-up */
    DELAY_US(startDelay);

    for (;;) {

        /* Calling motor status functions */
        detectMotorStatus();
        displayMotorStatus();
        DELAY_US(Delay);

    }

}

/* -------------------------------------------------------------------------------- */
/* FUNCTIONS */
/* -------------------------------------------------------------------------------- */

/* CONFIGURING EPWM2A AND EPWM2B */
void configurePWM2(void) {

    EPwm2Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC on A group
    EPwm2Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC on B group
    EPwm2Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC on up-count
    EPwm2Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC on up-count
    EPwm2Regs.ETPS.bit.SOCAPRD = 1; // Generating pulse on first event
    EPwm2Regs.ETPS.bit.SOCBPRD = 1; // Generating pulse on first event
    EPwm2Regs.CMPA.bit.CMPA = invOutCurrent; // Setting Compare A value
    EPwm2Regs.CMPB.bit.CMPB = invOutVoltage; // Setting Compare B value
    EPwm2Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm2Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING EPWM3A AND EPWM3B */
void configurePWM3(void) {

    EPwm3Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC on A group
    EPwm3Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC on B grouop
    EPwm3Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC on up-count
    EPwm3Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC on up-count
    EPwm3Regs.ETPS.bit.SOCAPRD = 1; // Generating pulse on first event
    EPwm3Regs.ETPS.bit.SOCBPRD = 1; // Generating pulse on first event
    EPwm3Regs.CMPB.bit.CMPB = batteryVoltage; // Setting Compare B value
    EPwm3Regs.TBPRD = EPWM_TIMER_TBPRD; // Setting timer period
    EPwm3Regs.TBCTL.bit.CTRMODE = 3; // Freezing counter

}

/* CONFIGURING EPWM4A AND EPWM4B */
void configurePWM4(void) {

    EPwm4Regs.ETSEL.bit.SOCAEN = 0; // Disabling SOC on A group
    EPwm4Regs.ETSEL.bit.SOCBEN = 0; // Disabling SOC on B grouop
    EPwm4Regs.ETSEL.bit.SOCASEL = 4; // Selecting SOC on up-count
    EPwm4Regs.ETSEL.bit.SOCBSEL = 4; // Selecting SOC on up-count
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
//    AdcaRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutCurrentLow;
    AdcaRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutCurrentFault;
//    AdcbRegs.ADCPPB2TRIPLO.bit.LIMITLO = invOutVoltageMin;
    AdcbRegs.ADCPPB2TRIPHI.bit.LIMITHI = invOutVoltageFault;

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
    AdcaRegs.ADCPPB2CONFIG.bit.CONFIG = 3;
    AdcbRegs.ADCPPB2CONFIG.bit.CONFIG = 3;

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

    /* Setting low and high limits */
//    AdcaRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridCurrentMin;
    AdcaRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridCurrentFault;
//    AdcbRegs.ADCPPB4TRIPLO.bit.LIMITLO = gridVoltageMin;
    AdcbRegs.ADCPPB4TRIPHI.bit.LIMITHI = gridVoltageFault;

    /* Enabling low and high limit events to generate interrupts */
//    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcaRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;
//    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPLO = 1;
    AdcbRegs.ADCEVTINTSEL.bit.PPB4TRIPHI = 1;

    /* Disabling register access */
    EDIS;

}

/* DETECTING MOTOR STATUS (0: OFF, 1: ON) */
void detectMotorStatus(void) {

    initialValue = GpioDataRegs.GPBDAT.bit.GPIO58;

    for (i = 1; i < 1000; i++) {

        DELAY_US(1000);

        /* Checking if status has changed */
        if (GpioDataRegs.GPBDAT.bit.GPIO58 != initialValue) {

            motorStatus = 1;
            return;

        }

    }

    motorStatus = 0;
}

/* DISPLAYING MOTOR STATUS VIA BLUE AND RED LEDS */
void displayMotorStatus(void) {

    if (motorStatus) {

        /* Blue LED (LD10) OFF */
        GpioDataRegs.GPASET.bit.GPIO31 = 1;

        /* Red LED (LD9) ON */
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;

    }

    else {

        /* Blue LED (LD10) ON */
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;

        /* Red LED (LD9) OFF */
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;

    }

}

/* -------------------------------------------------------------------------------- */
/* INTERRUPT SERVICE ROUTINES */
/* -------------------------------------------------------------------------------- */

/* AC-DC INVERTER OUTPUT ADC CONVERSION ISR TRIGGERED BY CPU1 TIMER0 */
interrupt void adcint2_isr(void) {

    /* Storing output of SOC2 conversion */
    invOutCurrent = AdcaResultRegs.ADCRESULT2;
    invOutVoltage = AdcbResultRegs.ADCRESULT2;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* BATTERY DC BUS/DC-AC INVERTER INPUT ADC CONVERSION ISR TRIGGERED BY EOC2 */
interrupt void adcint3_isr(void) {

    /* Storing output of SOC3 conversion */
    batteryVoltage = AdcbResultRegs.ADCRESULT3;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* GRID BRANCH ADC CONVERSION ISR TRIGGERED BY EOC3 */
interrupt void adcint4_isr(void) {

    /* Storing output of SOC4 conversion */
    gridCurrent = AdcaResultRegs.ADCRESULT4;
    gridVoltage = AdcbResultRegs.ADCRESULT4;

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* CURRENT FAULT (OVERCURRENT) PPB ISR TRIGGERED BY ADCA EVENTS */
interrupt void adca_ppb_isr(void) {

    if (AdcaRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Overcurrent conditions */
        ESTOP0;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    else if (AdcaRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Overcurrent conditions */
        ESTOP0;

        /* Clearing trip flags and continuing */
        AdcaRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

/* VOLTAGE FAULT (OVERVOLTAGE) PPB ISR TRIGERED BY ADCB EVENTS */
interrupt void adcb_ppb_isr(void) {

    if (AdcbRegs.ADCEVTSTAT.bit.PPB2TRIPHI) {

        /* Overcurrent conditions */
        ESTOP0; // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB2TRIPHI = 1;

    }

    else if (AdcbRegs.ADCEVTSTAT.bit.PPB3TRIPHI) {

        /* Overcurrent conditions */
        ESTOP0; // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB3TRIPHI = 1;

    }

    else if (AdcbRegs.ADCEVTSTAT.bit.PPB4TRIPHI) {

        /* Overcurrent conditions */
        ESTOP0; // Stopping DSP

        /* Clearing trip flags and continuing */
        AdcbRegs.ADCEVTCLR.bit.PPB4TRIPHI = 1;

    }

    /* Acknowledging PIE Group 10 to reenable future interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

}

///* FOR ECEN 404 */
///* POWER SOURCE SWITCHING (GRID <-> BATTERY) ISR TRIGGERED BY CPU1 TIMER0 */
//// Using SPDT switch when appropriate conditions detected
//interrupt void powerSrcSwitch_isr(void) {
//
//    /* Calling function DELAY_US(Delay) defined in F2837xD_Examples.h */
//    // Takes input in microseconds
//    DELAY_US(Delay);
//
//    /* When motor ON, switch to battery until reaches grid branch current sensor
//       reaches FLA, then switch back to grid and stay until next start-up */
//    while (motorStatus) {
//
//        GpioDataRegs.GPBSET.bit.GPIO41 = 1; // SPDT On -> Battery
//
//        if (gridCurrent == FLA) {
//            GpioDataRegs.GPBSET.bit.GPIO41 = 0; // SPDT Off -> Grid
//        }
//
//    }
//
//    /* Acknowledging PIE Group 10 to reenable future interrupts */
//    PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;
//
//}

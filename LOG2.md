# hybrid_start

Update Log 2
Week of Jan 31, 2022 - Feb 6, 2022

Ryan Aultman (Storage-to-Output):
I designed the PCB and ordered it, I am waiting for it to come in and as soon as it does, I will 
solder it. I also attempted to fix the connection issue between the inverter and the motor, as 
the motor had one live wire output and the motor had two live wire inputs.

Thomas Bergeron (Digital Interface):
I have designed my PCB for the remote. I have received the last of my parts for it so when it is 
done fabricating I will be ready to sauter. My next step is ordering the PCB, and making sure 
the display screen can run solely off of the 5V GPIO pins.

Janet Park (Controls/Monitoring):
Due to an error with the peripherals and baud rates, I had to completely start over my code for 
the microcontroller’s serial communication protocol with the Raspberry Pi. Additionally, I have 
removed and set up the correct files for implementation of emulation mode. Over the weekend, I was 
able to validate that the DSP deactivates the system whenever a push button (connected to GPIO38) is 
pressed, validating the functionality of the interrupt service routine. I have collected the dimensions 
of the motor, so that I can now finish scaling the design of the 3D printed disk to the motor shaft. I 
plan on completing the SCI protocol and emulation mode setups by the end of this weekend, and printing 
the disk by the middle of next week.

Hunter Ruff (Input-to-Storage):
I have received my PCB and am in the process of soldering and testing this week. This week, I will be 
testing the connections of all my traces for manufacturing defects and soldering. I will then test all 
connections again to make sure that the components are properly connected to the specified nets that 
they go with. The purpose of this is to rule out a lot of diagnostic issues before I test the functional 
circuits. This way I have less things to test if something goes wrong. After this I will begin running 
test benches and verifications.


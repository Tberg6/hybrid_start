# hybrid_start
Update Log 4
Week of Feb 14, 2022 - Feb 20, 2022

Ryan Aultman (Storage-to-Output):
This week I wasn’t able to do much, as I’m waiting for my PCB and perf board to come in so I can start 
soldering the switching and sensing circuits. They should come in next week, and I should be finished 
with both by next Sunday at the latest. I talked to Dr. Lusher about the issue I was having with the 
connection between the inverter and the motor, and he explained to me how this connection would work. 
Also, I downloaded AutoCAD and began work on the chassis that would hold our system. I also helped my 
other group members when I could. 

Thomas Bergeron (Digital Interface):
This week my PCB arrived. I was able to solder it all together, however it does not seem to be working 
so I need to continue testing the connections. I ran my test on the screen and it works with only the 
5V and Ground GPIO pins attached so I can now start integration with the microcontroller. I started 
integrating with Janet's microcontroller and we ran into more problems than expected. We are workshopping 
how to communicate through Serial Peripheral Interface(SPI)  and will likely finish and be fully 
integrated by next week.

Janet Park (Controls/Monitoring):
I was able to complete the setup of the serial peripheral protocol for communication with the Raspberry 
Pi. Additionally, I printed the disk for the motor shaft over the weekend. This week, I will ensure that 
the dimensions of the disk are suitable for mounting onto the motor shaft, and I will epoxy the NdFeB 
magnet onto it in order to implement detection of the motor status. Furthermore, I will begin integration 
with the digital interface subsystem by sending arbitrary integer values to the Raspberry Pi, which should 
then be processed as a string for output onto the LCD display. In terms of being able to integrate with 
the power subsystems, I ran into a few issues with running emulation mode, and will receive help with this 
during this next week. I was able to begin integration with the grid input-to-storage subsystem, but we 
came across a slight issue with the op amp. Therefore, we have decided to begin integration over this next week.

Hunter Ruff (Input-to-Storage):
This week I’ve soldered output pins onto my board, and have been connecting the appropriate wires to 
those pins. I’ve also connected headers to my wire connectors for the power supply for easy connection and 
disconnection. I’ve confirmed that my buck converter and battery charger work but have an issue with my 
3.3V linear regulator. The regulator is not behaving as expected, as it is outputting a higher than 
acceptable voltage and have a ripple of ~1V. This cannot work for supplying the microcontroller so I will 
be developing a solution, most likely another buck converter circuit. 


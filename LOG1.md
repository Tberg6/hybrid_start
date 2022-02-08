# hybrid_start
Update Log 1
Week of Jan 24, 2022 - Jan 30, 2022

Ryan Aultman (Storage-to-Output):
I finished revising my PCB designs from last semester and plan on ordering them within the next few days. 
I ordered the few remaining parts I needed for them (headers, pins, etc.) Also, I received the inverter from 
Dr. Lusher that is to be used for the rest of the project. I plan on testing the Inverter with the battery and 
the motor over the next few weeks as I wait for the PCBs to come in. 

Thomas Bergeron (Digital Interface):
This week I researched how to open up more GPIO pins to include all the sensors. Because the current display 
I have is touch screen compatible, it takes up a lot of gpio pins. I believe I can power the screen by just 
using two 5 V GPIO pins and connecting the screen to HDMI via cable. Still need to test. I edited comments in 
my code after looking at it over breaking and seeing where previous comments may cause confusion. My remote 
control PCB still needs to be designed in Altium. I have drawn an idea of what it needs to look like so 
I should just need to implement it in Altium. I am currently waiting on a license form Lusher

Janet Park (Controls/Monitoring):
Upon discussing I/O connections and a holistic system overview with my team members, I have completed mapping 
GPIO pins for complete inter-subsystem interactions. Additionally, I finished coding the interrupt service 
routines for subsystem integration (including those for emergency deactivation, power source switching, and 
overcurrent protection for start-up cycle). I’m approximately 20% complete with implementing the serial communication 
peripheral interfaces (for establishing communication with the digital interface subsystem), as well as setup for 
emulation mode (for supporting standalone flash memory on the MCU). My plan is to have the SCI and emulation mode 
setups completed, and to begin designing the motor shaft disk over the next two weeks.

Hunter Ruff (Input-to-Storage):
Over the winter break and leading up to this week, I ordered all remaining components for my subsystem and ordered 
my PCB. I made revisions to the PCB to add protection elements and match edits made to other subsystems. I have 
tested a new switching component that I added to my design and have validated its functionality. My PCB is scheduled 
to arrive on Monday, so in the meantime I will be researching issues that I encountered during validation in 403. 
There is a ringing effect in my power supply that I want to find methods of filtering for, and input pin connections 
that need to be coordinated with the power being inputted. Intermittently, I will also be assisting my other team 
members with PCB designs and orders.

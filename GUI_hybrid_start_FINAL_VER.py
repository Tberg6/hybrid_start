from guizero import App,Text,PushButton,ButtonGroup, Picture, Window
from gpiozero import DigitalOutputDevice, DigitalInputDevice, LED, Button
import pyautogui as mouse
import datetime
import spidev
import matplotlib.pyplot as plt
import os
app = App(title="Hybrid Start")
app.set_full_screen()

    ##########################
######Global Variables##################
    #########################
curveDisplay_indicator = 0
emergency_trip = 0
current_DateTime = datetime.datetime(2022,1,1,1)
stopwatch_seconds_onesPlace= 0
stopwatch_milseconds_tensPlace = 0
stopwatch_milseconds_thousadsPlace = 0
current = 0
voltage = 0


#####Existing power curve files#######
reverse_filenames_PowerC = os.listdir("/home/pi/Pictures")
filenames_PowerC = []
print(reverse_filenames_PowerC)
for i in range(0,(len(reverse_filenames_PowerC)-1)):
    filenames_PowerC.append(reverse_filenames_PowerC[0-i])
if (len(filenames_PowerC) ==0):
    filenames_PowerC.append("No Items")

powerCurve_data = [[],[]]


    ###############################
###### Functions #####################################
    #################################
#####Button mapping#################
up_button = Button(19)
down_button = Button(6)
left_button = Button(13)
right_button = Button(26)
select_button = Button(5)
emergency_switch = Button(21)
input_from_other_device = DigitalOutputDevice(20)
input_from_other_device.on()
#####GPIO-->Microcontroller#################

emergency_signal = DigitalOutputDevice(14)
gridPower_signal = DigitalOutputDevice(15)
error_current = DigitalInputDevice(23)
error_voltage= DigitalInputDevice(24)


######Coordinates for cursor movement#############
screenWidth,screenHeight = mouse.size()

start_CRD = (int(0.66*screenWidth),int(0.5*screenHeight))
settings_CRD = (int(0.33*screenWidth),int(0.5*screenHeight))
startPage_exit_CRD=(int(0.33*screenWidth),int(0.5*screenHeight))
settingsPage_exit_CRD=(int(0.1*screenWidth),int(0.5*screenHeight))
settingsPage_dateTime_CRD=(int(0.5*screenWidth),int(0.5*screenHeight))
settingsPage_powerCurve_CRD =( 0.75*screenWidth,0.5*screenHeight)
datetime_0_CRD = (int(0.04*screenWidth)), int(0.55*screenHeight)
datetime_1_CRD = (int(0.12*screenWidth), int(0.55*screenHeight))
datetime_2_CRD = (int(0.21*screenWidth), int(0.55*screenHeight))
datetime_3_CRD = (int(0.30*screenWidth), int(0.55*screenHeight))
datetime_4_CRD = (int(0.39*screenWidth), int(0.55*screenHeight))
datetime_5_CRD = (int(0.48*screenWidth), int(0.55*screenHeight))
datetime_6_CRD = (int(0.57*screenWidth), int(0.55*screenHeight))
datetime_7_CRD = (int(0.65*screenWidth), int(0.55*screenHeight))
datetime_8_CRD = (int(0.74*screenWidth), int(0.55*screenHeight))
datetime_9_CRD = (int(0.83*screenWidth), int(0.55*screenHeight))
datetime_exit_CRD = (int(0.92*screenWidth),int(0.55*screenHeight))
powerCurve_exit_CRD = (int(0.5*screenWidth),int(0.9*screenHeight))
listPowerCurve_exit_CRD = (int(0.5*screenWidth),int(0.9*screenHeight))
listPowerCurve_slot1_CRD = (int(0.5*screenWidth),int(0.1*screenHeight))
listPowerCurve_slot2_CRD = (int(0.5*screenWidth),int(0.15*screenHeight))
listPowerCurve_slot3_CRD = (int(0.5*screenWidth),int(0.2*screenHeight))
listPowerCurve_slot4_CRD = (int(0.5*screenWidth),int(0.25*screenHeight))
listPowerCurve_slot5_CRD = (int(0.5*screenWidth),int(0.3*screenHeight))
listPowerCurve_slot6_CRD = (int(0.5*screenWidth),int(0.35*screenHeight))
listPowerCurve_slot7_CRD = (int(0.5*screenWidth),int(0.4*screenHeight))
listPowerCurve_slot8_CRD = (int(0.5*screenWidth),int(0.45*screenHeight))
listPowerCurve_slot9_CRD = (int(0.5*screenWidth),int(0.5*screenHeight))
listPowerCurve_slot10_CRD = (int(0.5*screenWidth),int(0.55*screenHeight))
displayPowerCurve_exit_CRD = (int(0.95*screenWidth),int(0.5*screenHeight))
refocus_point = (int(0.5*screenWidth),int(0.05*screenHeight))


mouse.moveTo(start_CRD) #starting mouse position

#Activates when a button is pressed or error is sent from MCU
def buttons():
    left_button.when_released = move_left
    right_button.when_released = move_right
    up_button.when_released= move_up
    down_button.when_released = move_down
    select_button.when_released = select
    emergency_switch.when_pressed = emergency_on
    emergency_switch.when_released = emergency_off
    error_current.when_activated = error_options
    error_voltage.when_activated = error_options

#When the Left button is pressed, the following conditions are considered
#before bringing the cursor to the intended position
def move_left():
    print("left button pressed")
    if start_BtN.visible == True: #Main Page
        mouse.moveTo(settings_CRD)
    #Start Page only has one option so no need to inititate
    elif dateChange_BtN.visible == True: #Settings Page
        if(mouse.position() == settingsPage_powerCurve_CRD):
            mouse.moveTo(settingsPage_dateTime_CRD)
        elif (mouse.position()== settingsPage_dateTime_CRD):
            mouse.moveTo(settingsPage_exit_CRD)
    elif dateChange_instructions.visible == True: #Date/Time Page
        if (mouse.position()== datetime_exit_CRD):
            mouse.moveTo(datetime_9_CRD)
        elif (mouse.position() == datetime_9_CRD):
            mouse.moveTo(datetime_8_CRD)
        elif (mouse.position() == datetime_8_CRD):
            mouse.moveTo(datetime_7_CRD)
        elif (mouse.position() == datetime_7_CRD):
            mouse.moveTo(datetime_6_CRD)
        elif (mouse.position() == datetime_6_CRD):
            mouse.moveTo(datetime_5_CRD)
        elif (mouse.position() == datetime_5_CRD):
            mouse.moveTo(datetime_4_CRD)
        elif (mouse.position() == datetime_4_CRD):
            mouse.moveTo(datetime_3_CRD)
        elif (mouse.position() == datetime_3_CRD):
            mouse.moveTo(datetime_2_CRD)
        elif (mouse.position() == datetime_2_CRD):
            mouse.moveTo(datetime_1_CRD)
        elif (mouse.position() == datetime_1_CRD):
            mouse.moveTo(datetime_0_CRD) 
    else: #glitch?
       # main_menu()
       print("oh no")
       
       
#When the Right button is pressed, the following conditions are considered
#before bringing the cursor to the intended position      
def move_right():
    print("right button pressed")
    if start_BtN.visible == True: #Main Page
        mouse.moveTo(start_CRD)
    #Start Page only has one option so no need to inititate
    elif dateChange_BtN.visible == True: #Settings Page
        if(mouse.position() == settingsPage_exit_CRD):
            mouse.moveTo(settingsPage_dateTime_CRD)
        elif(mouse.position() == settingsPage_dateTime_CRD):
            mouse.moveTo(settingsPage_powerCurve_CRD)
    elif dateChange_instructions.visible == True: #Date/Time Page
        print("test1")
        if(mouse.position() == datetime_0_CRD):
            mouse.moveTo(datetime_1_CRD)
            print("test2")
        elif(mouse.position() == datetime_1_CRD):
            mouse.moveTo(datetime_2_CRD)
        elif(mouse.position() == datetime_2_CRD):
            mouse.moveTo(datetime_3_CRD)
        elif(mouse.position()== datetime_3_CRD):
            mouse.moveTo(datetime_4_CRD)
        elif(mouse.position() == datetime_4_CRD):
            mouse.moveTo(datetime_5_CRD)
        elif(mouse.position() == datetime_5_CRD):
            mouse.moveTo(datetime_6_CRD)
        elif(mouse.position() == datetime_6_CRD):
            mouse.moveTo(datetime_7_CRD)
        elif(mouse.position() == datetime_7_CRD):
            mouse.moveTo(datetime_8_CRD)
        elif(mouse.position() == datetime_8_CRD):
            mouse.moveTo(datetime_9_CRD)
        elif(mouse.position() == datetime_9_CRD):
            mouse.moveTo(datetime_exit_CRD)
    else: #glitch?
       # main_menu()
       print("oh no")

#When the Up button is pressed, the following conditions are considered
#before bringing the cursor to the intended position
def move_up():
    print("up button pressed")
    if power_curve_BtnGroup.visible == True:
        if(mouse.position() == listPowerCurve_exit_CRD):
            mouse.moveTo(listPowerCurve_slot10_CRD)   
        elif(mouse.position() == listPowerCurve_slot10_CRD):
            mouse.moveTo(listPowerCurve_slot9_CRD)
        elif(mouse.position() == listPowerCurve_slot9_CRD):
            mouse.moveTo(listPowerCurve_slot8_CRD)
        elif(mouse.position()== listPowerCurve_slot8_CRD):
            mouse.moveTo(listPowerCurve_slot7_CRD)
        elif(mouse.position() == listPowerCurve_slot7_CRD):
            mouse.moveTo(listPowerCurve_slot6_CRD)
        elif(mouse.position() == listPowerCurve_slot6_CRD):
            mouse.moveTo(listPowerCurve_slot5_CRD)
        elif(mouse.position() == listPowerCurve_slot5_CRD):
            mouse.moveTo(listPowerCurve_slot4_CRD)
        elif(mouse.position() == listPowerCurve_slot4_CRD):
            mouse.moveTo(listPowerCurve_slot3_CRD)
        elif(mouse.position() == listPowerCurve_slot3_CRD):
            mouse.moveTo(listPowerCurve_slot2_CRD)
        elif(mouse.position() == listPowerCurve_slot2_CRD):
            mouse.moveTo(listPowerCurve_slot1_CRD)
            
#When the Down button is pressed, the following conditions are considered
#before bringing the cursor to the intended position
def move_down():
    print("down button pressed")
    if power_curve_BtnGroup.visible == True:
        if(mouse.position() == listPowerCurve_slot1_CRD):
            mouse.moveTo(listPowerCurve_slot2_CRD)   
        elif(mouse.position() == listPowerCurve_slot2_CRD):
            mouse.moveTo(listPowerCurve_slot3_CRD)
        elif(mouse.position() == listPowerCurve_slot3_CRD):
            mouse.moveTo(listPowerCurve_slot4_CRD)
        elif(mouse.position()== listPowerCurve_slot4_CRD):
            mouse.moveTo(listPowerCurve_slot5_CRD)
        elif(mouse.position() == listPowerCurve_slot5_CRD):
            mouse.moveTo(listPowerCurve_slot6_CRD)
        elif(mouse.position() == listPowerCurve_slot6_CRD):
            mouse.moveTo(listPowerCurve_slot7_CRD)
        elif(mouse.position() == listPowerCurve_slot7_CRD):
            mouse.moveTo(listPowerCurve_slot8_CRD)
        elif(mouse.position() == listPowerCurve_slot8_CRD):
            mouse.moveTo(listPowerCurve_slot9_CRD)
        elif(mouse.position() == listPowerCurve_slot9_CRD):
            mouse.moveTo(listPowerCurve_slot10_CRD)
        elif(mouse.position() == listPowerCurve_slot10_CRD):
            mouse.moveTo(listPowerCurve_exit_CRD)

#When the Select button is pressed, the following conditions are considered
#before bringing the cursor to the intended position            
def select():
    global curveDisplay_indicator
    print("select button pressed")
    mouse.click()
    print("clicked")
    #setting default position 
    if start_BtN.visible == True: #Main Page
        mouse.moveTo(start_CRD)
    elif stopWatch.visible == True: #Start Page
        mouse.moveTo(settings_CRD)
    elif dateChange_instructions.visible == True: #Date/Time Page
        mouse.moveTo(datetime_0_CRD)
        if(mouse.position()== datetime_exit_CRD):
            mouse.moveTo(start_CRD)
    elif dateChange_BtN.visible == True: #Settings Page
        mouse.moveTo(settingsPage_dateTime_CRD)
    elif(power_curve_BtnGroup.visible == True and curveDisplay_indicator == 0): #Power Curve Page
        mouse.moveTo(listPowerCurve_slot1_CRD)
        print("power curve worked")
        curveDisplay_indicator=1
    elif (curveDisplay_indicator == 1): #display Power Curve Page
        mouse.moveTo(displayPowerCurve_exit_CRD)
        curveDisplay_indicator=0
        print("display power curve worked")
    else: #glitch?
       # main_menu()
       print("oh no")
    return

#When the Emergency switch moves to the On position, the Emergency error screen is appears and the start cycle is stopped
def emergency_on():
    print("switch on")
    global emergency_trip
    emergency_trip = 1
    errorMessage.value = "Emergency switch activated: \n Resolve issue, then turn switch off to reset"
    emergency_signal.on()
    gridPower_signal.off()
    errorWindow.show(wait= True)
    errorMessage.show()

#When the Emergency switch moves to the Off position, this function has the screen return to the main menu
def emergency_off():
    print("switch off")
    emergency_signal.off()
    main_menu()
    mouse.moveTo(refocus_point)
    mouse.click()
    mouse.moveTo(start_CRD)

#When an error signal is recieved from the MCU, this function interprets which type of 
#error it is and then shows the error screen
def error_options():
    print("error on")
    if (error_current.is_active):
        errorMessage.value = "Error: Over Current. Contact maintennce"
    elif (error_voltage.is_active):
        errorMessage.value = "Error: Over Voltage. Contact maintennce"
    emergency_signal.on()
    errorWindow.show(wait= True)
    error_exit_BtN.show()
    errorMessage.show()
    mouse.moveTo(displayPowerCurve_exit_CRD)

#This exit button allows the user to exit the error page back to the main page
def exit_error():
    print("exiting error page")
    emergency_signal.off()
    main_menu()
    mouse.moveTo(refocus_point)
    mouse.click()
    mouse.moveTo(start_CRD)
    ###############################
###########GUI Pages#######################################################################################
    ###############################
#Main Menu page
def main_menu():
    global curveDisplay_indicator, emergency_trip
    exit_BtN.resize("fill","fill")
    settings_BtN.show()
    start_BtN.show()
    start_BtN.enable()
    exit_BtN.hide()
    exit_BtN.enable()
    dateChange_BtN.hide()
    power_curve_BtN.hide()
    stopWatch.hide()
    power_curve_BtnGroup.hide()
    button0.hide()
    button1.hide()
    button2.hide()
    button3.hide()
    button4.hide()
    button5.hide()
    button6.hide()
    button7.hide()
    button8.hide()
    button9.hide()
    keypad_output.hide()
    dateChange_instructions.hide()
    power_curve_picture.hide()
    errorMessage.hide()
    errorWindow.hide()
    error_exit_BtN.hide()
    emergency_trip = 0
    curveDisplay_indicator = 0

#Settings Page
def show_settings():
    settings_BtN.hide()
    start_BtN.hide()
    exit_BtN.show()
    dateChange_BtN.show()
    power_curve_BtN.show()
    print("showing settings")

#Start Page  
def start():
    start_BtN.disable()
    settings_BtN.hide()
    start_BtN.hide()
    stopWatch.show()
    exit_BtN.show()
    exit_BtN.disable() #Can not leave menu during startup 
    #setting stopwatch to 0:00
    global stopwatch_seconds_onesPlace, stopwatch_milseconds_tensPlace, stopwatch_milseconds_thousadsPlace, powerCurve_data
    stopwatch_seconds_onesPlace = 0
    stopwatch_milseconds_tensPlace = 0
    stopwatch_milseconds_thousadsPlace = 0
    #starts stopwatch
    global current, voltage
    current = 0
    voltage = 0
    powerCurve_data[0].clear()
    powerCurve_data[1].clear()
    gridPower_signal.on()
    stopWatch.repeat(1,stopwatch_operation)#change repeat value to 10 for real time
    
    print("starting") #test purposes

#Date Changing settings
def dateChange(): 
    exit_BtN.resize("fill","fill")
    dateChange_BtN.hide()
    dateChange_instructions.show()
    power_curve_BtN.hide()
    dateChange_instructions.show()
    keypad_output.show()
    keypad_output.clear()
    button0.show()
    button1.show()
    button2.show()
    button3.show()
    button4.show()
    button5.show()
    button6.show()
    button7.show()
    button8.show()
    button9.show()
    print("Date Change settings")

    
def power_curve_list():#List of power curve pages
    exit_BtN.show()
    dateChange_BtN.hide()
    power_curve_BtN.hide()
    power_curve_BtnGroup.show()
    print("Power curves are here")

def display_PowerCurve(): #Displays selected graph
    power_curve_picture.value = "/home/pi/Pictures/" + power_curve_BtnGroup.value_text #directory of selected folower
    power_curve_picture.show()
    exit_BtN.resize(4,4)
    power_curve_BtnGroup.hide()
    print("Picture displayed")

    ###############################
##########Computations##########################################################################################################
    ###############################
#This function creates a plot based off of current and voltage data 
def newPowerCurve(): 
    power_curve_BtnGroup.show()
    fileName = dateTime_info.value + ".png" #names file after current time and date
    power_curve_BtnGroup.insert(0, fileName)
    filenames_PowerC.insert(0, fileName)
    if len(filenames_PowerC)>10: #If more than 10 files are saved, remove the oldest file
        power_curve_BtnGroup.remove(filenames_PowerC[10])
        os.remove("/home/pi/Pictures/" + filenames_PowerC[10])
        filenames_PowerC.remove(filenames_PowerC[10])
    power_curve_BtnGroup.hide()
    print(filenames_PowerC)

#This function saves the to the Pictures folder
def save_PowerCurve(data_list,filenames_list):
    plt.plot(data_list[0],data_list[1])
    plt.title(filenames_list[0])
    plt.xlabel("Voltage")
    plt.ylabel("Current")
    plt.savefig("/home/pi/Pictures/" + filenames_list[0])
    plt.clf()
    print("Figured saved")

#This function increments the timer used to control the length of battery start up
def stopwatch_operation(): 
    global stopwatch_seconds_onesPlace, stopwatch_milseconds_tensPlace, stopwatch_milseconds_thousadsPlace, emergency_trip
    global current, voltage, filenames_PowerC, powerCurve_data
    stopwatch_milseconds_thousadsPlace += 1
    if emergency_trip == 1: #If error occurs or emergency switch is set to On
        stopWatch.cancel(stopwatch_operation) #Stops from running in background
        newPowerCurve()
        power_curve_BtnGroup.hide()
        save_PowerCurve(powerCurve_data, filenames_PowerC)
        emergency_trip = 0
        powerCurve_data = [[],[]]
    elif stopwatch_seconds_onesPlace >= 35: #If the time limit has been reached, switch to grid power and stop timer
        gridPower_signal.off()
        stopwatch_seconds_onesPlace = 35
        stopwatch_milseconds_tensPlace = 0
        stopwatch_milseconds_thousadsPlace = 0
        stopWatch.cancel(stopwatch_operation) #Stops from running in background
        exit_BtN.enable()#allows user to leave after (timer completes)/(switch to grid)
        newPowerCurve()
        power_curve_BtnGroup.hide()
        save_PowerCurve(powerCurve_data, filenames_PowerC)
        powerCurve_data = [[],[]]
    else: #Incremeants the timer
        if stopwatch_milseconds_thousadsPlace == 10:
            stopwatch_milseconds_thousadsPlace = 0
            stopwatch_milseconds_tensPlace += 1
            if stopwatch_milseconds_tensPlace == 10:
                stopwatch_milseconds_thousadsPlace = 0
                stopwatch_milseconds_tensPlace = 0
                stopwatch_seconds_onesPlace += 1
                #SPI reciving data- working
                #spi_bus = 0
                #spi_device = 0
                #spi = spidev.SpiDev()
                #spi.open(spi_bus, spi_device)
                #spi.max_speed_hz = 1500000
                #data_rec = spi.readbytes(2)
                
                #for demo purposes
                print(current)
                print(voltage)
                powerCurve_data[0].append(voltage)
                powerCurve_data[1].append(current)
                current += 1
                voltage += 1
    stopWatch.value = str(stopwatch_seconds_onesPlace) + str(":") + str(stopwatch_milseconds_tensPlace) + str(stopwatch_milseconds_thousadsPlace)

#This function allows the user to input a correct date and time, while putting it in the correct formatting
def date_input(number):
    global current_DateTime
    if keypad_output.value == "Incorrect format: Please try again":
        keypad_output.clear()
        keypad_output.append(number)
    elif keypad_output.value == "Time and Date Adjusted":
        keypad_output.clear()
        keypad_output.append(number)
    elif (len(keypad_output.value)==1 or len(keypad_output.value)==4):
        keypad_output.append(number)
        keypad_output.append("-")
    elif len(keypad_output.value)==9:
        keypad_output.append(number)
        keypad_output.append(" ")
    elif len(keypad_output.value)==12:
        keypad_output.append(number)
        keypad_output.append(":")
    else:
        keypad_output.append(number)
        if len(keypad_output.value)==16:
            try:
                current_DateTime = datetime.datetime.strptime(keypad_output.value, "%m-%d-%Y %H:%M")
            except ValueError:
                keypad_output.value = "Incorrect format: Please try again"
                return
            dateTime_info.value = keypad_output.value
            keypad_output.value = "Time and Date Adjusted"

#This function creates a clock that is displyed constatly for the user to see   
def date_clock():
    global current_DateTime 
    current_DateTime += datetime.timedelta(minutes = 1)
    temp = current_DateTime.strftime("%m-%d-%Y %H:%M")
    dateTime_info.value=temp
    print(dateTime_info.value)
            
    ##################################
########Buttons For GUI###################
    ##################################
#error windows 
errorWindow = Window(app,  title = "Error",visible = False)
errorWindow.set_full_screen()
errorMessage = Text(errorWindow, text="Emergency switch activated: \n Resolve issue, then turn switch off",width = "fill",visible = False, size =50, align = "left", color = "red")
error_exit_BtN = PushButton(errorWindow, command= exit_error , text="Exit" , align = "left", width = "fill", height="fill",visible = False)
error_exit_BtN.text_size = 50

#Date/Time
dateTime_info = Text(app,text="01-01-2021 01:00",align = "top",width = "fill",visible = True, size =50)
dateTime_info.repeat(1000,date_clock)#set to 60000 to change every minute

#Power Settings
power_curve_BtN = PushButton(app, command= power_curve_list , text="Power Curves" , align = "right", width = "fill", height="fill",visible = False)
power_curve_BtN.text_size = 50
power_curve_picture = Picture(app,align = "left",visible = False,width = int(screenWidth*.9), height = int(screenHeight*.9))
power_curve_BtnGroup = ButtonGroup(app, options = filenames_PowerC, selected = filenames_PowerC[0], command = display_PowerCurve)
power_curve_BtnGroup.text_size = 30
power_curve_BtnGroup.hide() #BUTTON GROUP DEFAULT VISIBILITY DO NOT CHANGE

#Date Chainging
dateChange_BtN = PushButton(app, command= dateChange , text="Date/Time settings" , align = "right", width = "fill", height="fill",visible = False)
dateChange_BtN.text_size = 50
dateChange_instructions = Text(app,text="Insert new date date in the following \n format using the 24 hour clock: MM-DD-YYYY HH:MM",align = "top",width = "fill",visible = False, size = 30)

#Number pad
keypad_output = Text(app,text="", align = "top" ,visible = False, size = 30)
button0 = PushButton(app, text = "0", align = "left", width = "fill",visible = False,command = date_input,args=["0"])
button1 = PushButton(app, text = "1", align = "left", width = "fill",visible = False,command = date_input,args=["1"])
button2 = PushButton(app, text = "2", align = "left", width = "fill",visible = False,command = date_input,args=["2"])
button3 = PushButton(app, text = "3", align = "left", width = "fill",visible = False,command = date_input,args=["3"])
button4 = PushButton(app, text = "4", align = "left", width = "fill",visible = False,command = date_input,args=["4"])
button5 = PushButton(app, text = "5", align = "left", width = "fill",visible = False,command = date_input,args=["5"])
button6 = PushButton(app, text = "6", align = "left", width = "fill",visible = False,command = date_input,args=["6"])
button7 = PushButton(app, text = "7", align = "left", width = "fill",visible = False,command = date_input,args=["7"])
button8 = PushButton(app, text = "8", align = "left", width = "fill",visible = False,command = date_input,args=["8"])
button9 = PushButton(app, text = "9", align = "left", width = "fill",visible = False,command = date_input,args=["9"])
button0.text_size = 50
button1.text_size = 50
button2.text_size = 50
button3.text_size = 50
button4.text_size = 50
button5.text_size = 50
button6.text_size = 50
button7.text_size = 50
button8.text_size = 50
button9.text_size = 50

   #Start
stopWatch = Text(app,text="0:00",align = "right",width = "fill",visible = False, size = 50)

#main menu + Exit button
settings_BtN = PushButton(app, command= show_settings, text="Power Settings" , align="left", width = "fill", height="fill",visible = True)
settings_BtN.text_size = 50
start_BtN = PushButton(app, command= start, text="Start" , align = "right", width = "fill", height="fill",visible = True)
start_BtN.text_size = 50
exit_BtN = PushButton(app, command= main_menu , text="Exit" , align = "left", width = "fill", height="fill",visible = False)
exit_BtN.text_size = 50
exit_BtN.repeat(10, buttons)

app.display()


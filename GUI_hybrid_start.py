from guizero import App,Text,PushButton,ButtonGroup, Picture, Window
from gpiozero import Button
import pyautogui as mouse
import datetime
import matplotlib.pyplot as plt
import os


app = App(title="Hybrid Start") #intializes GUI
app.set_full_screen() #sets GUI window to full screen

    ##########################
######Global Variables##################
    #########################
curveDisplay_indicator = 0 #Needed to distinguish between list of Power curve page and display power curve page for select() function
emergency_trip = 0 #Needed for ending all processes when emergency switch is switch On.
current_DateTime = datetime.datetime(2021,1,1,1) #default date and time
stopwatch_seconds_onesPlace= 0 #Needed for stopwatch_operation to be repeatedly called
stopwatch_milseconds_tensPlace = 0 #Needed for stopwatch_operation to be repeatedly called
stopwatch_milseconds_thousadsPlace = 0 #Needed for stopwatch_operation to be repeatedly called
current = 0 #instantaneous current values from sensors
voltage = 0 #instantaneous voltage values from sensors

#function below check files in Graph directory and puts them in list
reverse_filenames_PowerC = os.listdir("/home/pi/Pictures")
filenames_PowerC = []
print(reverse_filenames_PowerC)
for i in range(0,(len(reverse_filenames_PowerC)-1)):
    filenames_PowerC.append(reverse_filenames_PowerC[0-i])
if (len(filenames_PowerC) ==0):
    filenames_PowerC.append("No Items")

powerCurve_data = [[],[]] #Graph data storage

    ###############################
###### Functions #####################################
    #################################
    
#####GPIO/Button mapping####################################################

up_button = Button(5)#sets GPIO5 Pin to up_button
down_button = Button(6)#sets GPIO6 Pin to down_button
left_button = Button(13)#sets GPIO13 Pin to left_button
right_button = Button(19)#sets GPIO19 Pin to right_button
select_button = Button(26)#sets GPIO26 Pin to select_button
emergency_switch = Button(21)#sets GPIO21 Pin to emergency_switch

screenWidth,screenHeight = mouse.size()#gets pixel dimensions of screen

#The list of variables below are assigned to coordinates so that the mouse can find each button when needed 
start_CRD = (int(0.66*screenWidth),int(0.5*screenHeight))
settings_CRD = (int(0.33*screenWidth),int(0.5*screenHeight))
startPage_exit_CRD=(int(0.33*screenWidth),int(0.5*screenHeight))
settingsPage_exit_CRD=(int(0.1*screenWidth),int(0.5*screenHeight))
settingsPage_dateTime_CRD=(int(0.5*screenWidth),int(0.5*screenHeight))
settingsPage_powerCurve_CRD =( 0.75*screenWidth,0.5*screenHeight)
datetime_0_CRD = (int(0.05*screenWidth)), int(0.55*screenHeight)
datetime_1_CRD = (int(0.14*screenWidth), int(0.55*screenHeight))
datetime_2_CRD = (int(0.23*screenWidth), int(0.55*screenHeight))
datetime_3_CRD = (int(0.32*screenWidth), int(0.55*screenHeight))
datetime_4_CRD = (int(0.41*screenWidth), int(0.55*screenHeight))
datetime_5_CRD = (int(0.50*screenWidth), int(0.55*screenHeight))
datetime_6_CRD = (int(0.59*screenWidth), int(0.55*screenHeight))
datetime_7_CRD = (int(0.68*screenWidth), int(0.55*screenHeight))
datetime_8_CRD = (int(0.77*screenWidth), int(0.55*screenHeight))
datetime_9_CRD = (int(0.86*screenWidth), int(0.55*screenHeight))
datetime_exit_CRD = (int(0.95*screenWidth),int(0.55*screenHeight))
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
refocus_point = (int(0.5*screenWidth),int(0.05*screenHeight))#When emergency switch is switched off, the raspberry pi taskbar emerges. This gives a position to click on to refocus on the app
mouse.moveTo(start_CRD) #starting mouse position

def button_move(): #This function detects when a physical tactical button/switch is pressed
    left_button.when_released = move_left
    right_button.when_released = move_right
    up_button.when_released= move_up
    down_button.when_released = move_down
    select_button.when_released = select
    emergency_switch.when_pressed = emergency_on
    emergency_switch.when_released = emergency_off
    
    
#When the left button is pressed, the move_left() function figures out what page the user is currently on and picks the appropriate coordinates set to move to based on its current location.
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


#When the right button is pressed, the move_right() function figures out what page the user is currently on and picks the appropriate coordinates set to move to based on its current location.       
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

#When the up button is pressed, the move_up() function figures out what page the user is currently on and picks the appropriate coordinates set to move to based on its current location.
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

#When the down button is pressed, the move_down() function figures out what page the user is currently on and picks the appropriate coordinates set to move to based on its current location.        
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

#The select() function is trigered when the select button is pressed. The current page is determined, the mouse is clicked, and the appropriate default coordinates are picked for the next page.            
def select():
    global curveDisplay_indicator #Needed to distinguish between list of Power curve page and display power curve page
    print("select button pressed")
    mouse.click()
    #setting default position 
    if start_BtN.visible == True: #Main Page
        mouse.moveTo(start_CRD)
    elif stopWatch.visible == True: #Start Page
        mouse.moveTo(settings_CRD)
    elif dateChange_BtN.visible == True: #Settings Page
        mouse.moveTo(settingsPage_dateTime_CRD)
    elif dateChange_instructions.visible == True: #Date/Time Page
        if(mouse.position() == settingsPage_dateTime_CRD):
            mouse.moveTo(datetime_0_CRD)
        elif(mouse.position()== datetime_exit_CRD):
            mouse.moveTo(start_CRD)
    elif (power_curve_BtnGroup.visible == True and curveDisplay_indicator == 0): #Power Curve Page
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


#The emergency_on() function is triggered when the Emergency switch is moved to the On position. All calculations/processes are stopped and the Emergency Window and Message are shown
def emergency_on():
    print("switch on")
    global emergency_trip
    emergency_trip = 1
    emergencyWindow.show(wait= True)
    emergencyMessage.show()
    
#The emergency_off() function is triggered when the Emergency switch is moved to the off position. The main menu appears, automatically making the Emergency Window and Message hidden. The mouse clicks on the refocus coordinates to make the raspberry pi task bar disappear, then the mouse returns too the main menu defualt coordinates.
def emergency_off():
    print("switch off")
    main_menu()
    mouse.moveTo(refocus_point)
    mouse.click()
    mouse.moveTo(start_CRD)

#####GUI Pages#############################
#The main_menu() function opens the main menu page. It hides all widgets except start_BtN,settings_BtN, and dateTime_info(displays Date and Time)
def main_menu():
    global curveDisplay_indicator
    exit_BtN.resize("fill","fill")
    settings_BtN.show()
    start_BtN.show()
    exit_BtN.hide()
    dateChange_BtN.hide()
    battery_charge.hide() 
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
    emergencyMessage.hide()
    emergencyWindow.hide()
    curveDisplay_indicator = 0 #resets indicator. Needed to distinguish between list of Power curve page and display power curve page for select() function

#The show_settings() function opens the Settings page. It hides all widgets except exit_BtN, battery_charge,power_curve_BtN, and dateTime_info(displays Date and Time)
def show_settings():
    settings_BtN.hide()
    start_BtN.hide()
    exit_BtN.show()
    dateChange_BtN.show()
    battery_charge.show() 
    power_curve_BtN.show()
    print("showing settings")
    
#The start() function opens the Start page. It hides all widgets except exit_BtN, stopWatch,and dateTime_info(displays Date and Time).It resets the stopwatch, current, voltage, and plot values to default values
def start():
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
    stopWatch.repeat(1,stopwatch_operation)#change repeat value 10ms for real time, 1ms for test
    print("starting") #test purposes

#The dateChange() function opens the Date/Time settings page. It hides all widgets except exit_BtN, dateChange_instructions,keypad_output, all keypad buttons, and dateTime_info(displays Date and Time)
def dateChange():
    dateChange_BtN.hide()
    battery_charge.hide() 
    power_curve_BtN.hide()
    dateChange_instructions.show()
    keypad_output.show()
    keypad_output.clear() #clears any previous values that may be left over from previous visits to the page
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
    print("Date Change settings") #for testing

#Once integrated, the battery_charge function will collect the battry charge informaiton from the microcontroller
def battery_charge(battery_charge_from_MC):
    print("Battery Charge =",battery_charge_from_MC, "%")

#The power_curve_list() function opens the List of power curves page. It hides all widgets except exit_BtN, power_curve_BtnGroup and dateTime_info(displays Date and Time)
def power_curve_list():
    exit_BtN.show()
    dateChange_BtN.hide()
    battery_charge.hide() 
    power_curve_BtN.hide()
    power_curve_BtnGroup.show()
    print("Power curves are here") #for testing

#The display_PowerCurve() function finds the file that aligns with the selected file name and displays it. It also resizes the exit button
def display_PowerCurve():
    power_curve_picture.value = "/home/pi/Pictures/" + power_curve_BtnGroup.value_text
    power_curve_picture.show()
    exit_BtN.resize(4,4)
    power_curve_BtnGroup.hide()
    print("Picture displayed") #for testing

#######Computations##########################################################
#The newPowerCurve() function saves the name of the new graph to the list of power curves while also limiting the amount of plot entrys to 10
def newPowerCurve():
    power_curve_BtnGroup.show()
    fileName = dateTime_info.value + ".png"
    power_curve_BtnGroup.insert(0, fileName)
    filenames_PowerC.insert(0, fileName)
    if len(filenames_PowerC)>10:
        power_curve_BtnGroup.remove(filenames_PowerC[10])
        os.remove("/home/pi/Pictures/" + filenames_PowerC[10])
        filenames_PowerC.remove(filenames_PowerC[10])
    power_curve_BtnGroup.hide()
    print(filenames_PowerC) #for testing

#The save_PowerCurve function takes 2 argumments, a 2D list that isused to plot a graph, and list of power curve names including the new plot that the data point apply to. This function uses Matplotlib.pyplot to plot the 2D list, and save it to the Raspberry Pi with the first entry from the list of power curve names
def save_PowerCurve(data_list,filenames_list):
    plt.plot(data_list[0],data_list[1])
    plt.title(filenames_list[0])
    plt.xlabel("Voltage")
    plt.ylabel("Current")
    plt.savefig("/home/pi/Pictures/" + filenames_list[0])
    plt.clf()
    print("Figured saved") #for testing

#The stopwatch_operation increments the Stopwatch Text by 10 ms per run. It stops the increments when 35 seconds have elapsed
def stopwatch_operation(): 
    global stopwatch_seconds_onesPlace, stopwatch_milseconds_tensPlace, stopwatch_milseconds_thousadsPlace, emergency_trip
    global current, voltage, filenames_PowerC, powerCurve_data
    stopwatch_milseconds_thousadsPlace += 1
    if emergency_trip == 1:
        stopWatch.cancel(stopwatch_operation) #Stops from running in background
        exit_BtN.enable()#allows user to leave after (timer completes)/(switch to grid)
        newPowerCurve()
        save_PowerCurve(powerCurve_data, filenames_PowerC)
        emergency_trip = 0
        powerCurve_data = [[],[]]
    elif stopwatch_seconds_onesPlace >= 35:
        stopwatch_seconds_onesPlace = 35
        stopwatch_milseconds_tensPlace = 0
        stopwatch_milseconds_thousadsPlace = 0
        stopWatch.cancel(stopwatch_operation) #Stops from running in background
        exit_BtN.enable()#allows user to leave after (timer completes)/(switch to grid)
        newPowerCurve()
        save_PowerCurve(powerCurve_data, filenames_PowerC)
        powerCurve_data = [[],[]]
    else:
        if stopwatch_milseconds_thousadsPlace == 10:
            stopwatch_milseconds_thousadsPlace = 0
            stopwatch_milseconds_tensPlace += 1
            if stopwatch_milseconds_tensPlace == 10:
                stopwatch_milseconds_thousadsPlace = 0
                stopwatch_milseconds_tensPlace = 0
                stopwatch_seconds_onesPlace += 1
                #get sensor data for current
                #get sendor data for voltage
                current += 1 #for demo purposes
                voltage += 1 #for demo purposes
                powerCurve_data[0].append(voltage) #for demo purposes
                powerCurve_data[1].append(current) #for demo purposes
    stopWatch.value = str(stopwatch_seconds_onesPlace) + str(":") + str(stopwatch_milseconds_tensPlace) + str(stopwatch_milseconds_thousadsPlace)

#The date_input() inputs a number from the Date keypad one at a time. It automaically includes any symbols needed to display time/date
def date_input(number):
    global current_DateTime
    if keypad_output.value == "Incorrect format: Please try again": #clears keypad output if occupied
        keypad_output.clear()
        keypad_output.append(number)
    elif keypad_output.value == "Time and Date Adjusted": #clears keypad output if occupied
        keypad_output.clear()
        keypad_output.append(number)
    elif (len(keypad_output.value)==1 or len(keypad_output.value)==4): #automatically puts "-" for date formatting.
        keypad_output.append(number)
        keypad_output.append("-")
    elif len(keypad_output.value)==9: #automatically puts " " between date and time.
        keypad_output.append(number)
        keypad_output.append(" ")
    elif len(keypad_output.value)==12: #automatically puts ":" for time formatting.
        keypad_output.append(number)
        keypad_output.append(":")
    else:
        keypad_output.append(number)
        if len(keypad_output.value)==16:
            try: #make sure that inputted date exist and is in valid format
                current_DateTime = datetime.datetime.strptime(keypad_output.value, "%m-%d-%Y %H:%M")
            except ValueError: #if not valid format
                keypad_output.value = "Incorrect format: Please try again"
                return
            dateTime_info.value = keypad_output.value #adjust values if correct
            keypad_output.value = "Time and Date Adjusted" # let user know it worked even though they can see the clock
 
#The date_clock() function increments the clock(dateTime_info)
def date_clock():
    global current_DateTime 
    current_DateTime += datetime.timedelta(minutes = 1)
    temp = current_DateTime.strftime("%m-%d-%Y %H:%M")
    dateTime_info.value=temp
    print(dateTime_info.value) #for testing
            
    ##################################
########Buttons For GUI###################
    ##################################
    #Emergency Window
emergencyWindow = Window(app,  title = "Error",visible = False)#initiates Emrgency window
emergencyWindow.set_full_screen() #sets Emergency Window to full screen
emergencyMessage = Text(emergencyWindow, text="Emergency switch activated: \n Resolve issue, then turn switch off",width = "fill",visible = False, size =50, align = "left", color = "red") #initiates Emrgency message that is displayed on Emrgency window

dateTime_info = Text(app,text="01-01-2021 01:00",align = "top",width = "fill",visible = True, size =50) #initiates Date and Time textbox
dateTime_info.repeat(1000,date_clock)#increments Date and Time every minute. Set to 60000ms for real time, 1000ms for test

    #Power Settings Page
battery_charge = Text(app,text="Battery Chage = 55%",align = "top",width = "fill",visible = False, size =30) #Initiates battery charge text box
power_curve_BtN = PushButton(app, command= power_curve_list , text="Power Curves" , align = "right", width = "fill", height="fill",visible = False) #Intitates powerve button and leads to list of Power curves
power_curve_BtN.text_size = 50 # increses text size of power_curve_BtN
power_curve_picture = Picture(app,align = "left",visible = False,width = int(screenWidth*.9), height = int(screenHeight*.9)) #Intitates picture for graph display
power_curve_BtnGroup = ButtonGroup(app, options = filenames_PowerC, selected = filenames_PowerC[0], command = display_PowerCurve) #Intiates for listing the names of power curves and linked to the files
power_curve_BtnGroup.text_size = 30 # increases text size of power_curve_BtnGroup
power_curve_BtnGroup.hide() #BUTTON GROUP DEFAULT VISIBILITY DO NOT CHANGE
dateChange_BtN = PushButton(app, command= dateChange , text="Date/Time settings" , align = "right", width = "fill", height="fill",visible = False)#Intiates Date/Time settings that links to the Number pad page
dateChange_BtN.text_size = 50
dateChange_instructions = Text(app,text="Insert new date date in the following \n format using the 24 hour clock: MM-DD-YYYY HH:MM",align = "top",width = "fill",visible = False, size = 30)

    #Number pad Page
    #Initiates Buttons 0-9 and changes text size 
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

    #Start Page
stopWatch = Text(app,text="0:00",align = "right",width = "fill",visible = False, size = 50) #Intiates Stop Watch
    #Main menu + Exit button
settings_BtN = PushButton(app, command= show_settings, text="Power Settings" , align="left", width = "fill", height="fill",visible = True) #Initiates Settings button that leads to the Power Settings Page
start_BtN = PushButton(app, command= start, text="Start" , align = "right", width = "fill", height="fill",visible = True) #Initiates Start button that leads to Start Page
exit_BtN = PushButton(app, command= main_menu , text="Exit" , align = "left", width = "fill", height="fill",visible = False) #Initiates Exit button that leads back to Main Menu
exit_BtN.repeat(10, button_move) #Could be placed under any button but exit seemed like a good one. Listens for movement from physical buttons

    #Changes text size for all Start Page Widgets
start_BtN.text_size = 50
settings_BtN.text_size = 50
exit_BtN.text_size = 50

app.display()





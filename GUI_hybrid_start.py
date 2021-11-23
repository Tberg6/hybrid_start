from guizero import App,Text,PushButton,ButtonGroup, Picture
from gpiozero import Button
import pyautogui as mouse
import datetime
import matplotlib.pyplot as plt
from os import listdir
app = App(title="Hybrid Start")
app.set_full_screen()


    ##########################
######Global Variables##################
    #########################
current_DateTime = datetime.datetime(2021,1,1,1)
stopwatch_seconds_onesPlace= 0
stopwatch_milseconds_tensPlace = 0
stopwatch_milseconds_thousadsPlace = 0
current = 0
voltage = 0

filenames_PowerC = ["No Items"]
powerCurve_data = [[],[]]

#GPIO.setup(14,GPIO.IN)
#battery_charge_from_MC = GPIO.input(14)


    ###############################
###### Functions #####################################
    #################################
#####GPIO/Button mapping#################
up_button = Button("GPIO5")
down_button = Button("GPIO6")
left_button = Button("GPIO13")
right_button = Button("GPIO19")
select_button = Button("GPIO26")

screenWidth,screenHeight = mouse.size()
#currentMouseX, currentMouseY = mouse.position()
mouse.move(0.66*screenWidth,0.4*screenHeight)
#Coordinates
#start:0.66*screenWidth,0.4*screenHeight
#settings: 0.33*screenWidth,0.4*screenHeight
#start/exit: 0.33*screenWidth,0.4*screenHeight
#settings/exit: 0.1*screenWidth,0.4*screenHeight
#settigns/dateTime: 0.4*screenWidth,0.4*screenHeight
#settigns/powerCurve: 0.7*screenWidth,0.4*screenHeight

def button_move():
    if left_button:
        #move_left()
        return
def move_left():
    global screenWidth,screenHeight
    if start_BtN.visible == True: #Main Page
        mouse.move(0.33*screenWidth,0.4*screenHeight)
        #print("main worked")
        return
    #Start Page only has one option so no need to inititate
    elif dateChange_BtN.visible == True: #Settings Page
        if (mouse.position[0] == 0.4*screenWidth):
            mouse.move(0.1*screenWidth,0.4*screenHeight)
        elif (mouse.position[0] == 0.7*screenWidth):
            mouse.move(0.4*screenWidth,0.4*screenHeight)
        #print("settings worked")
        return
    elif dateChange_instructions.visible == True: #Date/Time Page
        #print("date worked")
        return
    elif power_curve_BtnGroup.visible == True: #Power Curve Page
        #print("power curve worked")
        return
    else: #glitch?
       # main_menu()
       print("oh no")
       return
        

#####GUI Pages#############################
def main_menu():
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

def show_settings():
    settings_BtN.hide()
    start_BtN.hide()
    exit_BtN.show()
    dateChange_BtN.show()
    battery_charge.show() 
    power_curve_BtN.show()
    print("showing settings")
    
def start():
    settings_BtN.hide()
    start_BtN.hide()
    stopWatch.show()
    exit_BtN.show()
    exit_BtN.disable() #Can not leave menu during startup 
    #setting stopwatch to 0:00
    global stopwatch_seconds_onesPlace, stopwatch_milseconds_tensPlace, stopwatch_milseconds_thousadsPlace
    stopwatch_seconds_onesPlace = 0
    stopwatch_milseconds_tensPlace = 0
    stopwatch_milseconds_thousadsPlace = 0
    #starts stopwatch
    stopWatch.repeat(1,stopwatch_operation)#change repeat value 10ms for real time
    print("starting") #test purposes

def dateChange():
    dateChange_BtN.hide()
    battery_charge.hide() 
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

def battery_charge(battery_charge_from_MC):
    print("Battery Charge =",battery_charge_from_MC, "%")
    
def power_curve_list():
    exit_BtN.show()
    dateChange_BtN.hide()
    battery_charge.hide() 
    power_curve_BtN.hide()
    power_curve_BtnGroup.show()
    print("Power curves are here")

def display_PowerCurve():
    power_curve_picture.value = "/home/pi/Pictures/" + power_curve_BtnGroup.value_text
    power_curve_picture.show()
    exit_BtN.resize(2,2)
    power_curve_BtnGroup.hide()
    print("Picture displayed")

#######Computations###############################################3
def newPowerCurve(list_of_names):
    fileName = dateTime_info.value
    power_curve_BtnGroup.insert(0, fileName)
    list_of_names.insert(0, fileName)
    if list_of_names[1] == "No Items":
        power_curve_BtnGroup.remove(list_of_names[1])
        list_of_names.remove("No Items")
    if len(list_of_names)>10:
        power_curve_BtnGroup.remove(list_of_names[10])
        list_of_names.remove(10)
    print(list_of_names)
    
def save_PowerCurve(data_list,filenames_list):
    plt.plot(data_list[0],data_list[1], "x","y")
    plt.savefig("/home/pi/Pictures/" + filenames_list[0])
    print("Figured saved")
    
def stopwatch_operation(): 
    trip = 1
    global stopwatch_seconds_onesPlace, stopwatch_milseconds_tensPlace, stopwatch_milseconds_thousadsPlace
    global current, voltage, filenames_PowerC
    stopwatch_milseconds_thousadsPlace += 1
    if stopwatch_seconds_onesPlace >= 35:
        stopwatch_seconds_onesPlace = 35
        stopwatch_milseconds_tensPlace = 0
        stopwatch_milseconds_thousadsPlace = 0
        if trip == 1:
            stopWatch.cancel(stopwatch_operation) #Stops from running in background
            exit_BtN.enable()#allows user to leave after (timer completes)/(switch to grid)
            newPowerCurve(filenames_PowerC)
            save_PowerCurve(powerCurve_data, filenames_PowerC)
            trip = 0
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
                    #for demo purposes
                current += 1
                voltage += 1
                powerCurve_data[0].append(voltage)
                powerCurve_data[1].append(current)
    stopWatch.value = str(stopwatch_seconds_onesPlace) + str(":") + str(stopwatch_milseconds_tensPlace) + str(stopwatch_milseconds_thousadsPlace)

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
   
def date_clock():
    global current_DateTime 
    current_DateTime += datetime.timedelta(minutes = 1)
    temp = current_DateTime.strftime("%m-%d-%Y %H:%M")
    dateTime_info.value=temp#str(current_DateTime.month)+"-"+str(current_DateTime.day)+"-"+str(current_DateTime.year)+" "+str(current_DateTime.time)#+":"+str(current_DateTime.minute)
    print(dateTime_info.value)
            
    ##################################
########Buttons For GUI###################
    ##################################


dateTime_info = Text(app,text="01-01-2021 01:00",align = "top",width = "fill",visible = True)
dateTime_info.repeat(1000,date_clock)#set to 60000 to change every minute

    #Power Settings
battery_charge = Text(app,text="Battery Chage = 55%",align = "top",width = "fill",visible = False)
power_curve_BtN = PushButton(app, command= power_curve_list , text="Power Curves" , align = "right", width = "fill", height="fill",visible = False)
power_curve_picture = Picture(app,align = "left",visible = False)
power_curve_BtnGroup = ButtonGroup(app, options = filenames_PowerC, selected = filenames_PowerC[0], command = display_PowerCurve)
power_curve_BtnGroup.hide() #BUTTON GROUP DEFAULT VISIBILITY DO NOT CHANGE
        #Date Chainging
dateChange_BtN = PushButton(app, command= dateChange , text="Time/Date settings" , align = "right", width = "fill", height="fill",visible = False)
dateChange_instructions = Text(app,text="Insert new date date in the following \n format using the 24 hour clock: MM-DD-YYYY HH:MM",align = "top",width = "fill",visible = False)
            #keypad
keypad_output = Text(app,text="", align = "top" ,visible = False)
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
   #Start
stopWatch = Text(app,text="0:00",align = "right",width = "fill",visible = False)
    #main menu + Exit button
settings_BtN = PushButton(app, command= show_settings, text="Power Settings" , align="left", width = "fill", height="fill",visible = True)
start_BtN = PushButton(app, command= start, text="Start" , align = "right", width = "fill", height="fill",visible = True)
exit_BtN = PushButton(app, command= main_menu , text="Exit" , align = "left", width = "fill", height="fill",visible = False)
exit_BtN.repeat(1, button_move)




app.display()





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

filenames_PowerC = ["No Items"] #listdir("/home/pi/Pictures")



powerCurve_data = [[],[]]
#if (len(filenames_PowerC) ==0):
   # filenames_PowerC.append("No Items")
#else:
  #  for x in filenames_PowerC:
      #  filenames_PowerC.append(x)

    ###############################
###### Functions #####################################
    #################################
#####GPIO/Button mapping#################
up_button = Button(5)
down_button = Button(6)
left_button = Button(13)
right_button = Button(19)
select_button = Button(26)

#CRD = Coordinates
screenWidth,screenHeight = mouse.size()

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
displayPowerCurve_exit_CRD = (int(0.95*screenWidth),int(0.5*screenHeight))
mouse.moveTo(start_CRD) #starting mouse position
def button_move():
    left_button.when_released = move_left
    right_button.when_released = move_right
    #up_button.when_released= move_up
    #down_button.when_released = move_down
    select_button.when_released = select

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
    elif power_curve_BtnGroup.visible == True: #Power Curve Page
        print("power curve worked")  
    else: #glitch?
       # main_menu()
       print("oh no")
       
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
    elif power_curve_BtnGroup.visible == True: #Power Curve Page
        print("power curve worked")  
    else: #glitch?
       # main_menu()
       print("oh no")

def move_up():

    print("up button pressed")

def move_down():
    print("down button pressed")

def select():
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
    elif power_curve_BtnGroup.visible == True: #Power Curve Page
        print("power curve worked")  
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
power_curve_BtN.text_size = 50
power_curve_picture = Picture(app,align = "left",visible = False)
power_curve_BtnGroup = ButtonGroup(app, options = filenames_PowerC, selected = filenames_PowerC[0], command = display_PowerCurve)
power_curve_BtnGroup.hide() #BUTTON GROUP DEFAULT VISIBILITY DO NOT CHANGE
        #Date Chainging
dateChange_BtN = PushButton(app, command= dateChange , text="Date/Time settings" , align = "right", width = "fill", height="fill",visible = False)
dateChange_BtN.text_size = 50
dateChange_instructions = Text(app,text="Insert new date date in the following \n format using the 24 hour clock: MM-DD-YYYY HH:MM",align = "top",width = "fill",visible = False)
            #Number pad
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
exit_BtN.repeat(7, button_move)




app.display()





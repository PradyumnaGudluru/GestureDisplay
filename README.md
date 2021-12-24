# Gesture Controlled Display
Gesture Controlled Display using Bluetooth protocol on EFR32BG13 Blue Gecko.

## Table of Contents
* [Description](#description)
* [Author](#author)

## Description
For the Gesture controlled display, consist of a Gecko Client consisting of the on-board LCD module to be used as the display and a Gecko Server which will consist of a gesture sensor and an ambient light sensor. The aim of this project is to develop a prototype of a display which is controlled remotely using gestures only. The application of this project would be in areas such as malls, where a user can pan and scroll through the display to discover shops or services, or even in an automotive display where a driver or passenger can scroll through the display without touching it. It can also be used in applications such as an e-reader where the user can flip the pages of a book without touch the display.

For the purpose of this project, the Client will simulate an e-book which contains a block of data which can be read either by scrolling up and down on the display or by gesturing left and right to flip through the pages.

The Server will receive the gesture inputs from the user through an I2C gesture sensor and indicate this data to the Client. The client will then scroll the display in the direction in which the user requested. An analog ambient light sensor will be used to detect the lighting conditions and put the Server into a low power state when it is dark, simulating a scenario where the devices are not used.

Two new GATT Services will be created for each of the sensors. The I2C gesture sensor data will be sent to the client as soon as it is received such that the device appears responsive. Similarly, the ambient light sensor will be interfaced to the ACMP module and set to trigger the server when the light falls or rises above a set threshold. The Client will act on the inputs received and update the display accordingly.

#### Design Information:

Refer to the documentation at :
https://github.com/PradyumnaGudluru/GestureDisplay/blob/main/GESTURE%20CONTROLLED%20DISPLAY%20-%20Team%205%20-%20Final%20Report.docx

#### Video Reference:

https://drive.google.com/file/d/1uaxcN_KGLxolG5rK9rgUk3HgjuWyeTd8/view?usp=sharing

#### GitHub URL
URL: https://github.com/PradyumnaGudluru/GestureDisplay
  
 ## Author
 * Pradyumna Gudluru

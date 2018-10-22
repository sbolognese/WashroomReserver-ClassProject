# WashroomReserver-ClassProject
Class project developed by a group of 3 students

Originally hosted on university-owned gitlab server (Contributions made by 3 separate students including myself)

Problem Description:
=================================
You are in class or at your cubicle and you really need to "go". You quickly gather your things and head to the 
bathroom only to find that no stalls are available.

The washroom reserver is a bathroom monitoring device that prevents this problem from occuring. Using a simple
web application and monitoring sensors, you can find out whether stalls are available before you leave your desk,
or before you even enter the bathroom by looking at the number of open stalls on the beaglebone number display.
Reserve a stall for yourself on the web application to ensure you have a place to "go".

Notice that a stall seems in bad shape? Report the stall as needing maintenance on the website. This not only
alerts administrators, but also sets the stall as unavailable, so no one else has to walk in and see that mess!
Once the administrators handle the situation, the stall will return to its available status.


Expected Hardware Components:
=================================
- A beaglebone (black or green): directional buttons, two digit LED display
- 3 magnetic door sensors
- 3 tri-colour LEDs


System Overview:
=================================
OUTSIDE THE BATHROOM:

The beaglebone keeps track of the states of the stalls, acting as the "bathroom monitor"
- LED number display updates when information is recieved from web application OR doors are opened/closed
- C program on the beaglebone generates a unique password for admin usage on the website on startup
- (UNFINISHED) system can support 2 bathrooms at once using 2 beaglebones


THE STALLS:

Stall statuses are set on tri-colour LEDs placed on the outside of the stall door
- RED: stall is in use or is requiring service (unusable, unreservable)
- GREEN: Stall is open, can be reserved or used in person
- BLUE: Stall has been reserved, someone will be arriving to use the stall shortly

Stall statuses are affected by
- opening and closing the stall doors (connecting and disconnecting magnetic sensors)
-> Green for disconnected, red for connected
- stalls being set to "needs maintenance" through web application (red)
- stall reserved by web application (blue)


THE WEB APPLICATION:

Information displayed on the application:
- number of open stalls
- number of total stalls
- information updated periodically

Actions users can take on the application:
- reserve a stall (sends information to C program to change LED colours, updates stall statuses)
- request service on a stall (users select the stall number that needs service)
- ADMIN: admin user can log into admin mode using the unique password printed to console at C program startup
- ADMIN: admin user can see a list of stalls needing service
- ADMIN: admin user can set stalls as open again after being maintenanced
- ADMIN: admin user can freely switch back to regular user mode

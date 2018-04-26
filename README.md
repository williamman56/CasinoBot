# CasinoBot
The code and 3D designs I created for the final project of my Engineering 1 Lab. My project was ultimately voted the favorite of everyone in attendance. 

There are two attached images: One is an image of the final project as a reference for what it looks like, and the second is the schematic for the design. 

There are two main pieces of this bot out of the norm to be aware of, one physical and one software. The first is that in order to accomadate the large number of LEDs on this project, all LEDs must be first connected in a Charlieplexed array before being connected to the pins of the Arduino. For this I used a PCB with male header pins soldered on to simplify the circuitry. The next thing to be aware of is that the software will write directly to the memory of the Arduino -- this is done in a background process that controls multiple LEDs at a time, and is not done with higher-level commands in order to have the write execute fast enough so as not to interfere with other background processes such as the servos. 

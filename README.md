This project is a WIP DND 5E "Pocket" character sheet. 
This project has been built and run using the PlatformIO plugin in VSCode and has used wokwi to simulate the hardware, which can be found at https://wokwi.com/.

In order to simulate the hardware on wokwi do the following:
-go to wowoki.com and under "Simulate with Wokwi Online", select an empty ESP32 project by clicking on the ESP32 box, and then scrolling down to the starter templates and clicking ESP32.
- copy+paste everything from diagram.json in this project to the diagram.json file in wokwi.
- in the Library Manager on wokwi, press the + button and search "hd44780". Once found click on it to add the library.
- copy+paste everything in /src/main.cpp in this project to the sketch.ino file in wokwi. 

You can also use the wokwi plugin in vscode to simulate the hardware. Instructions to do so can be found here: https://docs.wokwi.com/vscode/getting-started

Controls: 
Scroll Up = Green, Scroll Down = Black, Scroll Left = Blue, Scroll Right = Yellow
Select = White, Back = Grey

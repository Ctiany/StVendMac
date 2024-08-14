#Stereotypical Vending Machine Model
##Description
This project is created by the creator during a research program. This project generally focuses on creating such a piece of code to reveal potential security problems that might occur for stereotypical vending machine models. Granted, a lot of problems have been discovered. However, there might still be more problems that would require fixing. Additionally, there are some flaws, and the creator is planning to fix them in the future. However, it might take a while before the next change, because the creator, as a high school student, has to face a lot of other work currently.

Please notice that this is only a bare-bone creation aiming to simulate the most basic actions of a vending machine, and does not include any real products.

It focuses on simulating the actions of the core components of a vending machine.

**The author does not take any responsibility for damages caused by this code.**

##Prequisites

To use it, you will need the following:

4 LED Bulbs, respectively connected to pins 12, 14, 27, and 26;
1 ST7789 OLED Screen, with SCL pin connected to pin 18, SDA pin connected to pin23, RST pin connected to pin 22, DC pin connected to pin 25, CS pin connected to 33, and BL pin connected to pin 13;
1 4x4 Keypad, with row pins connected to pins 5, 2, 0, and 4, and column pins connected to 16, 17, 5, and 19;
1 Speaker, with I/O pin connected to pin 21; and
1 Servo Motor, with I/O pin connected to pin 32.

If not directly mentioned, all the VCC pins should be connected to the 5V pin, and all the GND pins should be connected to the GND pin.

In addition to the hardware components, you should also have an [Arduino IDE](https://www.arduino.cc/en/software) installed. Also, you should install the following libraries: Keypad by Mark Stanley and Alexander Brevig, Adafruit ST7735 and ST7789 Library by Adafruit, ESP32Servo by Kevin Harrington and John K. Bennett, and Seeed_Arduino_mbedtls by Peter Yang.

Apart from that, you will need to fill in the network details respectively on line 387 and line 412 for the ESP32 code file, and the client details on line 13 and line 42 for the server code file, following the instructions in the comments. You can fill in the Wi-Fi details and the server details first, connect the board to the Wi-Fi, and watch the Serial output to gain the IP address of the board, before filling that into the server file.

##How to Use

The file named "Stereotype_E1.ino" is the code file for the ESP32 board and should be burnt into the board using an Arduino IDE, after making the changes required. The file named "server.py" is the code file for the server and should be run locally on the computer with an Internet connection.

The board will do a verification process upon booting and will go into the input unit if everything is okay. Then you can follow the instructions displayed on the screen.

The process of using the machine is given here:

Firstly, type an approved ID using the keypad. Currently, the accepted IDs are 123456, 654321, and 121212. You can add more of them by adding more approved users to the server file. To do this, you have to come up with an ID and put the SHA256-encrypted text in the server file, using the same secret key on line 69 in the server file.

Then, you can type a commodity number. The machine will verify the commodity number and dispense the product if the commodity number is genuine and you have enough balance. In reality, the action of dispensing would be simply turning the servo motor, and there will be no real products dispensed.

##PS

If the creator becomes a hobbyist in the future, this may work as a valuable repository created by the creator, for it is (was) the first stone of the creator's code journey.
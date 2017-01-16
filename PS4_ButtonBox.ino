

/*
									  +-----+
		 +----[PWR]-------------------| USB |--+
		 |                            +-----+  |
		 |         GND/RST2  [ ][ ]            |
		 |       MOSI2/SCK2  [ ][ ]  A5/SCL[X] |   
		 |          5V/MISO2 [ ][ ]  A4/SDA[X] |   
		 |                             AREF[ ] |
		 |                              GND[ ] |
		 | [ ]N/C                    SCK/13[ ] |   B5
		 | [ ]IOREF                 MISO/12[ ] |   .
		 | [ ]RST                   MOSI/11[ ]~|   .
		 | [ ]3V3    +---+               10[ ]~|   .
		 | [X]5v    -| A |-               9[ ]~|   .
		 | [X]GND   -| R |-               8[ ] |   B0
		 | [ ]GND   -| D |-                    |
		 | [ ]Vin   -| U |-               7[ ] |   D7
		 |          -| I |-               6[ ]~|   .
		 | [ ]A0    -| N |-               5[ ]~|   .
		 | [ ]A1    -| O |-               4[ ] |   .
		 | [ ]A2     +---+           INT1/3[ ]~|   .
		 | [ ]A3                     INT0/2[ ] |   .
		 | [X]A4/SDA  RST SCK MISO     TX>1[ ] |   .
		 | [X]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0
		 |            [ ] [ ] [ ]              |
		 |  UNO_R3    GND MOSI 5V  ____________/
		  \_______________________/

A5 -> to SCL cascaded I2C IO extender PCF8574
A4 -> to SDA cascaded I2C IO extender PCF8574
+5V -> to +5V cascaded I2C IO extender PCF8574      
GND -> to GND cascaded I2C IO extender PCF8574      

		  http://busyducks.com/ascii-art-arduinos


*/


// USB HID keyboard helpers:
// http://hunt.net.nz/users/darran/weblog/faf5e/Arduino_UNO_Keyboard_HID_part_2.html
// http://www.mindrunway.ru/IgorPlHex/USBKeyScan.pdf
// https://gist.github.com/houmei/3827425

/*
 * Byte Contents
0 Modifier keys:
Bit 0 - Left CTRL
Bit 1 - Left SHIFT
Bit 2 - Left ALT
Bit 3 - Left GUI
Bit 4 - Right CTRL
Bit 5 - Right SHIFT
Bit 6 - Right ALT
Bit 7 - Right GUI
1 Not used
2 - 7 HID active key usage codes. This represents up to 6 keys currently being pressed.
*/

#include <Wire.h>
#include <pcf8574.h>

const uint8_t keyA = 0x04;
const uint8_t keyB = 0x05;
const uint8_t keyC = 0x06;
const uint8_t keyD = 0x07;
const uint8_t keyE = 0x08;
const uint8_t keyF = 0x09;
const uint8_t keyG = 0x0A;
const uint8_t keyH = 0x0B;
const uint8_t keyI = 0x0C;
const uint8_t keyJ = 0x0D;
const uint8_t keyK = 0x0E;
const uint8_t keyL = 0x0F;
const uint8_t keyM = 0x10;
const uint8_t keyN = 0x11;
const uint8_t keyO = 0x12;
const uint8_t keyP = 0x13;
const uint8_t keyQ = 0x14;
const uint8_t keyR = 0x15;
const uint8_t keyS = 0x16;
const uint8_t keyT = 0x17;
const uint8_t keyU = 0x18;
const uint8_t keyV = 0x19;
const uint8_t keyW = 0x1A;
const uint8_t keyX = 0x1B;
const uint8_t keyY = 0x1C;
const uint8_t keyZ = 0x1D;
const uint8_t key1 = 0x1E;
const uint8_t key2 = 0x1F;
const uint8_t key3 = 0x20;
const uint8_t key4 = 0x21;
const uint8_t key5 = 0x22;
const uint8_t key6 = 0x23;
const uint8_t key7 = 0x24;
const uint8_t key8 = 0x25;
const uint8_t key9 = 0x26;
const uint8_t key0 = 0x27;
const uint8_t keyF1 = 0x3A;
const uint8_t keyF2 = 0x3B;
const uint8_t keyF3 = 0x3C;
const uint8_t keyF4 = 0x3D;
const uint8_t keyF5 = 0x3E;
const uint8_t keyF6 = 0x3F;
const uint8_t keyF7 = 0x40;
const uint8_t keyF8 = 0x41;
const uint8_t keyF9 = 0x42;
const uint8_t keyF10 = 0x43;
const uint8_t keyF11 = 0x44;
const uint8_t keyF12 = 0x45;
const uint8_t keyF13 = 0x68;
const uint8_t keyF14 = 0x69;
const uint8_t keyF15 = 0x6a;
const uint8_t keyF16 = 0x6b;
const uint8_t keyF17 = 0x6c;
const uint8_t keyF18 = 0x6d;
const uint8_t keyF19 = 0x6e;
const uint8_t keyF20 = 0x6f;

const uint8_t keyHOME = 0x4A;
const uint8_t keyPGUP = 0x4B;
const uint8_t keyDEL = 0x4C;
const uint8_t keyEND = 0x4D;
const uint8_t keyPGDOWN = 0x4E;
const uint8_t keyRIGHT = 0x4F;
const uint8_t keyLEFT = 0x50;
const uint8_t keyDOWN = 0x51;
const uint8_t keyUP = 0x52;
const uint8_t keyENTER = 0x2;
const uint8_t keyESC = 0x29;
const uint8_t keyBACK = 0x2A;
const uint8_t keyTB = 0x2B;
const uint8_t keypad1 = 0x93;

uint8_t key[8] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t keyNone[8] = { 0, 0, 0, 0, 0, 0, 0 };

//Declare I2C IO extenders for matrix keypad
PCF8574 PCF_COL(0x20); //a0 a1 a2 all to GND
PCF8574 PCF_ROW(0x21); //a0 to vcc , a1 , a2 to gnd

uint8_t row=0;
uint8_t col=0;
uint8_t previous=0;
boolean repeat=0;
boolean pressed = false;

uint8_t KeyMatrix[8][8]={
  {keyA,keyB,keyC,keyD,keyE,keyF,keyG,keyH},
  {keyI,keyJ,keyK,keyL,keyM,keyN,keyO,keyP},
  {keyQ,keyR,keyS,keyT,keyU,keyV,keyW,keyX},
  {keyY,keyZ,key1,key2,key3,key4,key5,key6},
  {key7,key8,key9,key0,keyRIGHT,keyLEFT,keyDOWN,keyUP},
  {keyENTER,keyESC,keyBACK,keyTB,keyF12,keyF9,keyF10,keyF11},
  {keyF1,keyF2,keyF3,keyF4,keyF5,keyF6,keyF7,keyF8} ,
  {keyF13,keyF14,keyF15,keyF16,keyF17,keyF18,keyF19}
  };

//usefull for bitmasking
const int hex_data[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

void setup()
{
Serial.begin(9600);
delay(200);
Wire.begin(0);
PCF_COL.write8(0xff);  //Pull high
PCF_ROW.write8(0xff);  //Pull high
Serial.write(keyNone, 8);
delay(200);
}

void loop()
{
	row = 0;  col = 0;
	int r;
	// Search row low
	//scan each column by pulling down the column and checking for high state due to short by button press
	pressed = false;
	for (r = 0;r < 8;r++) {
		PCF_COL.write8(~hex_data[r]);		
		uint8_t valueCOL = ~PCF_COL.read8();
		uint8_t valueROW = ~PCF_ROW.read8();
		//Serial.print("COL: ");Serial.print(valueCOL);Serial.print(" ROW: ");Serial.println(valueROW);
		if (valueROW != 0) {// A key was pressed within the column
			pressed = true;
			col = r;
			while (valueROW >0) {
				valueROW = valueROW >> 1;
				row++;
			}
			key[2] = KeyMatrix[row][col];
			//Serial.print("KEYCOL: ");Serial.print(col);Serial.print(" KEYROW: ");Serial.print(row);Serial.print(" CAR: ");Serial.println(key[2]);
			if (previous == key[2]) {
				repeat = true;
			}
			else {
				repeat = false;
			}
			previous = key[2];
				//Serial.print(millis());Serial.print(" ");Serial.print("Row/Col Key Repeat ");Serial.print(" ");Serial.print(row);Serial.print("/");Serial.print(col);Serial.print(" ");Serial.print(key[2]);Serial.print(" ");Serial.println(repeat);
				Serial.write(key, 8);
				delay(10); // Give the host time to read the key
				Serial.read();
				Serial.write(keyNone, 8);
			if (repeat) {
				delay(30);
			}
			else {
				delay(400);
			}
		}
		
	}
	if (pressed == false) {
	// no keypress - avoid repeat
		previous = 0;
	}
}

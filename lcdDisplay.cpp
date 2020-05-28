#include <unistd.h>		   //Needed for I2C port
#include <fcntl.h>		   //Needed for I2C port
#include <sys/ioctl.h>	   //Needed for I2C port
#include <linux/i2c-dev.h> //Needed for I2C port
#include <iostream>
#include <bitset>
#include <vector>
#include <cstring>
#include "lcdDisplay.h"

#define OUT_REG 0x01
#define CONF_REG 0x03
#define I2C_BUS "/dev/i2c-1" // I2C bus device on a Raspberry Pi
#define I2C_ADDR 0x27		 // I2C slave address for the LCD module

using namespace std;

int file_i2c;

lcdDisplay::lcdDisplay()
{
	init();
}

lcdDisplay::~lcdDisplay()
{
	i2c_stop();
}

void lcdDisplay::i2c_start()
{
	if ((file_i2c = open(I2C_BUS, O_RDWR)) < 0)
	{
		printf("Error failed to open I2C bus [%s].\n", I2C_BUS);
		exit(-1);
	}
	// set the I2C slave address for all subsequent I2C device transfers
	if (ioctl(file_i2c, I2C_SLAVE, I2C_ADDR) < 0)
	{
		printf("Error failed to set I2C address [%s].\n", I2C_ADDR);
		exit(-1);
	}
}

void lcdDisplay::i2c_stop() { close(file_i2c); }

void lcdDisplay::i2c_send_byte(unsigned char data)
{
	unsigned char byte[2];
	byte[0] = OUT_REG;
	byte[1] = data;
	write(file_i2c, byte, sizeof(byte));
	/* -------------------------------------------------------------------- *
    * Below wait creates 1msec delay, needed by display to catch commands  *
    * -------------------------------------------------------------------- */
	usleep(1000);
}

void lcdDisplay::init()
{
	unsigned char buffer[60] = {0};
	i2c_start();

	// INit the I2C-Driver
	//----- Init the output register -----
	buffer[0] = OUT_REG;
	buffer[1] = 0x00;
	write(file_i2c, buffer, sizeof(buffer));

	usleep(1000);

	//----- Init the configuration register -----
	buffer[0] = CONF_REG;
	buffer[1] = 0x00;
	write(file_i2c, buffer, sizeof(buffer));
	// ENd I2C-Driver init

	//Initial configuration
	usleep(15000);			   // wait 15msec
	i2c_send_byte(0b00110100); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=1
	i2c_send_byte(0b00110000); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=0
	usleep(4100);			   // wait 4.1msec
	i2c_send_byte(0b00110100); //
	i2c_send_byte(0b00110000); // same
	usleep(100);			   // wait 100usec
	i2c_send_byte(0b00110100); //
	i2c_send_byte(0b00110000); // 8-bit mode init complete
	usleep(4100);			   // wait 4.1msec
	i2c_send_byte(0b00100100); //
	i2c_send_byte(0b00100000); // switched now to 4-bit mode

	//End of initial configuration

	// INit the display
	usleep(31000);

	//----- Function Set -----
	i2c_send_byte(0b00100100); //
	i2c_send_byte(0b00100000); // keep 4-bit mode
	i2c_send_byte(0b10000100); //
	i2c_send_byte(0b10000000); // D3=2lines, D2=char5x8

	usleep(40);

	//----- Display Control -----
	i2c_send_byte(0b00000100); //
	i2c_send_byte(0b00000000); // D7-D4=0
	i2c_send_byte(0b10000100); //
	i2c_send_byte(0b10000000); // D3=1 D2=display_off, D1=cursor_off, D0=cursor_blink

	usleep(40);

	//------- Display Clear ------
	i2c_send_byte(0b00000100); //
	i2c_send_byte(0b00000000); // D7-D4=0
	i2c_send_byte(0b00010100); //
	i2c_send_byte(0b00010000); // D0=display_clear

	usleep(1600);

	//------ Entry Mode --------
	i2c_send_byte(0b00000100); //
	i2c_send_byte(0b00000000); // D7-D4=0
	i2c_send_byte(0b01100100); //
	i2c_send_byte(0b01100000); // print left to right

	usleep(40);

	//------ Display turn on -------
	i2c_send_byte(0b00000100); //
	i2c_send_byte(0b00000000); // D7-D4=0
	i2c_send_byte(0b11000100); //
	i2c_send_byte(0b11000000); // D3=1 D2=display_on, D1=cursor_on, D0=cursor_blink

	// End display init
}

string lcdDisplay::TextToBinaryString(string data)
{
	string binaryString = "";
	for (char &_char : data)
	{
		binaryString += bitset<8>(_char).to_string();
	}
	return binaryString;
}

void lcdDisplay::ClearDisplay()
{
	//------- Display Clear ------
	i2c_send_byte(0b00000100); //
	i2c_send_byte(0b00000000); // D7-D4=0
	i2c_send_byte(0b00010100); //
	i2c_send_byte(0b00010000); // D0=display_clear

	usleep(1600);
}

void lcdDisplay::RowSelection(int row)
{
	// Init row 0
	if (row == 0 || row != 1)
	{
		i2c_send_byte(0b00000100); //
		i2c_send_byte(0b00000000); // D7-D4=0
		i2c_send_byte(0b00100100); //
		i2c_send_byte(0b00100000); // return cursor to home position
	}
	// Init row 1
	else
	{
		i2c_send_byte(0b11000100); //
		i2c_send_byte(0b11000000); // D7-D4=0
		i2c_send_byte(0b00000100); //
		i2c_send_byte(0b00000000); // print on lower row, first collum
	}

	usleep(40);
}

void lcdDisplay::sendStringToLCD(string StringToConvert, int row)
{
	string BinaryString = TextToBinaryString(StringToConvert); // Convert String to a binary output
	int decimal = 0;

	string EN_SIGNAL = "1101";	  // Enable high signal
	string LATCH_SIGNAL = "1001"; // Enable Low signal (latch)

	int StringLenght = BinaryString.length();

	RowSelection(row);

	for (int i = 0; i < StringLenght; i += 4)
	{
		string sub = BinaryString.substr(i, 4);

		decimal = stoi(sub + EN_SIGNAL, 0, 2);
		i2c_send_byte(decimal);

		decimal = stoi(sub + LATCH_SIGNAL, 0, 2);
		i2c_send_byte(decimal);
	}
}

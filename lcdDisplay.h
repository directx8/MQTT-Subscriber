#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H
#include <string>

using namespace std;

class lcdDisplay
{
private:
    void init();
    void i2c_start();
    void i2c_stop();
    void i2c_send_byte(unsigned char data);
    void RowSelection(int row);
    string TextToBinaryString(string data);

public:
    lcdDisplay();
    ~lcdDisplay();
    void ClearDisplay();
    void sendStringToLCD(string StringToConvert, int row);
};

#endif
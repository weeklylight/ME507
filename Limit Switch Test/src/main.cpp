#include <Arduino.h>
void setup() 
{
  Serial.begin(115200);
}

void loop()
{
  const uint8_t angleRes = 10; // resolution of grid in degrees
  const uint8_t maxAngle = 35; // maximum angle of device
  const uint8_t divCount = maxAngle / angleRes; // number of cells in one direction, rounds down
  const uint32_t sivArraySize = (2*divCount + 1)*(2*divCount + 1);

  int sivArrayX [sivArraySize] = {0}; // creates an array with (2n+1)^2 cells (a divCount of 2 makes 9 squares)
  int sivArrayY [sivArraySize] = {0};
  sivArrayX[0] = angleRes * divCount;
  sivArrayY[0] = angleRes * divCount;
  int8_t dirFlag = -1;
  bool flipFlag = true;
  for (uint8_t i = 1; i < sivArraySize; i++)
  {
    if ((flipFlag == false) && (abs(sivArrayX[i-1]) == abs (angleRes * divCount)))
    {
      dirFlag = -1 * dirFlag;
      sivArrayY[i] = sivArrayY[i-1] - angleRes;
      sivArrayX[i] = sivArrayX[i-1];
      flipFlag = true;
    }
    else
    {
      sivArrayY[i] = sivArrayY[i-1];
      sivArrayX[i] = sivArrayX[i-1] + dirFlag * angleRes;
      flipFlag = false;
    }
    
    Serial.print(i-1);
    Serial.print(": ");
    Serial.print(sivArrayX[i-1]);
    Serial.print(", ");
    Serial.println(sivArrayY[i-1]);

  }
  Serial.print(48);
  Serial.print(": ");
  Serial.print(sivArrayX[48]);
  Serial.print(", ");
  Serial.println(sivArrayY[48]);
  delay(10000);
}
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4 // display connected to I2C on analog pins 4 and 5

Adafruit_SSD1306 display(OLED_RESET);
int gyro = 0; // analog pin 0
float gyroVoltage = 5; // running at 5v
float gyroZeroVoltage; // 2.5 is what the spec sheet said, but calibration is better than static
float gyroSensitivity = .007; // from the spec sheet, .007/degree/second
float rotationThreshold = 1; // filter out some of the noise

float currentAngle = 0;

int startTime = 0;
int endTime = 0;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,1);
  display.print("Calibrating!!");
  display.display();
  float calibrate=0;
  delay(100); // wait for the gyro just a little, seems to make the calibrate a little more accurate
  for (int i =0; i<1500;i++) { // while the gyro is still, check the reported voltage ever 1ms for 1500 counts, enables creating an average of the observed idle voltage
    calibrate += (analogRead(gyro) * gyroVoltage) / 1023;
    delay(1);
  }
  gyroZeroVoltage = calibrate/1500; //use the observed average zero as the zero voltage for furture calculations
}

void loop() {
  endTime = startTime; // Captures the time that the previous loop actually took.
  startTime = millis();
  float gyroRate = (analogRead(gyro) * gyroVoltage)/1023;
  gyroRate -= gyroZeroVoltage;
  gyroRate /= gyroSensitivity;
  if (gyroRate >= rotationThreshold || gyroRate <= -rotationThreshold) {
    gyroRate /= (1000/(startTime-endTime)); //adjusts the time in motion calc for the time the previous loop actually took
    currentAngle += gyroRate;
  }
  if (currentAngle < 0) {//keeps the reported angle between 0 and 360, rather than counting rotations...  which you could do, if you want...  might be helpful when the robot is on a cable, so it can calculate how to untangle...
    currentAngle += 360;
  } else if (currentAngle >= 360) {
    currentAngle -= 360;
  }
  long now = millis();
  long minutes = (now - (now % 60000))/60000;
  int seconds = (now % 60000)/1000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Gyro: ");
  display.print(currentAngle);
  display.println((char)247);
  display.print("ZeroVoltage: ");
  display.println(gyroZeroVoltage,6);
  display.print("Time: ");
  display.print(minutes);
  display.print(":");
  display.print(seconds);
  display.display();
  delay(1); //could probably drop this, as the processing loop (with display interaction) is taking about 29ms without any delay() calls, but not hurting anything...  and having a pause in a continous loop is a good idea...  usually...

}

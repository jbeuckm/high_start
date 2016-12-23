#include "I2Cdev.h"
#include "MPU6050.h"
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

Servo myservo;
int pos = 0;
int dir = 1;

const int chipSelect = 13;
File dataFile;

void setup() {
  Serial.begin(38400);


  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
        Serial.println("Fastwire");
  #endif

  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  myservo.attach(9);


  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
  
  dataFile = SD.open("datalog.tsv", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }

      String dataString = "millis\t";

  dataString += "ax\t"; 
  dataString += "ay\t"; 
  dataString += "az\t"; 
  dataString += "gx\t"; 
  dataString += "gy\t"; 
  dataString += "gz\t"; 

  dataFile.println(dataString);
        Serial.println(dataString);
}

void loop() {

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  String dataString = String(millis()) + "\t";

  dataString += String(ax) + "\t"; 
  dataString += String(ay) + "\t"; 
  dataString += String(az) + "\t"; 
  dataString += String(gx) + "\t"; 
  dataString += String(gy) + "\t"; 
  dataString += String(gz) + "\t"; 

  dataFile.println(dataString);
  dataFile.flush();

  // print to the serial port too:
  Serial.println(dataString);

  
    myservo.write(ay >> 7);
    pos += dir;
    if ((dir == 1) && (pos >= 180)) dir = -1;
    else if ((dir == -1) && (pos <= 0)) dir = 1;

}

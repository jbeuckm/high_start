
//#include <Adafruit_GPS.h>

#include <PID_v1.h>
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

const int chipSelect = 13;
File dataFile;


void setupAccelGyro() {
  accelgyro.initialize();

  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}


Servo xServo;
Servo yServo;

double xSetpoint, xInput, xOutput;
PID xPID(&xInput, &xOutput, &xSetpoint, .1,0,.01, DIRECT);

double ySetpoint, yInput, yOutput;
PID yPID(&yInput, &yOutput, &ySetpoint, .1,0,.01, DIRECT);


void setupServos() {
  
  xInput = ax;
  xSetpoint = 0;
  xPID.SetOutputLimits(-90, 90);
  xPID.SetMode(AUTOMATIC);
  
  yInput = ay;
  ySetpoint = 0;
  yPID.SetOutputLimits(-90, 90);
  yPID.SetMode(AUTOMATIC);
  
  xServo.attach(8);
  yServo.attach(9);
}



void dateTime(uint16_t* date, uint16_t* time) {

  *date = FAT_DATE(2015, 12, 25);

  *time = FAT_TIME(12, 0, 0);
}


void setupSDcard() {
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");

  String filename = "tracking.tsv";
  
  SdFile::dateTimeCallback(dateTime);
  dataFile = SD.open(filename, O_WRITE | O_CREAT | O_TRUNC);
//  dataFile = SD.open(filename, FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening "+filename);
    // Wait forever since we cant write data
    while (1) ;
  }

  String dataString = "millis\tax\tay\taz\tgx\tgy\tgz\txServo\tyServo"; 
  dataFile.println(dataString);
  
  Serial.println("starting TSV file write with columns...");
  Serial.println(dataString);
}


void setup() {
  Serial.begin(38400);

  Serial.println("Initializing I2C devices...");

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
        Serial.println("Fastwire");
  #endif

  setupAccelGyro();

  setupServos();

  setupSDcard();
}



void writeSDcardData() {
  
  String dataString = String(millis()) + "\t";

  dataString += String(ax) + "\t"; 
  dataString += String(ay) + "\t"; 
  dataString += String(az) + "\t"; 
  dataString += String(gx) + "\t"; 
  dataString += String(gy) + "\t"; 
  dataString += String(gz) + "\t"; 

  dataString += String(xOutput) + "\t"; 
  dataString += String(yOutput); 

  dataFile.println(dataString);
  dataFile.flush();

  // print to the serial port too:
  Serial.println(dataString);

}

void updateServos() {
  
  xInput = ax;
  xPID.Compute();    
  xServo.write(90 + xOutput);
  
  yInput = ay;
  yPID.Compute();    
  yServo.write(90 + yOutput);
  
}



void loop() {

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  updateServos();

  writeSDcardData();

}


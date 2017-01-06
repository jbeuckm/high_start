#include <Adafruit_BMP280.h>

#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>

#include <PID_v1.h>
//#include "I2Cdev.h"
#include "MPU6050.h"
#include <Servo.h>
#include <SPI.h>
#include <SD.h>


#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif


TinyGPS gps;
SoftwareSerial gpsSerial(2,4);

Adafruit_BMP280 bmp;
  
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

const int chipSelect = 13;
File gyroDataFile, gpsDataFile;

#define TAB_CHAR F("\t")

void setupAccelGyro() {
  accelgyro.initialize();

  Serial.println(F("Testing device connections..."));
  Serial.println(accelgyro.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
}


Servo servo1, servo2, servo3;

double xSetpoint, xInput, xOutput;
PID xPID(&xInput, &xOutput, &xSetpoint, .05, 0, .01, DIRECT);

double ySetpoint, yInput, yOutput;
PID yPID(&yInput, &yOutput, &ySetpoint, .05, 0, .01, DIRECT);


void setupServos() {
  
  xInput = ax;
  xSetpoint = 0;
  xPID.SetOutputLimits(-90, 90);
  xPID.SetMode(AUTOMATIC);
  
  yInput = ay;
  ySetpoint = 0;
  yPID.SetOutputLimits(-90, 90);
  yPID.SetMode(AUTOMATIC);
  
  servo1.attach(3);
  servo2.attach(5);
  servo3.attach(6);
}


unsigned long fix_age;
int year;
byte month, day, hour, minute, second, hundredths;

void dateTime(uint16_t* date, uint16_t* time) {

  *date = FAT_DATE(year, month, day);

  *time = FAT_TIME(hour, minute, second);
}


void setupSDcard() {
  Serial.println(F("Initting SD..."));
  pinMode(SS, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD init failed"));
    // don't do anything more:
//    while (1) ;
  }
  Serial.println(F("SD initted"));

  String gyrosFilename = F("gyros.tsv");
  
  SdFile::dateTimeCallback(dateTime);
  gyroDataFile = SD.open(gyrosFilename, O_WRITE | O_CREAT | O_TRUNC);

  if (!gyroDataFile) {
    Serial.print(F("error opening "));
    Serial.println(gyrosFilename);
//    while (1) ;
  }

  gyroDataFile.println(F("millis\tax\tay\taz\tgx\tgy\tgz\tservo1\tyservo2\tservo3"));
  

  String gpsFilename = F("gps.tsv");
  
  SdFile::dateTimeCallback(dateTime);
  gpsDataFile = SD.open(gpsFilename, O_WRITE | O_CREAT | O_TRUNC);

  if (!gpsDataFile) {
    Serial.print(F("error opening "));
    Serial.println(gpsFilename);
    while (1) ;
  }

  gpsDataFile.println(F("millis\tsatellites\tlat\tlon\taltitude\tspeed"));
  
}


void setup() {

  Serial.begin(115200);
  gpsSerial.begin(9600);

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
  #endif

  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }
 
  setupAccelGyro();

  setupServos();
}



void writeSDcardData() {

  String dataString = String(millis()) + TAB_CHAR;

  dataString += String(ax) + TAB_CHAR;
  dataString += String(ay) + TAB_CHAR;
  dataString += String(az) + TAB_CHAR;
  dataString += String(gx) + TAB_CHAR;
  
  dataString += String(gy) + TAB_CHAR;
  dataString += String(gz) + TAB_CHAR;

  dataString += String(xOutput) + TAB_CHAR;
  dataString += String(yOutput);

  gyroDataFile.println(dataString);
  Serial.println(dataString);

  gyroDataFile.flush();
}

void updateServos() {
  
  xInput = ax;
  xPID.Compute();
//  xServo.write(90 + xOutput);
  
  yInput = ay;
  yPID.Compute();
//  yServo.write(90 + yOutput);
  
}

enum MISSION_STATE {
  WAIT_FOR_FIX,
  BOOST,
  COAST,
  DROGUE,
  RECOVERY,
  LANDED
};

MISSION_STATE mission_state = WAIT_FOR_FIX;

void checkGpsReady() {
  
  while (gpsSerial.available()) {
    int c = gpsSerial.read();
    if (gps.encode(c)) {
       
      gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

      setupSDcard();
      
      Serial.println(F("GPS Fix"));

      mission_state = BOOST;
    }
  }
}


void updateGPS() {
  
  while (gpsSerial.available()) {
    int c = gpsSerial.read();
    if (gps.encode(c))
    {

      float flat, flon;
      unsigned long time, date, speed, course;

      gps.f_get_position(&flat, &flon, &fix_age);

      String dataString = String(millis()) + TAB_CHAR + gps.satellites() + TAB_CHAR;

      dataString += String(flat)+TAB_CHAR+String(flon)+TAB_CHAR;
      dataString += String(gps.f_altitude())+TAB_CHAR+String(gps.f_speed_mps());
      
      gpsDataFile.println(dataString);
      gpsDataFile.flush();
      Serial.println(dataString);

    }
    
  }
}


void loop() {

  switch (mission_state) {
    case WAIT_FOR_FIX:
      checkGpsReady();
      break;
    case BOOST:
      boost_loop();
      break;
    case COAST:
      coast_loop();
      break;
    case DROGUE:
      break;
    case RECOVERY:
      break;
    case LANDED:
      break;
  }
}

void boost_loop() {

//  bmp.readPressure();

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  updateServos();

  writeSDcardData();

  updateGPS();
}


void coast_loop() {
/*  
  Serial.print(F("Pressure = "));
  Serial.print(bmp.readPressure());
  Serial.println(F(" Pa"));

  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(1013.25)); // this should be adjusted to your local forcase
  Serial.println(F(" m"));
*/
}




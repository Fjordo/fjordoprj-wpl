/*
  Arduino YUN Sketch.
  Uses a sonar sensor to show the distance from an object aimed by the sensor.
  Sends the sensor readed data, and other computed data, to a remote server with a POST call.

  Circuit Scheme:

  SONAR (HC-SR04) Ping distance sensor:
    VCC to arduino 5v
    GND to arduino GND
    Echo to Arduino pin 7, green wire
    Trig to Arduino pin 8, yellow wire


    The Timing diagram is shown at https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf.
    You only need to supply a short 10uS pulse to the trigger input to start the ranging, and then the module will send out an 8 cycle burst of ultrasound at 40 kHz and raise its echo.
    The Echo is a distance object that is pulse width and the range in proportion.
    You can calculate the range through the time interval between sending trigger signal and receiving echo signal.
    Formula: uS / 58 = centimeters or uS / 148 = inch;
    or: the range = high level time * velocity (340M/S) / 2;
    we suggest to use over 60ms measurement cycle, in order to prevent trigger signal to the echo signal.

  examples:
  https://create.arduino.cc/projecthub/abdularbi17/ultrasonic-sensor-hc-sr04-with-arduino-tutorial-327ff6
  http://winkleink.blogspot.com.au/2012/05/arduino-hc-sr04-ultrasonic-distance.html

  -----------------------
  WI-FI CONF.
  Arduino YUN wi-fi has to be previously configured.
  See https://www.twilio.com/blog/2015/02/arduino-wifi-getting-started-arduino-yun.html for a guide.

*/

// include the library code:
#include <LiquidCrystal.h>
#include <Process.h>

#define echoPin 7 // Echo Pin, green wire
#define trigPin 8 // Trigger Pin, yellow wire
#define LEDPin 13 // Onboard LED

int maximumRange = 300;  // Maximum range needed in cm
int minimumRange = 0;    // Minimum range needed
double duration, durSum; // Duration used to calculate distance, and variable to calculate the duration mean
double distance;         // Distance from the water surface
double volume;           // Residual volume

String server = "http://fjordoprj.altervista.org";

void setup()
{
  // Initialize Bridge
  Bridge.begin();

  // set up the SONAR
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
}

void loop()
{
  /* The following trigPin/echoPin cycle is used to determine the
    distance of the nearest object by bouncing soundwaves off of it.
    It gets 10 measures and does an arithmentic mean
  */

  //  int i = 0;
  duration = 0; //reset value for next cicle

  //while (i < 10) {

  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds to start the trigger
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  durSum = pulseIn(echoPin, HIGH);

  Serial.println("durSum:" + (String)durSum);

  duration = durSum + duration;

  Serial.println("duration: " + (String)duration);
  //i++;
  //}

  // calculating the duration mean on 10 measurement
  // duration = duration / 10;

  // Calculating the distance
  // Speed of sound wave divided by 2 (go and back)
  distance = duration * 0.034 / 2;
  // distance = duration / 58;

  // Displays the distance on the Serial Monitor
  Serial.println("Distance: " + (String) distance + " cm");

  /*
    COMPUTE VOLUME OF THE WATER IN THE WELL
  */
  volume = computeVolume(distance);

  /*
    SENDING DATA TO SERVER
  */
  //sendData(distance, volume);

  // Delay 10 seconds before next reading.
  // delay(10000);
  // Delay 3600000 mS (1 hour) before next reading.
  // delay(3600000);
  // 12 h before next reading.
  // delay(43200000);
  // 24 h before next reading.
  delay(86400000);
  
}

/**
   Computes the water volume inside the well
   the sensor is about 40 cm over the maximum water level, so I compensate this subtracting 40 cm.
*/
double computeVolume(long distance)
{
  if (distance > 0) {
    // outer well dimension: base diameter of 2 meters, 3 meters of height
    // inner well dimension
    int wellHeight = 30;                    // measure in decimeters
    double wellDiameter = 20;               // measure in dm
    double waterHeight = (distance - 40) / 10; // subtract a default of 4 decimeters from the height measured by the sensor. Then compute the total in decimeters
    double base = wellDiameter * 3.14159;
    return (base * waterHeight);
  } else {
    return -1;
  }
}

void sendData(long distance, double volume)
{

  Process p; // Create a process and call it "p"

  p.runShellCommand("curl -X POST https://fjordoprj.altervista.org/wpl/add.php -F dist=" + (String)distance + " -F vol=" + (String)volume);

  // A process output can be read with the stream methods
  //  while (p.available() > 0) {
  //    char c = p.read();
  //    Serial.print(c);
  //  }
  // Ensure the last bit of data is sent.
  //  Serial.flush();
}

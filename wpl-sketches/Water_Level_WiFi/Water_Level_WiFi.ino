
/*
  Arduino YUN Sketch.
  Uses a sonar sensor and an LCD 16*x2 panel
  to show the distance from an object aimed by the sensor.
  Sends the sensor readed data, and other computed data, to a remote server with a POST call.

  Circuit Scheme:
   LCD RS pin to digital pin 12
   LCD Enable pin to digital pin 11
   LCD D4 pin to digital pin 5
   LCD D5 pin to digital pin 4
   LCD D6 pin to digital pin 3
   LCD D7 pin to digital pin 2
   LCD R/W pin to ground
   LCD VSS pin to ground
   LCD VCC pin to 5V
   10K resistor:
    15 ends to +5V and 16 to ground
    wiper to LCD VO pin (pin 3)

  SONAR (HC-SR04) Ping distance sensor:
    VCC to arduino 5v
    GND to arduino GND
    Echo to Arduino pin 7
    Trig to Arduino pin 8

  Sonar sketch originates from Virtualmix: http://goo.gl/kJ8Gl
  Has been modified by Winkle ink here: http://winkleink.blogspot.com.au/2012/05/arduino-hc-sr04-ultrasonic-distance.html
  And modified further by ScottC here: http://arduinobasics.blogspot.com.au/2012/11/arduinobasics-hc-sr04-ultrasonic-sensor.html
  on 10 Nov 2012.

  -----------------------
  CONF. WI-FI
  Arduino YUN wi-fi has to be previously configured.
  See https://www.twilio.com/blog/2015/02/arduino-wifi-getting-started-arduino-yun.html for a guide.

*/

// include the library code:
#include <LiquidCrystal.h>
#include <Process.h>

#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int maximumRange = 200;  // Maximum range needed in cm
int minimumRange = 0;    // Minimum range needed
long duration, distance; // Duration used to calculate distance
double volume;           // residual volume

String server = "http://fjordoprj.altervista.org";

void setup()
{
  // Initialize Bridge
  Bridge.begin();

  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("distanza in cm!");

  // set up the SONAR
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
}

void loop()
{

  /* The following trigPin/echoPin cycle is used to determine the
    distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;

  if (distance >= maximumRange || distance <= minimumRange)
  {
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    /* Send a negative number to computer and Turn LED ON
      to indicate "out of range" */
    lcd.print("-1");
    digitalWrite(LEDPin, HIGH);
  }
  else
  {
    /* Send the distance to the computer using Serial protocol, and
      turn LED OFF to indicate successful reading. */

    lcd.setCursor(0, 1);
    // print the distance in cm:
    if (distance < 10)
    {
      lcd.clear();
      lcd.print("distanza in cm!");
      lcd.setCursor(0, 1);
    }
    lcd.print(distance);

    digitalWrite(LEDPin, LOW);
  }

  /*
    COMPUTE VOLUME OF THE WELL
  */
  volume = computeVolume(distance);

  /*
    SENDING DATA TO SERVER
  */
  sendData(distance, volume);

  // Delay 3600000 mS (1 hour) before next reading.
  delay(3600000);
  // delay(86400000);
}

double computeVolume(long distance)
{                                         // outer well dimension: base diameter of 2.5 meters, 2,5 meters of height
  int wellHeight = 22;                    // measure in decimeters
  double diametroCisterna = 23;           // measure in decimeters
  int waterHeight = (distance - 40) / 10; // subtract a default of 4 decimeters from the height measured by the sensor

  long realHeight = wellHeight;

  if (waterHeight > 0)
  {
    realHeight = wellHeight - waterHeight;
  }

  double base = 11.5 * 11.5 * 3.1419;

  return (base * realHeight);
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


/*
  Sketch per Arduino YUN.
  Utilizza un sensore sonar e un pannello a LCD 16*x2
  per mostrare la distanza a cui si trova l'oggetto puntato dal sensore.
  Invia il dato letto dal sensore e altri calcolati ad un server remoto mediante una chiamata POST

  Schema Circuito:
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
  Arduino YUN deve essere preventivamente configurato, mediante onfigurazione iniziale.
  Si rimanda a https://www.twilio.com/blog/2015/02/arduino-wifi-getting-started-arduino-yun.html per una guida.

*/

// include the library code:
#include <LiquidCrystal.h>
#include <Process.h>

#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int maximumRange = 200; // Maximum range needed in cm
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
double volume; // residual volume

String server = "http://fjordoprj.altervista.org";

void setup() {
  // Initialize Bridge
  Bridge.begin();

  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("distanza in cm!");

  // set up the SONAR
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)

}

void loop() {

  /* The following trigPin/echoPin cycle is used to determine the
    distance of the nearest object by bouncing soundwaves off of it. */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration / 58.2;

  if (distance >= maximumRange || distance <= minimumRange) {
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    /* Send a negative number to computer and Turn LED ON
      to indicate "out of range" */
    lcd.print("-1");
    digitalWrite(LEDPin, HIGH);
  }
  else {
    /* Send the distance to the computer using Serial protocol, and
      turn LED OFF to indicate successful reading. */

    lcd.setCursor(0, 1);
    // print the distance in cm:
    if (distance < 10) {
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

  //Delay 3600000 mS (1 hour) before next reading.
  delay(1000);
}


double computeVolume(long distance) {  // Dimensioni esterne cisterna: diametro base di 2.5 metri, altezza di 2.5 metri
  int altezzaCisterna = 22; // misura in decimetri
  double diametroCisterna = 23; // dm
  int altezzaAcqua = (distance - 40) / 10; // sottraggo 4 decimetri di default rispetto all'altezza misurata dal sensore

  long altezzaEffettiva = altezzaCisterna;

  if (altezzaAcqua > 0) {
    altezzaEffettiva = altezzaCisterna - altezzaAcqua;
  }

  double base = 11.5 * 11.5 * 3.1419;

  return (base * altezzaEffettiva);
}

void sendData(long distance, double volume) {

  Process p;        // Create a process and call it "p"

  p.runShellCommand("curl -X POST https://fjordoprj.altervista.org/wpl/add.php -F dist=" + (String)distance + " -F vol=" + (String)volume);

  // A process output can be read with the stream methods
  //  while (p.available() > 0) {
  //    char c = p.read();
  //    Serial.print(c);
  //  }
  // Ensure the last bit of data is sent.
  //  Serial.flush();
}

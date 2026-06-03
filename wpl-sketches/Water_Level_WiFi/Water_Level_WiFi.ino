/*
  Arduino YUN Sketch.
  Uses a sonar sensor and an LCD 16*2 panel
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

  -----------------------
  API TOKEN
  Set API_TOKEN below to match the value of API_TOKEN in wpl/.env on the server.
*/

#include <LiquidCrystal.h>
#include <Process.h>

#define ECHO_PIN 7  // Echo Pin
#define TRIG_PIN 8  // Trigger Pin
#define LED_PIN  13 // Onboard LED

#define NUM_SAMPLES      5    // readings per measurement cycle — median is used
#define MAX_RANGE_CM   200    // Maximum range needed in cm
#define MIN_RANGE_CM     2    // Minimum range needed
#define SENSOR_OFFSET_CM 40   // sensor is mounted 40 cm above max water level

#define MAX_RETRIES 3         // HTTP POST retries before giving up

// *** Set this token to match the API_TOKEN value in wpl/.env ***
const String API_TOKEN  = "your_secret_token_here";
const String SERVER_URL = "https://fjordoprj.altervista.org/wpl/add.php";

// Well dimensions (LCD variant)
const int    WELL_HEIGHT_DM   = 22;   // measure in decimeters
const double WELL_DIAMETER_DM = 23.0; // measure in decimeters

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


/*
  Takes NUM_SAMPLES readings from the HC-SR04, applies a bubble-sort median
  to reject outlier echoes, and returns the distance in cm.
  Returns -1 if the median falls outside the sensor's valid range.
*/
double measureDistance() {
  double readings[NUM_SAMPLES];

  /* The following trigPin/echoPin cycle is used to determine the
     distance of the nearest object by bouncing soundwaves off of it.
     It gets NUM_SAMPLES measures and returns the median. */
  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Clears the trigPin condition
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds to start the trigger
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Reads the echoPin; 30 ms timeout avoids blocking if no echo is received
    double duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
    readings[i] = duration * 0.034 / 2.0; // Speed of sound wave divided by 2 (go and back)
    delay(60); // minimum 60 ms between pulses per datasheet
  }

  // Bubble sort to find median — discards high and low outliers
  for (int i = 0; i < NUM_SAMPLES - 1; i++) {
    for (int j = i + 1; j < NUM_SAMPLES; j++) {
      if (readings[j] < readings[i]) {
        double tmp = readings[i];
        readings[i] = readings[j];
        readings[j] = tmp;
      }
    }
  }

  double median = readings[NUM_SAMPLES / 2];

  if (median < MIN_RANGE_CM || median > MAX_RANGE_CM) {
    return -1; // out of range
  }

  return median;
}


/*
  COMPUTE VOLUME OF THE WELL
  The sensor is about 40 cm over the maximum water level, so we compensate
  by subtracting SENSOR_OFFSET_CM before converting to decimeters.
  outer well dimension: base diameter of 2.5 meters, 2.5 meters of height
*/
double computeVolume(double distance) {
  if (distance <= 0) return -1;

  // subtract a default of 4 decimeters from the height measured by the sensor
  double waterHeightDm = (distance - SENSOR_OFFSET_CM) / 10.0;
  if (waterHeightDm < 0) waterHeightDm = 0;

  double radius = WELL_DIAMETER_DM / 2.0;
  double base   = radius * radius * 3.14159; // area = π × r²
  return base * waterHeightDm;
}


/*
  SENDING DATA TO SERVER
  Sends distance and volume via HTTPS POST to add.php.
  Retries up to MAX_RETRIES times on failure; checks HTTP response code.
  Returns true on success (HTTP 201), false if all retries fail.
*/
bool sendData(double distance, double volume) {
  for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
    Process p; // Create a process and call it "p"
    p.runShellCommand(
      "curl -s -o /dev/null -w \"%{http_code}\" -X POST " + SERVER_URL +
      " -F token=" + API_TOKEN +
      " -F dist=" + String((int)distance) +
      " -F vol=" + String(volume)
    );

    // Read HTTP response code from curl output
    String httpCode = "";
    while (p.available() > 0) {
      httpCode += (char)p.read();
    }
    httpCode.trim();

    Serial.println("Attempt " + String(attempt) + " HTTP " + httpCode);

    if (httpCode == "201") {
      return true;
    }

    if (attempt < MAX_RETRIES) {
      delay(5000); // wait 5 s before retry
    }
  }
  return false;
}


void setup() {
  // Initialize Bridge
  Bridge.begin();

  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("distanza in cm!");

  // set up the SONAR
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT); // Use LED indicator (if required)
}


void loop() {
  double distance = measureDistance();
  double volume   = computeVolume(distance);

  // Update LCD display
  lcd.setCursor(0, 0);
  lcd.print("Dist:           ");
  lcd.setCursor(6, 0);

  if (distance < 0) {
    /* Send a negative number to computer and Turn LED ON
       to indicate "out of range" */
    lcd.print("-1");
    digitalWrite(LED_PIN, HIGH);
  } else {
    /* Send the distance to the computer using Serial protocol, and
       turn LED OFF to indicate successful reading. */
    if (distance < 10) {
      lcd.clear();
      lcd.print("distanza in cm!");
      lcd.setCursor(6, 0);
    }
    lcd.print(String((int)distance) + " cm");
    digitalWrite(LED_PIN, LOW);
  }

  lcd.setCursor(0, 1);
  lcd.print("Vol:            ");
  lcd.setCursor(5, 1);
  lcd.print(String((int)volume) + " dm3");

  Serial.println("Distance: " + String(distance) + " cm");
  Serial.println("Volume: " + String(volume) + " dm3");

  /*
    SENDING DATA TO SERVER
  */
  sendData(distance, volume);

  // Delay 3600000 mS (1 hour) before next reading.
  delay(3600000UL);
}

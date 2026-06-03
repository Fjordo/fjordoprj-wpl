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

  -----------------------
  API TOKEN
  Set API_TOKEN below to match the value of API_TOKEN in wpl/.env on the server.
*/

#include <Process.h>

#define ECHO_PIN 7  // Echo Pin, green wire
#define TRIG_PIN 8  // Trigger Pin, yellow wire
#define LED_PIN  13 // Onboard LED

#define NUM_SAMPLES      5    // readings per measurement cycle — median is used
#define MAX_RANGE_CM   300    // Maximum range needed in cm
#define MIN_RANGE_CM     2    // Minimum range needed
#define SENSOR_OFFSET_CM 40   // sensor is mounted 40 cm above max water level

#define MAX_RETRIES 3         // HTTP POST retries before giving up

// *** Set this token to match the API_TOKEN value in wpl/.env ***
const String API_TOKEN  = "your_secret_token_here";
const String SERVER_URL = "https://fjordoprj.altervista.org/wpl/add.php";

// Well dimensions (no-LCD variant)
const int    WELL_HEIGHT_DM   = 30;   // measure in decimeters
const double WELL_DIAMETER_DM = 20.0; // measure in decimeters


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

    // Reads the echoPin, returns the sound wave travel time in microseconds
    // 30 ms timeout avoids blocking if no echo is received
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

  // Discard reading if out of sensor range
  if (median < MIN_RANGE_CM || median > MAX_RANGE_CM) {
    return -1;
  }

  return median;
}


/*
  COMPUTE VOLUME OF THE WATER IN THE WELL
  The sensor is about 40 cm over the maximum water level, so we compensate
  by subtracting SENSOR_OFFSET_CM before converting to decimeters.
*/
double computeVolume(double distance) {
  if (distance <= 0) return -1;

  // outer well dimension: base diameter of 2 meters, 3 meters of height
  // inner well dimension
  double waterHeightDm = (distance - SENSOR_OFFSET_CM) / 10.0; // convert cm to dm
  if (waterHeightDm < 0) waterHeightDm = 0;

  double radius = WELL_DIAMETER_DM / 2.0;
  double base   = radius * radius * 3.14159; // area = π × r²
  return base * waterHeightDm;
}


/*
  Returns the current Unix timestamp from the Yun's Linux side.
  The AR9331 Linux SoC syncs time via NTP automatically, so this value
  is reliable after the first boot that has internet access.
*/
String getArduinoTimestamp() {
  Process p;
  p.runShellCommand("date +%s");
  String ts = "";
  while (p.available() > 0) ts += (char)p.read();
  ts.trim();
  return ts;
}


/*
  SENDING DATA TO SERVER
  Sends distance, volume and the Arduino NTP timestamp via HTTPS POST to add.php.
  Retries up to MAX_RETRIES times on failure; checks HTTP response code.
  Returns true on success (HTTP 201), false if all retries fail.
*/
bool sendData(double distance, double volume) {
  String ts = getArduinoTimestamp();

  for (int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
    Process p; // Create a process and call it "p"
    p.runShellCommand(
      "curl -s -o /dev/null -w \"%{http_code}\" -X POST " + SERVER_URL +
      " -F token=" + API_TOKEN +
      " -F dist=" + String((int)distance) +
      " -F vol=" + String(volume) +
      " -F ts=" + ts
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

  // set up the SONAR
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT); // Use LED indicator (if required)
}


void loop() {
  double distance = measureDistance();
  Serial.println("Distance: " + String(distance) + " cm");

  double volume = computeVolume(distance);
  Serial.println("Volume: " + String(volume) + " dm3");

  bool ok = sendData(distance, volume);
  // LED on if all retries failed, off on success
  digitalWrite(LED_PIN, ok ? LOW : HIGH);

  // Delay 24 h before next reading.
  delay(86400000UL);
}

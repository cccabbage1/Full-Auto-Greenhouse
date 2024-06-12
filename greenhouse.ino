//========= LCD and sensor Libraries =============
#include <Adafruit_SCD30.h>

Adafruit_SCD30  scd30;    // Create an instance of sensor

// Display libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// Variables to store CO, Temp, Humidity
float co2;
float temp;
float humid;

// Adjustable parameters
float maxCo2 = 700;       // Maximum CO2 value to activate relay 1
float minHum = 60;        // Minimum humidity value to activate relay 2

// === Defining Relay pins and relay logic ====
int relay1 = 15;           // Relay 2 Pin
int relay2 = 13;           // Relay 1 Pin
bool relayLogic = HIGH;    // Relay Operating Logic change this with LOW if your relay works on LOW logic

bool relay1Flag = LOW;    // Flag to check if relay is on or off
bool relay2Flag = LOW;    // Flag to check if relay is on or off

// ==== Disply function format ======
// A function is created to display message on oled the format of function is like this
// displayMessage(text size, x position, y position, text to be displayed, HIGH/LOW to clear display);

// Buttons configuration
// OLED FeatherWing buttons map to different pins depending on board:

#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2

void setup(void) {
  Serial.begin(115200);

  // Initially set relay pins as OUTPUT and set them off
  pinMode(relay1, OUTPUT);                // Set relay pin as output
  pinMode(relay2, OUTPUT);                // Set relay pin as output
  digitalWrite(relay1, !relayLogic);      // Initially turn off relay
  digitalWrite(relay2, !relayLogic);      // Initially turn off relay

  // Initialize Button pins as input with internal pullup
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  Serial.println("SCD30 OLED CO2 meter!");


  // Try to initialize!
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 chip");
    // while (1) {
    //   delay(10);
    // }
  }
  Serial.println("SCD30 Found!");


  Serial.println("128x64 OLED FeatherWing test");
  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  Serial.println("OLED begun");


  if (!scd30.setMeasurementInterval(4)) {           // Set sensor reading interval as 4 seconds
    Serial.println("Failed to set measurement interval");
    // while (1) {
    //   delay(10);
    // }
  }
  Serial.print("Measurement Interval: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" seconds");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  display.setRotation(1);
  delay(500); // Pause for half second

  display.setTextColor(SH110X_WHITE);
  displayMessage(1, 0, 5, "CO2:" + String(co2) + " ppm ", HIGH);  // Show error message on display
  displayMessage(1, 0, 20, "T:" + String(temp) + " C H:" + String(humid) + " % ", LOW);  // Show error message on display

}


void loop() {
  if (scd30.dataReady()) {    // Check if sensor is ready to give new reading

    Serial.println("Data available!");

    if (!scd30.read()) {
      Serial.println("Error reading sensor data");
      displayMessage(2, 0, 0, "READ ERR", HIGH);      // Show error message on display
      return;
    }
    co2 = scd30.CO2;                // Read CO2 from sensor
    temp = scd30.temperature;       // Read Temperature from sensor
    humid = scd30.relative_humidity;// Read Humidity from sensor

    String msg = "CO2: " + String(co2) + " ppm  Temperature:" + String(temp) + " C  Humidity:" + String(humid) + " %";
    Serial.println(msg);

    displayMessage(1, 0, 5, "CO2:" + String(co2) + " ppm ", HIGH);  // Show error message on display
    displayMessage(1, 0, 20, "T:" + String(temp) + " C H:" + String(humid) + " % ", LOW);  // Show error message on display

    // Check CO2 and Humidity condition to operate relays
    if ((co2 > maxCo2) && (relay1Flag == LOW)) {
      // Check if CO2 is greater than max co2 value than turn on relay 1 and previously relay is off
      relay1Flag = HIGH;        // Set relay flag
      digitalWrite(relay1, relayLogic);      // Turn on relay
    }
    else if ((co2 < maxCo2) && (relay1Flag == HIGH)) {
      // Check if CO2 is less than max co2 value than and previously relay is on turn off relay 1
      relay1Flag = LOW;        // ReSet relay flag
      digitalWrite(relay1, !relayLogic);      // Turn off relay
    }

    // Check Humidity condition to operate relay
    if ((humid < minHum) && (relay2Flag == LOW)) {
      // Check if Humidity is less than min humidity value than turn on relay 1 and previously relay is off
      relay2Flag = HIGH;        // Set relay flag
      digitalWrite(relay2, relayLogic);      // Turn on relay
    }
    else if ((humid > minHum) && (relay2Flag == HIGH)) {
      // Check if Humidity is greaer than min humidity relay value than and previously relay is on turn off relay 1
      relay2Flag = LOW;        // ReSet relay flag
      digitalWrite(relay2, !relayLogic);      // Turn off relay
    }
  }
  delay(100);
  // Take to Menu if button A is pressed
  if (!digitalRead(BUTTON_A)) {
    delay(1000);  // Wait for one second if button is still pressed than change to menu
    if (!digitalRead(BUTTON_A)) {
      while (!digitalRead(BUTTON_A)) {
        ; // Wait until button released
      }
      displayMenu();    // Display menu
    }
  }
}

void displayMenu() {
  // This function will display menu to adjust values of variables
  int count = 0;        // This will be incremented by one each time Button A is pressed
  displayMessage(10, 0, 0, "", HIGH);  // Show  message on display
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  displayMessage(1, 0, 0, "*", LOW); // Show  message on display

  while (count < 2) {
    if (!digitalRead(BUTTON_A)) {
      while (!digitalRead(BUTTON_A)) {
        ; // Wait until button released
      }
      // Increment count with 1
      count++;
      displayMessage(1, 0, 0, " ", LOW); // Show  message on display
      displayMessage(1, 0, 10, "*", LOW); // Show  message on display

    }
    if ((count == 0) && (!digitalRead(BUTTON_B))) {
      // If current count is 0 and button B is pressed increase CO2 Value by 5
      maxCo2 += 5;
      delay(100);   // Wait for 100 milliseconds
    }
    else if ((count == 0) && (!digitalRead(BUTTON_C))) {
      // If current count is 0 and button C is pressed decrease CO2 Value by 5
      maxCo2 -= 5;
      delay(100);   // Wait for 100 milliseconds
    }
    if ((count == 1) && (!digitalRead(BUTTON_B))) {
      // If current count is 0 and button B is pressed increase Humidity Value by 1
      minHum += 1;
      if (minHum > 100) {
        minHum = 100;    // if humidity is greater than 100 keep it 100
      }
      delay(100);   // Wait for 100 milliseconds
    }
    else if ((count == 1) && (!digitalRead(BUTTON_C))) {
      // If current count is 0 and button C is pressed decrease Humidity Value by 1
      minHum -= 1;
      if (minHum < 0) {
        minHum = 0;    // if humidity is less than 0 keep it 0
      }
      delay(100);   // Wait for 100 milliseconds
    }
    displayMessage(1, 8, 0, "CO2:" + String(maxCo2) + " ppm ", LOW); // Show  message on display
    displayMessage(1, 8, 10, "Hum:" + String(minHum) + " % ", LOW); // Show  message on display
    delay(50);
  }
  displayMessage(1, 0, 0, "" , HIGH); // Clear Display

}

void displayMessage(int textSize, int x, int y, String msg, bool cls) {
  // This function will display msg received on oled
  if (cls) {
    // If clear display flag is high first clear display
    display.clearDisplay();   // Clear Display
    display.display();        // Transfer the change to display
  }
  display.setCursor(x, y);  // Set cursor at this point
  display.setTextSize(textSize);   // Set Text Size
  display.print(msg);           // Print msg on lcd
  display.display();        // Show change on display
}

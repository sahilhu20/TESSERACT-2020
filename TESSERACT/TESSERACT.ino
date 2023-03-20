//#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <DHT_U.h>
#include <DHT.h>
#include <MQ135.h>

// defines

#define dataPin 2
#define DHTType DHT22
#define ANALOGPIN 0

// Global variables

int charCount = 0, loop_run = 0;
// how many passes are required for preloading MQ135 (recommended 60)
const int loadTime = 1; // <-- 1 to load instantly

//SoftwareSerial s(5,6);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
DHT dht = DHT(dataPin, DHTType);
MQ135 gasSensor = MQ135(ANALOGPIN);

// Initialize the special characters
byte charS[] = {
  B01111,
  B10000,
  B10000,
  B01110,
  B00001,
  B11110,
  B00000,
  B11111
};

byte charI[] = {
  B01110,
  B00100,
  B00100,
  B00100,
  B00100,
  B01110,
  B00000,
  B11111
};

byte charL[] = {
  B10000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11111,
  B00000,
  B11111
};

byte charC[] = {
  B01110,
  B10001,
  B10000,
  B10000,
  B10001,
  B01110,
  B00000,
  B11111
};

byte charO[] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000,
  B11111
};

byte charN[] = {
  B10001,
  B11001,
  B10101,
  B10101,
  B10011,
  B10001,
  B00000,
  B11111
};

byte charPlug[] = {
  B00000,
  B00100,
  B00110,
  B01010,
  B01001,
  B10001,
  B10001,
  B01110
};


// Main functions

void setup() {
  //s.begin(9600);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  // set up the LCD
  lcd.begin(20, 4);
  lcd.noDisplay();
  delay(100);
  lcd.display();
  
  printWelcomeScreen();
  delay(2500);
  lcd.clear();
  printCredits();
  delay(2000);
  digitalWrite(LED_BUILTIN, HIGH); 
  lcd.clear();
  printSpecialChar(charPlug, 19, 3);
}

void loop() {
  if (loop_run == 0) {
    lcd.home();
    lcd.print("Initializing Sensors");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
    
  delay(2000); // wait for init
  loop_run++;
  digitalWrite(LED_BUILTIN, HIGH); // blink each pass
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float rzero = gasSensor.getRZero();
  float ppm = gasSensor.getPPM();
  
  lcd.home();
  lcd.print("Temperature: ");
  lcd.print(t);
  lcd.print((char) 223);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.print("%");

  int warn = 0;
  lcd.setCursor(0, 2);
  lcd.print("Air Quality: ");
  if (loop_run < loadTime) {
    lcd.print("WAITING");
  } else {
    warn = showAirStr(ppm);
  }

  lcd.setCursor(0, 3);
  if (loop_run < loadTime) {
    lcd.print("SENSOR LOADING ");
    lcd.print((loadTime - loop_run) * 2);
    lcd.print(" ");
    return; // wait until sensor has finished loading
  }
  
  if (warn == 0) {
    lcd.print("Gases within limits ");
  } else if (warn == 1) {
    lcd.print("Gases outside limits");
  } else if (warn == 2) {
    lcd.print("WARNING TOXIC LEVELS");
  } else if (warn == -1) {
    lcd.print("SENSOR ERROR!");
    lcd.print(ppm);
  }
  
  Serial.println(ppm); // for debugging
  // Check if ESP8266 is responding
  /*float sData[] = { h, t, ppm };
  if(s.available() >= 3) {
    for (int i = 0; i < 3; i++) {
      s.write(sData[i]);
    }
  }*/
}

// Other functions

/*
 * return
 *  0 Levels within health limits
 *  1 Toxic concenteration levels
 *  -1 Unknown level detected
 * Values adjusted for an uncalibrated 24hrs preheated MQ135 sensor for Kolkata,
 * India. Expect incorrect outputs for other places (esp. outdoors). Smoke and
 * high gas concentrations near the sensor will still trigger some of the right
 * values however.
 */
int showAirStr(float mp) {
  if (mp < 10.00) {
    lcd.print("HEALTHY");
  } else if (mp >= 15.00 && mp < 25.00) {
    lcd.print("NORMAL ");
  } else if (mp >= 25.00 && mp < 35.00) {
    lcd.print("STUFFED");
  } else if (mp >= 35.00 && mp < 50.00) {
    lcd.print("UNSAFE ");
    return 1;
  } else if (mp > 50.00) {
    lcd.print("HAZARD ");
    return 2;
  } else {
    lcd.print("UNKNOWN");
    return -1;
  }
  return 0;
}

// Print functions

/*
 * Prints a special character on the 20x4 at the given row and column.
 * Please note that only a limited amount of special characters are
 * allowed and this function does not check for those.
 */
void printSpecialChar(byte character[], int localeR, int localeC) {
  lcd.createChar(charCount, character);
  lcd.setCursor(localeR, localeC);
  lcd.write(byte(charCount));
  charCount++;
}

void printWelcomeScreen() {
  lcd.home();
  lcd.print("      WELCOME!      ");
  lcd.setCursor(0, 1);
  lcd.print("  IoT Project for   ");
  lcd.setCursor(0, 2);
  lcd.print("Enviornment and Air ");
  lcd.setCursor(0, 3);
  lcd.print("  Quality Analysis  ");
}

void printCredits() {
  lcd.home();
  printSpcTeamName();
  lcd.setCursor(0, 1);
  lcd.print("Syed and Sahil ");
  lcd.setCursor(0, 3);
  lcd.print("      PRESENT      ");
}

void printSpcTeamName() {
  lcd.home();
  lcd.print("*Team ");
  printSpecialChar(charS, 6, 0);
  printSpecialChar(charI, 7, 0);
  printSpecialChar(charL, 8, 0);
  printSpecialChar(charI, 9, 0);
  printSpecialChar(charC, 10, 0);
  printSpecialChar(charO, 11, 0);
  printSpecialChar(charN, 12, 0);
  lcd.print(" Stomp*");
}

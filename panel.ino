/*
 * Affiche une horloge binaire
 * Ou une matrice de pixels
 */
#include <TimeLib.h>

#include <gfxfont.h>
#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <math.h>


// -------------------- RGB Matrix --------------------
#define CLK 11
#define LAT 10
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);


// -------------------- STATE --------------------
long lastStateUpdatedAt = 0;
int state = -1;
#define STATE_OK 0
#define STATE_ERROR -1
#define STATE_BUSY 99
#define STATE_HIDE 9999

// -------------------- TIME AND DAY --------------------
#define OFFSET_HOUR_1 6
#define OFFSET_HOUR_2 10
#define OFFSET_MINUTE_1 15
#define OFFSET_MINUTE_2 19
#define OFFSET_SECOND_1 24
#define OFFSET_SECOND_2 28
#define WIDTH_HOUR 4
#define WIDTH_DAY 2
#define OFFSET_DAY_1 0
#define OFFSET_DAY_2 3

int previousHour = 0; //< Pour éviter de redessiner ce qui ne change pas.
int previousMinute = 0;
int previousSecond = 0;
int previousDay = 0;


#define BLUETOOTH_PIN 12
#define COLOR_YELLOW matrix.Color333(1, 1, 0)
#define COLOR_ORANGE matrix.Color333(1, 1, 0)
#define COLOR_BLUE matrix.Color333(0, 0, 7)
#define COLOR_LBLUE matrix.Color333(1, 1, 2)
#define COLOR_WHITE matrix.Color333(1, 1, 1)
#define COLOR_GREEN matrix.Color333(0, 1, 0)
#define COLOR_RED matrix.Color333(1, 0, 0)

#define TZ -14400

#define BUTTON_PIN 8

// App config
#define DEFAULT_TIME 1548608895
#define END_TOKEN ';'
#define COMMAND_END_TOKEN '>'
#define COMMAND_TIME_SET "T"
#define COMMAND_COLOR_SET "C"

// Variables
String stringBuffer = String();
bool buttonWasDown = false;
int color = COLOR_BLUE;


void setup() {
    matrix.begin();
    matrix.fillScreen(1);  // clear screen
    long tm = 0;
    setTime(tm);
    matrix.begin();
    pinMode(BLUETOOTH_PIN, OUTPUT);
    digitalWrite(BLUETOOTH_PIN, HIGH);
    delay(500); // attend que le module bluetooth demarre
    Serial.begin(115200);
    Serial.setTimeout(50); // Pour éviter de bloquer la boucle plus que 50ms;
    pinMode(BUTTON_PIN, INPUT);
    setTime(1548630709);
}

void loop() {
  drawTime();
  readSerial();
  showState();
  matrix.swapBuffers(false);
  
  if (digitalRead(BUTTON_PIN) == LOW) buttonWasDown = true;
  if (digitalRead(BUTTON_PIN) == HIGH && buttonWasDown)  {
      buttonWasDown = false;
      delay(50);
      color += 30;
      if (color >= 1536)
        color = -1535;
      redraw();
  }
}

/**
 * Effectue la lecture du serial, délimité par des ;
 */
void readSerial() {
  if (Serial.available()) {
    setState(STATE_BUSY);
    String chunk = Serial.readString();
    stringBuffer += chunk;
    const int endPos = stringBuffer.indexOf(END_TOKEN);
    const int commandEndPos = stringBuffer.indexOf(COMMAND_END_TOKEN);
    if (endPos != -1) {
      const String packet = stringBuffer.substring(0, endPos);
      handleCommand(packet.substring(0, commandEndPos), packet.substring(commandEndPos+1));
      stringBuffer = String();
    }
    setState(STATE_OK);
  }
}


/**
 * Gère le comandes.
 * Une commande est du format :  COMMANDE>VALEUR;
 * 
 */
bool handleCommand(String code, String value) {
  if (code.compareTo(COMMAND_TIME_SET) == 0) {
    Serial.write("COMMAND_TIME_SET>");
    commandSetTime(value);
  } else if (code.compareTo(COMMAND_COLOR_SET) == 0) {
    Serial.write("COMMAND_COLOR_SET>");
    commandSetColor(value);
  } else {
    while (Serial.available())
      Serial.read();
    return false;
  }
  Serial.write(";\n");
  return true;
}

void commandSetTime(String value) {
  char* ptr;
  setTime(strtol(value.c_str(), &ptr, 10) + TZ);
  redraw();
  Serial.print("OK");
}

void commandSetColor(String value) {
  char *ptr;
  const int r = value[0] - '0';
  const int g = value[1] - '0';
  const int b = value[2] - '0';
  color = matrix.Color333(r, g, b);
  redraw();
  Serial.print("OK");
}

// ---------------------------------------- TIME  ----------------------------------------

void redraw() {
    previousHour = 0;
    previousMinute = 0;
    previousSecond = 0;
    drawTime();
    drawDay();
}

void drawTime() {
  char str[3];
  if (hour() != previousHour) {
    sprintf(str, "%02d", hour());
    matrix.fillRect(OFFSET_HOUR_1, 0, 8, 16, 0);
    drawColumn(str[0], OFFSET_HOUR_1, WIDTH_HOUR);
    matrix.fillRect(OFFSET_HOUR_2, 0, 4, 16, 0);
    drawColumn(str[1], OFFSET_HOUR_2, WIDTH_HOUR);
    previousHour = hour();
  }
  if (minute() != previousMinute) {
    sprintf(str, "%02d", minute());
    matrix.fillRect(OFFSET_MINUTE_1, 0, 4, 16, 0);
    drawColumn(str[0], OFFSET_MINUTE_1, WIDTH_HOUR);
    matrix.fillRect(OFFSET_MINUTE_2, 0, 4, 16, 0);
    drawColumn(str[1], OFFSET_MINUTE_2, WIDTH_HOUR);
    previousMinute = minute();
  }
  if (second() != previousSecond) {
    sprintf(str, "%02d", second());
    matrix.fillRect(OFFSET_SECOND_1, 0, 4, 16, 0);
    drawColumn(str[0], OFFSET_SECOND_1, WIDTH_HOUR);
    matrix.fillRect(OFFSET_SECOND_2, 0, 4, 16, 0);
    drawColumn(str[1], OFFSET_SECOND_2, WIDTH_HOUR);
    previousSecond = second();
  }
  if (day() != previousDay) {
    drawDay();
    previousDay = day();
  }
}

void drawColumn(char c, int position, int width) {
  switch (c) {
    case '1': matrix.fillRect(position, 16-4, width, 4, color); break;
    case '2': matrix.fillRect(position, 16-8, width, 4, color); break;
    case '3': matrix.fillRect(position, 16-8, width, 8, color); break;
    case '4': matrix.fillRect(position, 16-12, width, 4, color); break;
    case '5': matrix.fillRect(position, 16-12, width, 4, color); matrix.fillRect(position, 16-4, width, 4, color); break;
    case '6': matrix.fillRect(position, 16-12, width, 4, color); matrix.fillRect(position, 16-8, width, 4, color); break;
    case '7': matrix.fillRect(position, 16-12, width, 4, color); matrix.fillRect(position, 16-8, width, 8, color); break;
    case '8': matrix.fillRect(position, 16-16, width, 4, color); break;
    case '9': matrix.fillRect(position, 16-16, width, 4, color); matrix.fillRect(position, 16-4, width, 4, color); break;
    default:  matrix.fillRect(position, 0, width, 4, 0); break;
  }
}

void drawDay() {
  matrix.fillRect(0, 0, 6, 16, 0);
  char str[3];
  sprintf(str, "%02d", day());
  drawColumn(str[0], OFFSET_DAY_1, WIDTH_DAY);
  drawColumn(str[1], OFFSET_DAY_2, WIDTH_DAY);
}


// ---------------------------------------- STATE ----------------------------------------

void setState(int newState) {
  lastStateUpdatedAt = millis();
  state = newState;
  showState();
}


void showState() {
  switch(state) {
    case STATE_OK:
      matrix.drawPixel(0, 0, COLOR_GREEN);
      break;
    case STATE_ERROR:
      matrix.drawPixel(0, 0, COLOR_RED);
      break;
    case STATE_BUSY:
      matrix.drawPixel(0, 0, COLOR_ORANGE);
      break;
    default:
      matrix.drawPixel(0, 0, 0);
      break;
  }
  if (lastStateUpdatedAt+5000 < millis()) {
    state = STATE_HIDE;
  }
}

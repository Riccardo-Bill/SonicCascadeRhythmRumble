#include <Wire.h>
#include <FastLED.h>
#include <LiquidCrystal_I2C.h>
#include <HttpClient.h> // For making the API calls

LiquidCrystal_I2C lcdDisplay(0x20,16,2);
#define LED_STRIPS 5
#define STRIP_ROWS 9
#define STRIP_COLUMNS 5
#define TOTAL_COLUMNS 25
#define LEDS_PER_STRIP 45
HttpClient httpClient;

int bpmPin = A0;
int bpmReading = 0;
int bpmValue = 80;

int moodPin = A1;
int moodReading = 0;
int moodValue = 5;

String chordData = "1F,1A,1B$1F#,1B,1C$1E,1F,1C#";
const char* noteMapping[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
const int MAX_ACTIVE_KEYS = 15;

int apiButtonPin = 8;
int parseButtonPin = 7;
HttpClient httpClient;

struct ActiveKey {
    int column;
    int row;
    unsigned long lastUpdate;
    unsigned long interval;
};

ActiveKey activeKeys[MAX_ACTIVE_KEYS];
int activeKeysCount = 0;
unsigned long keyInterval = 1000;

CRGB ledArray[LED_STRIPS][LEDS_PER_STRIP];
CRGB colorArray[TOTAL_COLUMNS];

void setup() { 
    pinMode(apiButtonPin, INPUT);
    pinMode(parseButtonPin, INPUT);

    Serial.begin(9600);
    lcdDisplay.init();
    lcdDisplay.clear();
    lcdDisplay.setCursor(1, 0);
    lcdDisplay.scrollDisplayLeft();

    lcdDisplay.backlight();
    lcdDisplay.print("Welcome to Sonics");
    lcdDisplay.setCursor(1, 1);
    lcdDisplay.print("Cascades RR!");
    delay(2000);
    lcdDisplay.clear();
    lcdDisplay.print("Enjoy the show!!");
  
    for (int i = 0; i < MAX_ACTIVE_KEYS; i++) {
        activeKeys[i].column = -1;
    }

    FastLED.addLeds<NEOPIXEL, 9>(ledArray[0], LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, 10>(ledArray[1], LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, 11>(ledArray[2], LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, 12>(ledArray[3], LEDS_PER_STRIP);
    FastLED.addLeds<NEOPIXEL, 13>(ledArray[4], LEDS_PER_STRIP);

    for (int i = 0; i < TOTAL_COLUMNS; i++) {
        colorArray[i] = CHSV(i * (255 / (TOTAL_COLUMNS - 1)), 255, 255);
    }

    delay(2000);
    char sampleChords[] = "1F,1A,1B;1F#,1B,1C;1E,1F,1C#";
    parseAndActivate(sampleChords);
}

void updateBPM(int bpmReading) {
    float normalizedReading = bpmReading / 1023.00;
    int maxBPM = 120;
    int minBPM = 50;
    bpmValue = 50 + normalizedReading * (maxBPM - minBPM);
}

void updateMood(int moodReading) {
    float normalizedReading = moodReading / 1023.00;
    moodValue = normalizedReading * 10;
}

int calculateLEDIndex(int row, int col, int stripIndex) {
    int index;
    if (stripIndex == 0) {
        col = STRIP_COLUMNS - 1 - col;
    }
    if (col % 2 == 0) {
        index = col * STRIP_ROWS + row;
    } else {
        index = (col + 1) * STRIP_ROWS - 1 - row;
    }
    return index;
}

void controlLED(int row, int column) {
    int stripIndex = column / STRIP_COLUMNS;
    int stripColumn = column % STRIP_COLUMNS;
    int index = calculateLEDIndex(row, stripColumn, stripIndex);
    ledArray[stripIndex][index] = colorArray[column];
    FastLED.show();
}

void deactivateLED(int row, int column) {
    int stripIndex = column / STRIP_COLUMNS;
    int stripColumn = column % STRIP_COLUMNS;
    int index = calculateLEDIndex(row, stripColumn, stripIndex);
    ledArray[stripIndex][index] = CRGB::Black;
    FastLED.show();
}

int noteToColumn(const char* note, int octave) {
    int noteIndex = -1;
    for (int i = 0; i < 12; i++) {
        if (strcmp(noteMapping[i], note) == 0){
            noteIndex = i;
            break;
        }
    }
    if (noteIndex == -1) {
        return -1;
    }
    int column = 12 * (octave - 1) + noteIndex;
    return column;
}

void activateKey(int column) {
    for (int i = 0; i < MAX_ACTIVE_KEYS; i++) {
        if (activeKeys[i].column == -1) {
            activeKeys[i].column = column;
            activeKeys[i].row = 0;
            activeKeys[i].lastUpdate = millis();
            activeKeys[i].interval = keyInterval;
            activeKeysCount++;
            break;
        }
    }
}

void parseAndActivate(const char* chords) {
    char* chord = strtok(chords, ";");
    while(chord != NULL) {
        char* note = strtok(chord, ",");
        while(note != NULL) {
            int octave = note[0] - '0';
            char noteName[4];
            strcpy(noteName, note + 1);
            int column = noteToColumn(noteName, octave);
            activateKey(column);
            note = strtok(NULL, ",");
        }
        chord = strtok(NULL, ";");
    }
}

void getChordProgression() {
  // Define the URL for the API request
  char server[] = "http://example.com/api/chordProgression";
  httpClient.get(server);

  // Read the status code and body of the response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  // Check the status code for a successful response
  if (statusCode == 200) {
    // The request was successful
    // Store the response to chordData
    response.toCharArray(chordData, response.length() + 1);
  } else {
    // The request failed
    Serial.print("GET request failed with status code ");
    Serial.println(statusCode);
  }
}


void loop() {
    unsigned long currentMillis = millis();
    for (int i = 0; i < MAX_ACTIVE_KEYS; i++) {
        if (activeKeys[i].column != -1 && currentMillis - activeKeys[i].lastUpdate >= activeKeys[i].interval) {
            deactivateLED(activeKeys[i].row, activeKeys[i].column);
            activeKeys[i].row++;
            if (activeKeys[i].row >= STRIP_ROWS) {
                activeKeys[i].column = -1;
                activeKeysCount--;
            } else {
                controlLED(activeKeys[i].row, activeKeys[i].column);
                activeKeys[i].lastUpdate = currentMillis;
            }
        }
    }
    // Make an API call and save chord progression when button on digital pin 8 is pressed
    if (digitalRead(BUTTON_PIN_API_CALL) == HIGH) {
        getChordProgression();
    }
    // Parse and activate when button on digital pin 7 is pressed
    if (digitalRead(BUTTON_PIN_PARSE_ACTIVATE) == HIGH) {
        parseAndActivate(chordData);
    }
    // Update bpm and mood
    int bpmRead = analogRead(BPM_PIN);
    updateBPM(bpmRead);
    int moodRead = analogRead(MOOD_PIN);
    updateMood(moodRead);
    // Update key interval with bpm value
    keyInterval = 60000 / bpmValue;
    delay(1000);
}

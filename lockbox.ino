#include <LiquidCrystal.h>
#include <RotaryEncoder.h>


#define RED_1 9
#define RED_2 8
#define RED_3 7
#define RED_4 6
#define GREEN_1 5
#define GREEN_2 4
#define GREEN_3 3
#define GREEN_4 2

#define LCD_RS A0
#define LCD_E A1
#define LCD_D4 A2
#define LCD_D5 A3
#define LCD_D6 A4
#define LCD_D7 A5

#define ENC_BUT 13
#define ENC_LEFT 12
#define ENC_RIGHT 11

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
RotaryEncoder encoder(ENC_RIGHT, ENC_LEFT, RotaryEncoder::LatchMode::FOUR0); 

void resetLEDs() {
  digitalWrite(RED_1, LOW);
  digitalWrite(RED_2, LOW);
  digitalWrite(RED_3, LOW);
  digitalWrite(RED_4, LOW);
  digitalWrite(GREEN_1, LOW);
  digitalWrite(GREEN_2, LOW);
  digitalWrite(GREEN_3, LOW);
  digitalWrite(GREEN_4, LOW);
}

void setup() {
  pinMode(RED_1, OUTPUT);
  pinMode(RED_2, OUTPUT);
  pinMode(RED_3, OUTPUT);
  pinMode(RED_4, OUTPUT);
  pinMode(GREEN_1, OUTPUT);
  pinMode(GREEN_2, OUTPUT);
  pinMode(GREEN_3, OUTPUT);
  pinMode(GREEN_4, OUTPUT);
  
  Serial.begin(9600);
  pinMode(ENC_BUT, INPUT_PULLUP);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(1, 0);
  lcd.print("SAFE CRACKERS");
  lcd.setCursor(0, 1);
  lcd.print("Press the Button");
}

enum GameState {RESET, NUM_SELECT, GUESS};
GameState currState = RESET;

int guess[4];
int code[4];
int guessRemaining = 10;
int currNum = 0;
RotaryEncoder::Direction currDir = RotaryEncoder::Direction::NOROTATION;
bool lastButton = false;
bool currButton = false;

void gameReset() {
  resetLEDs();
  //lock servo here
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Enter Code");
  currNum = 0;
  guess[0] = 0;
  guess[1] = 0;
  guess[2] = 0;
  guess[3] = 0;
  guessRemaining = 10;
  randomSeed(analogRead(0));
  code[0] = random(9);
  code[1] = random(9);
  code[2] = random(9);
  code[3] = random(9);
  currState = NUM_SELECT;
}

void selectNums() {
  encoder.tick();
  currDir = encoder.getDirection();
  switch(currDir) {
    // if turning clockwise, don't go over 9, instead set to 0. otherwise add 1
    // same with turning ccw but with not going below 0
    case RotaryEncoder::Direction::CLOCKWISE: guess[currNum] = guess[currNum] == 9 ? 0 : guess[currNum] + 1; break;
    case RotaryEncoder::Direction::COUNTERCLOCKWISE: guess[currNum] = guess[currNum] == 0 ? 9 : guess[currNum] - 1; break;
  }
  lcd.setCursor(6, 1);
  for( int i = 0; i < 4; i++ ) {
    lcd.print(guess[i]);  
  }
  // input next number if encoder button is pressed
  if (!lastButton && currButton) {
    if (currNum < 3) currNum++;
    else currState = GUESS; 
  }
}

int red[] = {RED_1, RED_2, RED_3, RED_4};
int green[] = {GREEN_1, GREEN_2, GREEN_3, GREEN_4};
void updateLEDs() {
  resetLEDs();
  int bulls = 0, cows = 0;
  bool checkedNums[4] = {false};
  for (int i = 0; i < 4; i++) {
    if (code[i] == guess[i]) bulls++;
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (guess[i] == code[j] && !checkedNums[j]) {
        cows++; 
        checkedNums[j] = true;
        break;
      }
    }
  }
  cows = cows - bulls;
  for (int i = 0; i < cows; i++) {
    digitalWrite(red[i], HIGH);
  }
  for (int i = 0; i < bulls; i++) {
    digitalWrite(green[i], HIGH);
  }
}

void makeGuess() {
  if (guessRemaining == 0) currState = RESET;
  else if (guess[0] == code[0] && guess[1] == code[1] && guess[2] == code[2] && guess[3] == code[3]) {
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Correct Code");
    currState = RESET;
    delay(2000);
  } else {
    currState = NUM_SELECT;
    currNum = 0;
    updateLEDs();
    lcd.setCursor(1,0);
    lcd.print("Incorrect Code");
    lcd.setCursor(1,1);
    lcd.print(guessRemaining);
    lcd.print(" guesses left");
    guessRemaining--;
    delay(2000);
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print("Enter Code");
  }
}

void loop() {
  currButton = digitalRead(ENC_BUT) == LOW;
  switch(currState) {
    case RESET: if (currButton) gameReset(); break;
    case NUM_SELECT: selectNums(); break;
    case GUESS: makeGuess(); break;
  }
  lastButton = currButton;
}

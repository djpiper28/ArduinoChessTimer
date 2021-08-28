#include <HID.h>
#include <LiquidCrystal.h>

// LCD pins <--> Arduino pins
const int RS = 11, EN = 12, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

#define POLL_TIME 43
#define TURN_SWAP_PIN 7
#define WHITE_TIMER_PIN 9
#define BLACK_TIMER_PIN 8

#define DEFAULT_TIME 10l * 60l * 1000l

#define STATE_LOADING -1
#define STATE_TIMER 0

struct timer_settings
{
  unsigned long timeLimit;
  unsigned long incrementFromMove;
  unsigned long timeIncrement;
};

struct timer_state
{
  struct timer_settings settings;
  bool paused;
  bool whitesTurn;
  unsigned long turnStartTime;
  //^ Used with current time to calculate time
  // differences for updating the time remaining
  long whiteTimeLeft, blackTimeLeft;
};

struct chess_timer_state
{
  int state;
  struct timer_state chess_timer_state;
};

struct chess_timer_state state = {
  STATE_LOADING,
  {
    {
      DEFAULT_TIME,
      0,
      0
    },
    false,
    true,
    millis(),
    DEFAULT_TIME,
    DEFAULT_TIME
  }
};

void lightOn()
{
  digitalWrite(LED_BUILTIN, HIGH);
}

void dlightOn(int x)
{
  digitalWrite(x, HIGH);
}

void lightOff()
{
  digitalWrite(LED_BUILTIN, LOW);
}

void dlightOff(int x)
{
  digitalWrite(x, LOW);
}

bool pinSwapDown;
void setup()
{
  // init light
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WHITE_TIMER_PIN, OUTPUT);
  pinMode(BLACK_TIMER_PIN, OUTPUT);
  pinMode(TURN_SWAP_PIN, INPUT);
  lightOn();

  lcd.begin(16, 2); // set up number of columns and rows
  lcd.print("Loading..."); // print the custom char at (2, 0)

  pinSwapDown = false;
}

void printTimer()
{
  lcd.home();
  unsigned long whiteMins = (state.chess_timer_state.whiteTimeLeft / 1000 / 60) % 60;
  unsigned long whiteSeconds = (state.chess_timer_state.whiteTimeLeft / 1000) % 60;
  unsigned long whiteMilliSeconds = (state.chess_timer_state.whiteTimeLeft % 1000) / 10;

  unsigned long blackMins = (state.chess_timer_state.blackTimeLeft / 1000 / 60) % 60;
  unsigned long blackSeconds = (state.chess_timer_state.blackTimeLeft / 1000) % 60;
  unsigned long blackMilliSeconds = (state.chess_timer_state.blackTimeLeft % 1000) / 10;

  if (whiteMins / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(whiteMins, 10);

  lcd.print(":");
  if (whiteSeconds / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(whiteSeconds, 10);

  lcd.print(":");
  if (whiteMilliSeconds / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(whiteMilliSeconds, 10);

  // draw black timer
  lcd.setCursor(8, 1);

  if (blackMins / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(blackMins, 10);

  lcd.print(":");
  if (blackSeconds / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(blackSeconds, 10);
  lcd.print(":");
  if (blackMilliSeconds / 10 == 0)
  {
    lcd.print("0");
  }
  lcd.print(blackMilliSeconds, 10);
}

void loop()
{
  if (state.state == STATE_LOADING)
  {
    lcd.clear();
    lcd.print("Loaded.");
    state.state = STATE_TIMER;
  }
  else if (state.state == STATE_TIMER)
  {
    if (state.chess_timer_state.whiteTimeLeft <= 0 || state.chess_timer_state.blackTimeLeft <= 0)
    {
      state.chess_timer_state.paused = true;
      if (state.chess_timer_state.whiteTimeLeft < 0)
      {
        state.chess_timer_state.whiteTimeLeft = 0;
      }
      else if (state.chess_timer_state.blackTimeLeft < 0)
      {
        state.chess_timer_state.blackTimeLeft = 0;
      }
      if (millis() % 1000 > 500)
      {
        dlightOff(WHITE_TIMER_PIN);
        dlightOff(BLACK_TIMER_PIN);
      }
      else
      {
        if (state.chess_timer_state.whitesTurn)
        {
          dlightOn(WHITE_TIMER_PIN);
          dlightOff(BLACK_TIMER_PIN);
        }
        else
        {
          dlightOn(BLACK_TIMER_PIN);
          dlightOff(WHITE_TIMER_PIN);
        }
      }
    }
    else
    {
      if (state.chess_timer_state.whitesTurn)
      {
        dlightOn(WHITE_TIMER_PIN);
        dlightOff(BLACK_TIMER_PIN);
      }
      else
      {
        dlightOn(BLACK_TIMER_PIN);
        dlightOff(WHITE_TIMER_PIN);
      }

      unsigned long timeDiff = millis() - state.chess_timer_state.turnStartTime;

      if (timeDiff >= POLL_TIME && !state.chess_timer_state.paused)
      {
        if (state.chess_timer_state.whitesTurn)
        {
          state.chess_timer_state.whiteTimeLeft -= POLL_TIME;
          state.chess_timer_state.turnStartTime = millis();
        }
        else
        {
          state.chess_timer_state.blackTimeLeft -= POLL_TIME;
          state.chess_timer_state.turnStartTime = millis();
        }
      }

      int swapTurn = digitalRead(TURN_SWAP_PIN);
      if (swapTurn == HIGH)
      {
        if (!pinSwapDown)
        {
          pinSwapDown = true;
          state.chess_timer_state.whitesTurn = !state.chess_timer_state.whitesTurn;
          lightOn();
        }
      }
      else if (swapTurn == LOW)
      {
        pinSwapDown = false;
      }
    }
    printTimer();
  }

  if (millis() % 1000 > 500)
  {
    lightOff();
  }
  else
  {
    lightOn();
  }
}

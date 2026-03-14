const int porch = 5;
const int hallway = 8;
const int button = 11;

void setupPins();
void checkButton();
void turnPorch();
void turnHallway();

void setup()
{
  setupPins();
}

void loop()
{
  checkButton();
}

void setupPins()
{
  pinMode(porch, OUTPUT);
  pinMode(hallway, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  digitalWrite(porch, LOW);
  digitalWrite(hallway, LOW);
}

void checkButton()
{
  if (digitalRead(button) == LOW)
  {
    turnPorch();
    turnHallway();
    delay(500);
  }
}

void turnPorch()
{
  digitalWrite(porch, HIGH);
  delay(30000);
  digitalWrite(porch, LOW);
}

void turnHallway()
{
  digitalWrite(hallway, HIGH);
  delay(60000);
  digitalWrite(hallway, LOW);
}
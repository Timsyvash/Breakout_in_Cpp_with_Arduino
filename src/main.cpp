#include <Adafruit_NeoPixel.h>

// --- НАЛАШТУВАННЯ ГРИ ---
#define LED_PIN 12   // Пін підключення стрічки NeoPixel
#define NUM_LEDS 64  // Кількість світлодіодів
#define JOY_X_PIN A0 // Вісь X (рух ракетки)
#define JOY_Y_PIN A1 // Вісь Y (підкручування м'яча)

#define PADDLE_SIZE 3 // Розмір ракетки

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Змінні м'яча
float ballPos = 1.0;
float ballSpeed = 0.12; // Початкова швидкість
short ballDir = 1;      // 1 - до ракетки, -1 - до стіни

// Змінні ракетки
float paddlePos = 0;      // float для плавного руху
float paddleSpeed = 0.25; // Швидкість руху ракетки від джойстика
short minPaddleZone;

unsigned long previousMillis = 0;
unsigned long previousMillis_fill = 0;
unsigned long previousMillis_clear = 0;
const short updateInterval = 25; // Кадрова частота (мс)

// Малювання кадру
void renderGame(short joyY)
{
  strip.clear();

  // Колір ракетки залежить від осі Y
  uint32_t paddleColor = strip.Color(0, 0, 255); // Синя за замовчуванням
  if (joyY < 300)
  {
    paddleColor = strip.Color(255, 0, 255); // Пурпурна
  }
  else if (joyY > 700)
  {
    paddleColor = strip.Color(0, 255, 255); // Блакитна
  }

  // 1. Спочатку малюємо ракетку
  short intPaddlePos = (short)paddlePos;
  for (int i = 0; i < PADDLE_SIZE; i++)
  {
    short pixelIndex = intPaddlePos + i;
    if (pixelIndex >= 0 && pixelIndex < NUM_LEDS)
    {
      strip.setPixelColor(pixelIndex, paddleColor);
    }
  }

  // 2. Потім малюємо м'яч
  short displayBallPos = constrain((short)ballPos, 0, NUM_LEDS - 1);
  if (ballDir == 1)
  {
    strip.setPixelColor(displayBallPos, strip.Color(255, 0, 0)); // Червоний (на нас)
  }
  else
  {
    strip.setPixelColor(displayBallPos, strip.Color(0, 255, 0)); // Зелений (від нас)
  }

  strip.show();
}

void resetGame()
{
  ballPos = 1.0;
  ballSpeed = 0.12;
  ballDir = 1;
  paddlePos = NUM_LEDS - PADDLE_SIZE - 2;
}

void gameOverEffect()
{
  for (int i = 0; i < 3; i++)
  {
    if (millis() - previousMillis_fill >= 200)
    {
      previousMillis_fill = millis();
      strip.fill(strip.Color(255, 0, 0));
      strip.show();
    }

    if (millis() - previousMillis_clear >= 400)
    {
      previousMillis_clear = millis();
      strip.clear();
      strip.show();
    }
  }
}

void setup()
{
  strip.begin();
  strip.setBrightness(50);
  strip.show();

  minPaddleZone = NUM_LEDS - 8; // Зона руху ракетки
  if (minPaddleZone < 0)
    minPaddleZone = 0;

  paddlePos = NUM_LEDS - PADDLE_SIZE - 2;
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= updateInterval)
  {
    previousMillis = currentMillis;

    // 1. Зчитування джойстика
    short joyX = analogRead(JOY_X_PIN);
    short joyY = analogRead(JOY_Y_PIN);

    // 2. Рух ракетки
    if (joyX < 400)
    {
      paddlePos -= paddleSpeed;
    }
    else if (joyX > 600)
    {
      paddlePos += paddleSpeed;
    }

    // Обмеження руху ракетки
    if (paddlePos < minPaddleZone)
      paddlePos = minPaddleZone;
    if (paddlePos > NUM_LEDS - PADDLE_SIZE)
      paddlePos = NUM_LEDS - PADDLE_SIZE;

    // 3. Рух м'яча
    ballPos += ballSpeed * ballDir;

    // Відбивання від лівої стіни (початок стрічки)
    if (ballPos <= 0)
    {
      ballPos = 0;
      ballDir = 1;
    }

    // 4. Перевірка зіткнення з ракеткою
    short intPaddlePos = (short)paddlePos;
    if (ballDir == 1 && ballPos >= intPaddlePos)
    {
      // Якщо м'яч торкнувся переднього краю або перебуває в межах довжини ракетки
      if (ballPos < intPaddlePos + PADDLE_SIZE)
      {
        ballDir = -1;                 // Відбиваємо назад
        ballPos = intPaddlePos - 1.0; // ВІДКИДАЄМО М'ЯЧ НА ПІКСЕЛЬ ПЕРЕД РАКЕТКОЮ

        // Логіка підкручування по осі Y
        if (joyY < 300)
        {
          ballSpeed += 0.05;
        }
        else if (joyY > 700)
        {
          ballSpeed -= 0.03;
          if (ballSpeed < 0.08)
            ballSpeed = 0.08;
        }
        else
        {
          ballSpeed += 0.015;
        }
      }
      else
      {
        // М'яч повністю пролетить повз ракетку (промах)
        gameOverEffect();
        resetGame();
      }
    }

    // 5. Оновлення екрана
    renderGame(joyY);
  }
}
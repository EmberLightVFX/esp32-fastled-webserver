#include <FastLED.h>

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DATA_PIN 2
// #define CLK_PIN   4
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 52
CRGB leds[NUM_LEDS];

#define MILLI_AMPS 1500 // IMPORTANT: set the max milli-Amps of your power supply (4A = 4000mA)
#define FRAMES_PER_SECOND 120

CRGB solidColor = CRGB::Blue;

uint8_t randomPattern = 1;
uint8_t randomPalette = 1;
uint8_t power = 1;
uint8_t speed = 30;
uint8_t brightness = 100;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t currentPatternIndex = 0;
uint8_t currentPaletteIndex = 0;

uint8_t autoplay = 1;
uint8_t autoplayDuration = 50;
unsigned long autoPlayTimeout = 0;

uint8_t cyclePalettes = 1;
uint8_t paletteDuration = 12;
unsigned long paletteTimeout = 0;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 120;

#include "patterns.h"

void ledSetup()
{
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection(UncorrectedColor)
        .setDither(true);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS); // 1A
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void nextPattern()
{
    if (randomPattern == 1)
    {
        currentPatternIndex = random(0, patternCount) % patternCount;
    }
    else
    {
        currentPatternIndex = (currentPatternIndex + 1) % patternCount;
    }
}

void nextPalette()
{
    if (randomPalette == 1)
    {
        currentPaletteIndex = random(0, paletteCount) % paletteCount;
    }
    else
    {
        currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
    }
    targetPalette = palettes[currentPaletteIndex];
}

void ledLoop()
{
    if (power == 0)
    {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    else
    {
        // Call the current pattern function once, updating the 'leds' array
        patterns[currentPatternIndex].pattern();

        EVERY_N_MILLISECONDS(40)
        {
            // slowly blend the current palette to the next
            nblendPaletteTowardPalette(currentPalette, targetPalette, 20);
            gHue++; // slowly cycle the "base color" through the rainbow
        }

        if (autoplay == 1 && (millis() > autoPlayTimeout))
        {
            nextPattern();
            autoPlayTimeout = millis() + (autoplayDuration * 1000);
        }

        if (cyclePalettes == 1 && (millis() > paletteTimeout))
        {
            nextPalette();
            paletteTimeout = millis() + (paletteDuration * 1000);
        }
    }

    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);
}
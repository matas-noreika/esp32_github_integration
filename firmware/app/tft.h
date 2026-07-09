/*
 * Programmer: Matas Noreika
 * Date: 2026-07-04
 * Purpose:
 * Header API to seperate TFT screen configuration from main app source code.
*/

#pragma once

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 135
#define TFT_MOSI     35
#define TFT_SCLK     36
#define TFT_MISO     37  // Not physically used by screen but assigned to SPI hardware
#define TFT_CS        7
#define TFT_DC       39
#define TFT_RST      40
#define TFT_BL       45  // Backlight pin
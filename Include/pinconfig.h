#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_

#ifdef PCB

// Buttons
#define BUTTONTOPRIGHT 4
#define BUTTONTOPLEFT 26
#define BUTTONBOTTOMLEFT 25
#define BUTTONBOTTOMRIGHT 10

// General SPI
#define SPIMOSI 22
#define SPICLK 24

// Epaper Display
#define EPAPERRST 12
#define EPAPERDC 14
#define EPAPERCS 15
#define EPAPERBUSY 8

#endif

#ifdef TESTING

// Buttons
#define BUTTONTOPRIGHT 4
#define BUTTONTOPLEFT 26
#define BUTTONBOTTOMLEFT 25
#define BUTTONBOTTOMRIGHT 10

// General SPI
#define SPIMOSI 22
#define SPICLK 24

// Epaper Display
#define EPAPERRST 12
#define EPAPERDC 14
#define EPAPERCS 15
#define EPAPERBUSY 8

#endif

#endif  // if_PIN_CONFIG_H_

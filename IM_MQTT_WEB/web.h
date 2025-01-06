//web.h

#ifndef WEB_H
#define WEB_H
#include <Arduino.h>

extern String html;

String SendHTML(float temperature, float humidity, int lightValue, String kondisi_kipas, String kondisi_sprayer, String kondisi_lampu);

#endif

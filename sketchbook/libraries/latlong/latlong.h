#ifndef _LATLONG_H
#define _LATLONG_H
#include <Arduino.h>
#include <bstrlib.h>
#include <constable.h>
#include <conshell.h>

typedef struct {
	// Latitude and longitude are in degrees
	// The Earth is about 2^25 m across at the equator, and floats give us 2^24
	// bits of precision, so our worst-case error is ~5m.
	// Note that with Arduino, doubles are the same as floats.
	float lat;
	float lon;
} position;

// Returns the distance between two points.
float distance(position* p1, position* p2);

// Returns the index of the nearest point in the array
uint8_t nearest(position* boat, position* p[], uint8_t count);

// Returns the index of the furthest point in the array
uint8_t furthest(position* boat, position* p[], uint8_t count);

// Prints the latitude and longitude of a point to the console
void printPosition(position* p);

// calls the above functions for testing
int latlongtest(blist list);

// Returns 1 if the coordinates for p are valid
// (i.e. -180 <= lat <= 180, -90 <= lon <= 90)
uint8_t verifyPosition(position* p);
#endif

#include "latlong.h"

// All distances are in metres
// The Earth is treated as a sphere
const float pi = 3.141592653589793;
const float earthRadius = 6371000;

float distanceRad(float lat1, float lon1, float lat2, float lon2) {
	// Uses the Haversine formula
	// https://en.wikipedia.org/wiki/Haversine_formula

	float a = pow(sin((lat2 - lat2)/2),2) +
		cos(lat1) * cos(lat2) * pow(sin((lon2 - lon1)/2),2);
	float c = 2 * atan2(sqrt(a), sqrt(1-a));
	return earthRadius * c;
}

float distance(position* p1, position* p2) {
	return distanceRad(
		p1->lat * (pi/180),
		p1->lon * (pi/180),
		p2->lat * (pi/180),
		p2->lon * (pi/180)
		);
}

uint8_t nearest(position* boat, position* p[], uint8_t count) {
	if(count < 1) return -1;
	
	float boatLat = boat->lat * (pi/180);
	float boatLon = boat->lon * (pi/180);

	float nearestDistance = earthRadius * 2 * pi;
	uint8_t nearestIndex;
	for(uint8_t i = 0; i < count; i++) {
		float pLat = p[i]->lat * (pi/180);
		float pLon = p[i]->lon * (pi/180);

		float distance = distanceRad(boatLat, boatLon, pLat, pLon);
		if(distance < nearestDistance) {
			nearestDistance = distance;
			nearestIndex = i;
		}
	}
	return nearestIndex;
}

uint8_t furthest(position* boat, position* p[], uint8_t count) {
	if(count < 1) return -1;
	
	float boatLat = boat->lat * (pi/180);
	float boatLon = boat->lon * (pi/180);

	float furthestDistance = -1;
	uint8_t furthestIndex;
	for(uint8_t i = 0; i < count; i++) {
		float pLat = p[i]->lat * (pi/180);
		float pLon = p[i]->lon * (pi/180);

		float distance = distanceRad(boatLat, boatLon, pLat, pLon);
		if(distance > furthestDistance) {
			furthestDistance = distance;
			furthestIndex = i;
		}
	}
	return furthestIndex;
}

void printPosition(position* p) {
	Serial.print(abs(p->lat));
	Serial.print(" ");
	Serial.print( p->lat > 0 ? "N" : "S" );
	Serial.print(", ");
	Serial.print(abs(p->lon));
	Serial.print(" ");
	Serial.print( p->lon > 0 ? "E" : "W" );
	Serial.println();
}

int latlongtest(blist list) {
   	position b;
	position p[3];

	// Kingston
	b.lat = 44.233333;
	b.lon = -76.5;
	
	// Toronto
	p[0].lat = 43.7;
	p[0].lon = -79.4;

	// Montreal
	p[1].lat = 45.5;
	p[1].lon = -73.566667;

	// London, UK
	p[2].lat = 51.5072222;
	p[2].lon = -0.1275;

	Serial.print("Kingston: ");
	printPosition(&b);
	Serial.print("Toronto: ");
	printPosition(&p[0]);
	Serial.print("Montreal: ");
	printPosition(&p[1]);
	Serial.print("London, UK: ");
	printPosition(&p[2]);
	
	Serial.print("Distance between Kingston and Toronto: ");
	Serial.print(distance(&b, &p[0]));
	Serial.println(" m");
	Serial.print("Distance between Kingston and Montreal: ");
	Serial.print(distance(&b, &p[1]));
	Serial.println(" m");
	Serial.print("Distance between Kingston and London (UK): ");
	Serial.print(distance(&b, &p[2]));
	Serial.println(" m");

	return 0;
}

uint8_t verifyPosition(position* p) {
	if( abs(p->lat) > 180.0 || abs(p->lon) > 90 ) return 0;
	else return 1;
}

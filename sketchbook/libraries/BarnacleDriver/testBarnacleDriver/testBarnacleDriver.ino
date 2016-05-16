/*	Program to test functionality of the BarnacleDriver class.
*	
*/

#include <SoftwareSerial.h>
#include <BarnacleDriver.h>

SoftwareSerial barnacleSerial(10,11);
BarnacleDriver *barnacle_client = new BarnacleDriver(barnacleSerial);

void setup() {
  Serial.begin(57600);
  barnacleSerial.begin(19200);
}

void loop() { // run over and over
	
	uint16_t result = 0;
	Serial.print("\nBattery voltage: ");
	result = barnacle_client->barn_get_battery_voltage();
	printResult(result);
	
	Serial.print("\nBatteru current: ");
	result = barnacle_client->barn_get_battery_current();
	printResult(result);
	
	Serial.print("\nCharger voltage: ");
	result = barnacle_client->barn_get_charger_voltage();
	printResult(result);
	
	Serial.print("\nCharger current: ");
	result = barnacle_client->barn_get_charger_current();
	printResult(result);
	
	Serial.print("\nW1 ticks: ");
	result = barnacle_client->barn_get_w1_ticks();
	printResult(result);
	
	Serial.print("\nW1 clear: ");
	result = barnacle_client->barn_clr_w1_ticks();
	printResult(result);
	
	Serial.print("\nW2 ticks: ");
	result = barnacle_client->barn_get_w2_ticks();
	printResult(result);
	
	Serial.print("\nW2 clear: ");
	result = barnacle_client->barn_clr_w2_ticks();
	printResult(result);
	
	Serial.print("\nGet and clear W1 ticks: ");
	result = barnacle_client->barn_getandclr_w1_ticks();
	printResult(result);
	
	Serial.print("\nGet and clear W2 ticks: ");
	result = barnacle_client->barn_getandclr_w2_ticks();
	printResult(result);
	
	Serial.print("\nBarnacle latency: ");
	Serial.println(barnacle_client->barn_check_latency());
	
	Serial.print("\nBarnacle data summary: ");
	Serial.print(barnacle_client->barn_get_data_summary());
	
	while (1); //end test program
}

void printResult(uint16_t result){
	Serial.print(result);
}
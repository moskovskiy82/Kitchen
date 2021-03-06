/* http://forum.mysensors.org/topic/994/multi-button-relay-switch/43


*/


#include <MySensor.h>
#include <SPI.h>
#include <Bounce2.h>
#define RELAY_ON 1 //switch around for realy HIGH/LOW state
#define RELAY_OFF 0
//
MySensor gw;

#define noRelays 4
const int relayPin[] = {7, 8, A0, A1};
const int buttonPin[] = {3, 4, 5, 6};

class Relay							// relay class, store all relevant data (equivalent to struct)
{
public:                             		 
  int buttonPin;					// physical pin number of button
  int relayPin;						// physical pin number of relay
  byte oldValue;               		// last Values for key (debounce)
  boolean relayState;             	// relay status (also stored in EEPROM)
};

Relay Relays[noRelays];	
Bounce debouncer[noRelays];
MyMessage msg[noRelays];

void setup(){
	gw.begin(incomingMessage, AUTO, true);
	delay(250);
	gw.sendSketchInfo("MultiRelayButton", "0.9b");
	delay(250);
	// initialize Relays with corresponding buttons
	for (int i = 0; i < noRelays; i++){
		Relays[i].buttonPin = buttonPin[i];				// assign physical pins
		Relays[i].relayPin = relayPin[i];
		msg[i].sensor = i;								// initialize messages
		msg[i].type = V_LIGHT;
		debouncer[i] = Bounce();						// initialize debouncer
		debouncer[i].attach(buttonPin[i]);
		debouncer[i].interval(5);
		pinMode(Relays[i].buttonPin, INPUT_PULLUP);
	//	pinMode(Relays[i].relayPin, OUTPUT);
		Relays[i].relayState = gw.loadState(i);			// retrieve last values from EEPROM
		digitalWrite(Relays[i].relayPin, Relays[i].relayState? RELAY_ON:RELAY_OFF); // and set relays accordingly
		gw.send(msg[i].set(Relays[i].relayState? true : false), true);	// make controller aware of last status
		gw.present(i, S_LIGHT);							// present sensor to gateway
		delay(250);
	}
}

void loop()
	{
	gw.process();
	for (byte i = 0; i < noRelays; i++)
		{
		debouncer[i].update();
		byte value = debouncer[i].read();
		if (value != Relays[i].oldValue && value == 0)
			{
			Relays[i].relayState = !Relays[i].relayState;
			digitalWrite(Relays[i].relayPin, Relays[i].relayState);
			gw.send(msg[i].set(Relays[i].relayState? true : false), true);
			gw.saveState( i, Relays[i].relayState ); // save sensor state in EEPROM (location == sensor number)
			}
		Relays[i].oldValue = value;
		}
	}

// process incoming message 
void incomingMessage(const MyMessage &message)
{
	if (message.isAck()){
		Serial.println(F("This is an ack from gateway"));
	}
	if (message.type == V_LIGHT){ 
		if (message.sensor <= noRelays){ 				// check if message is valid for relays
			Relays[message.sensor].relayState = message.getBool(); 
			digitalWrite(Relays[message.sensor].relayPin, Relays[message.sensor].relayState? RELAY_ON:RELAY_OFF); // and set relays accordingly
			gw.saveState( message.sensor, Relays[message.sensor].relayState ); // save sensor state in EEPROM (location == sensor number)
		}
	}
}


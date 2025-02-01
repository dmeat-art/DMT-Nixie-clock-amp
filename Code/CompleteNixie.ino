/*
Nixie Clock using an Arduino UNO R4 Wifi and the Arduinix Shield
By Dmitri "D-Meat" Engman
More info : https://github.com/dmeat-art/DMT-Nixie-clock-amp/


Wifi Example code credits :
	created 13 July 2010
	by dlf (Metodo2 srl)
	modified 31 May 2012
	by Tom Igoe
*/
#include <WiFiS3.h>

#include <EEPROM.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

char ApSsid[] = "Nixie_AP";        // your network SSID (name)
char ApPass[] = "123456789";    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiServer server(80);

String	submittedSsid = "";
String	submittedPassword = "";
bool 	gotNewSsidPass = false;

// EEPROM Variables
int i = 0;
int statusCode;
String st;
String content;
String esid;
String epass = "";


typedef enum {
	CLOCK,
	SETUPWIFI,
	CONNECTWIFI,
	GETTIME
} currentMode_t;

currentMode_t currentMode;

// Pins Setup :
//______________________________________________________
// Button :
int buttonPin = A0;

// SN74141 (1)
int ledPin_0_a = 2;
int ledPin_0_b = 3;
int ledPin_0_c = 4;
int ledPin_0_d = 5;
// SN74141 (2)
int ledPin_1_a = 6;                
int ledPin_1_b = 7;
int ledPin_1_c = 8;
int ledPin_1_d = 9;

// anod pins - actually inverted from the printed numbers
int ledPin_a_1 = 13;
int ledPin_a_2 = 12;
int ledPin_a_3 = 11;
int ledPin_a_4 = 10;


// Clock Variables :
//______________________________________________________
// Temps d'illumination de chaque tube
// 5ms semble bien, tester en dessous à l'occasion
#define tubeRefresh 	  	3 //in ms
// #define dateDuration		2 //in seconds // removed because we use a switch in displayTime
#define randomTableSize		32

short int digits[4];
short int lamps[4];

unsigned long secs;
int currentSec = 0;
int secondsOffset = 0;
bool updateMemory = false;

byte  month, day, hours, minutes;
bool isWinter;
short year;
short tempDigits[4]; // delete later
short tempNum;

short int currentYear[4];
short int currentMonth[2];
short int currentDay[2];
short int currentTime[4]; // hh:mm

short int randomDigits[randomTableSize];
short int randomTableIndex = 0;

short monthsDuration[13] = {
	0,  // unused
	31, // jan
	28, // fev
	31, // mar
	30, // avr
	31, // may
	30, // jun
	31, // jul
	31, // aug
	30, // sep
	31, // oct
	30, // nov
	31  // dec
};

void generateRandoms ()
{
	int i, j;
	short int temp = -1;
	randomSeed(analogRead(0));
	for (i = 0; i < randomTableSize; i++)
	{
		temp = random(0,10);
		
		if (i > 0) {
			while (temp == randomDigits[i-1])
			{
				temp = random(0, 10);
			}
		} else if (i == randomTableSize)
		{
			while (temp == randomDigits[0])
			{
				temp = random(0, 10);
			}
		}
		randomDigits[i] = temp;
	}
	
	// debug
	Serial.println("Random Digits table :");
	for (j = 0; j < randomTableSize/8; j++)
	{
		for (i = 0; i < 8; i++)
		{
			temp = (j*8) + i;
			Serial.print(randomDigits[temp]);
			Serial.print(", ");
		}
		Serial.println();
	}
}

void initiateTimeVariables() {
	// Default time
	year = 		2018;
	month = 	1;
	day = 		1;
	hours = 	12;
	minutes = 	00;
	
	
	// Init variables
	updateTime();
	
	currentMonth[1] = month % 10;
	currentMonth[0] = (month - currentMonth[1]) /10;
	currentDay[1] = day % 10;
	currentDay[0] = (day - currentDay[1]) /10;
	
	currentYear[0] = year / 1000;
	currentYear[1] = (year / 100) % 10;
	currentYear[2] = (year / 10) % 10;
	currentYear[3] = year % 10;
	
	tempNum = 1337;
	tempDigits[0] = 0;
	tempDigits[1] = 0;
	tempDigits[2] = 0;
	tempDigits[3] = 0;
	
	// Génération du tableau des random
	// pour les transitions et l'entretien des tubes
	generateRandoms();
}

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};



// EEPROM :
//______________________________________________________

void readEEPROM() {
	Serial.println("Reading EEPROM ssid");

	for (int i = 0; i < 32; ++i)
	{
	esid += char(EEPROM.read(i));
	}
	Serial.print("SSID: ");
	Serial.println(esid);
	Serial.println("Reading EEPROM pass");

	for (int i = 32; i < 96; ++i)
	{
	epass += char(EEPROM.read(i));
	}
	Serial.print("PASS: ");
	Serial.println(epass);
}
// Network :
//______________________________________________________
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// setupAccessPoint :
//______________________________________________________
void setupAccessPoint() {
		// check for the WiFi module:
		if (WiFi.status() == WL_NO_MODULE) {
		Serial.println("Communication with WiFi module failed!");
		// don't continue
		while (true);
		}

		String fv = WiFi.firmwareVersion();
		if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
		Serial.println("Please upgrade the firmware");
		}

		// by default the local IP address will be 192.168.4.1
		// you can override it with the following:
		WiFi.config(IPAddress(192,48,56,2));

		// print the network name (SSID);
		Serial.print("Creating access point named: ");
		Serial.println(ApSsid);

		// Create open network. Change this line if you want to create an WEP network:
		status = WiFi.beginAP(ApSsid, ApPass);
		if (status != WL_AP_LISTENING) {
		Serial.println("Creating access point failed");
		// don't continue
		while (true);
		}

		// wait 5 seconds for connection:
		delay(5000);

		// start the web server on port 80
		server.begin();

		// you're connected now, so print out the status
		printWiFiStatus();
}

// Arduino Setup :
//______________________________________________________
void setup() {
	//Initialize serial and wait for port to open:
	Serial.begin(9600);
	while (!Serial) {
	; // wait for serial port to connect. Needed for native USB port only
	}

	Serial.println("*************************** BOOTING THE CLOCK ******************************");
	// Read eeprom for ssid and pass
	readEEPROM();

	//if button is pressend on startup, start into Wifi Setup Mode
	if (analogRead(buttonPin) < 10) {
		Serial.println("Button pressed, starting Access Point Mode");
		currentMode = SETUPWIFI;
		setupAccessPoint();
	} else {
		Serial.println("Starting Wifi Client to get Current Time");
		currentMode = CONNECTWIFI;
	}

	
	//nixie ports setup
	pinMode(ledPin_0_a, OUTPUT);
	pinMode(ledPin_0_b, OUTPUT);
	pinMode(ledPin_0_c, OUTPUT);
	pinMode(ledPin_0_d, OUTPUT);

	pinMode(ledPin_1_a, OUTPUT);
	pinMode(ledPin_1_b, OUTPUT);
	pinMode(ledPin_1_c, OUTPUT);
	pinMode(ledPin_1_d, OUTPUT);

	pinMode(ledPin_a_1, OUTPUT);
	pinMode(ledPin_a_2, OUTPUT);
	pinMode(ledPin_a_3, OUTPUT);
	pinMode(ledPin_a_4, OUTPUT);
  
	initiateTimeVariables();

	// Get the time ---- move to SetupWifiClient ?
		// Initialize a NTPClient to get time
	  timeClient.begin();
	  // Set offset time in seconds to adjust for your timezone, for example:
	  // GMT +1 = 3600
	  // GMT +8 = 28800
	  // GMT -1 = -3600
	  // GMT 0 = 0
	  timeClient.setTimeOffset(3600);
}

// Time :
//______________________________________________________
void updateDisplay (void)
{
	currentTime[1] = hours % 10;
	currentTime[0] = (hours - currentTime[1]) / 10;
	currentTime[3] = minutes % 10;
	currentTime[2] = (minutes - currentTime[3]) / 10;
	
	currentMonth[1] = month % 10;
	currentMonth[0] = (month - currentMonth[1]) /10;
	currentDay[1] = day % 10;
	currentDay[0] = (day - currentDay[1]) /10;
	
	currentYear[0] = year / 1000;
	currentYear[1] = (year / 100) % 10;
	currentYear[2] = (year / 10) % 10;
	currentYear[3] = year % 10;
}

void updateTime ( void )
{
	if (minutes >= 60)
	{
		minutes = 0;
		hours ++;
	}
	
	if (hours >= 24)
	{
		hours = 0;
		day ++;
	}
	
	//debug
	Serial.print("updateTime : ");
	Serial.print(hours);
	Serial.print("h ");
	Serial.print(minutes);
	Serial.println("m");
	
	if (day > monthsDuration[month])
	{
		//29 fev pour les années bissextiles
		if ((month == 2) && (year % 4 == 0))
		{
			day ++;
		}
		else
		{
			day = 1;
			month ++;
		}

	}
	
	if (month > 12)
	{
		month = 1;
		year ++;
	}
	
  // Weekly Synchronization (Friday at 9am)
	if (timeClient.getDay() == 5) {
		if ((hours == 9) && (minutes == 0)) {
			currentMode = CONNECTWIFI;
		}
	}
	
	updateDisplay();
}

int ticker (unsigned long secs)
{
	if (secs == 0)
	{
		minutes ++;
		updateTime();
		// printTime();

		String weekDay = weekDays[timeClient.getDay()];
		Serial.print("Week Day: ");
		Serial.println(weekDay);
	}

	return secs;
}

void printTime () {
	timeClient.update();

	time_t epochTime = timeClient.getEpochTime();
	Serial.print("Epoch Time: ");
	Serial.println(epochTime);

	String formattedTime = timeClient.getFormattedTime();
	Serial.print("Formatted Time: ");
	Serial.println(formattedTime);  

	int currentHour = timeClient.getHours();
	Serial.print("Hour: ");
	Serial.println(currentHour);  

	int currentMinute = timeClient.getMinutes();
	Serial.print("Minutes: ");
	Serial.println(currentMinute); 

	int currentSecond = timeClient.getSeconds();
	Serial.print("Seconds: ");
	Serial.println(currentSecond);  

	String weekDay = weekDays[timeClient.getDay()];
	Serial.print("Week Day: ");
	Serial.println(weekDay);

	//Get a time structure
	struct tm *ptm = gmtime ((time_t *)&epochTime); 

	int monthDay = ptm->tm_mday;
	Serial.print("Month day: ");
	Serial.println(monthDay);

	int currentMonth = ptm->tm_mon+1;
	Serial.print("Month: ");
	Serial.println(currentMonth);

	String currentMonthName = months[currentMonth-1];
	Serial.print("Month name: ");
	Serial.println(currentMonthName);

	int currentYear = ptm->tm_year+1900;
	Serial.print("Year: ");
	Serial.println(currentYear);

	//Print complete date:
	String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
	Serial.print("Current date: ");
	Serial.println(currentDate);

	Serial.println("");
}


// Set Nixie Display :
//______________________________________________________
void setDisplay (unsigned long secs)
{
	short int number = 0;
	
	if (secs > 59) // workaround en attendant de comprendre pourquoi
	{				// ça reste bloqué sur Random
		secs = 0;
	}
	
	number = secs % 10;
	
	digits[0] = number;
	digits[1] = number;
	digits[2] = number;
	digits[3] = number;
	
	
	switch (secs)
	{
		case 0 :
		case 1 :
		// case 28 :
		// case 29 :
			digits[0] = currentYear[0];
			digits[1] = currentYear[1];
			digits[2] = currentYear[2];
			digits[3] = currentYear[3];
			break;
		case 2 :
		case 3 :
		// case 30 :
		// case 31 :
			digits[0] = currentMonth[0];
			digits[1] = currentMonth[1];
			digits[2] = currentDay[0];
			digits[3] = currentDay[1];
			break;
		// case 27 :
		case 59 :
			for (int i = 0; i<4; i++)
			{
				digits[i] = randomDigits[randomTableIndex];
				randomTableIndex++;
				if(randomTableIndex > randomTableSize)
				{
					randomTableIndex = 0;
				}
			}
			break;
		default :
			digits[0] = currentTime[0];
			digits[1] = currentTime[1];
			digits[2] = currentTime[2];
			digits[3] = currentTime[3];
			break;
	}
}

void setLamps (unsigned long secs, unsigned long tenths)
{  
	// loupiotes - 0=on, 1=off
	byte currentLamp;

	for (byte i = 0; i < 4; i++) 
	{ //reset lamps
		lamps[i] = -1;
	}
		
	// new style
	if (tenths < 4)
	{
		currentLamp = secs % 4;
		currentLamp += tenths;
		if (currentLamp >= 4)
		{
			currentLamp -= 4;
		}
	} else {
		currentLamp = secs % 4;
	}
	lamps[currentLamp] = 0;
}

void SetSN74141Chips( int num2, int num1 )
{
  int a,b,c,d;
  
  // set defaults.
  a=0;b=0;c=0;d=0; // will display a zero.
  // /!\ 0 = HIGH, 1 = LOW /!\
  
  // Load the a,b,c,d.. to send to the SN74141 IC (1)
  switch( num1 )
  {
    case 0: a=0;b=0;c=0;d=0;break;
    case 1: a=1;b=0;c=0;d=0;break;
    case 2: a=0;b=1;c=0;d=0;break;
    case 3: a=1;b=1;c=0;d=0;break;
    case 4: a=0;b=0;c=1;d=0;break;
    case 5: a=1;b=0;c=1;d=0;break;
    case 6: a=0;b=1;c=1;d=0;break;
    case 7: a=1;b=1;c=1;d=0;break;
    case 8: a=0;b=0;c=0;d=1;break;
    case 9: a=1;b=0;c=0;d=1;break;
    default: a=1;b=1;c=1;d=1;
    break;
  }
  
  // Write to output pins.
  digitalWrite(ledPin_0_d, d);
  digitalWrite(ledPin_0_c, c);
  digitalWrite(ledPin_0_b, b);
  digitalWrite(ledPin_0_a, a);

  // Load the a,b,c,d.. to send to the SN74141 IC (2)
  if (num2 == 0) num2 = 9;
  switch( num2 )
  {
    case 0: a=0;b=0;c=0;d=0;break;
    case 1: a=1;b=0;c=0;d=0;break;
    case 2: a=0;b=1;c=0;d=0;break;
    case 3: a=1;b=1;c=0;d=0;break;
    case 4: a=0;b=0;c=1;d=0;break;
    case 5: a=1;b=0;c=1;d=0;break;
    case 6: a=0;b=1;c=1;d=0;break;
    case 7: a=1;b=1;c=1;d=0;break;
    case 8: a=0;b=0;c=0;d=1;break;
    case 9: a=1;b=0;c=0;d=1;break;
    default: a=1;b=1;c=1;d=1;
    break;
  }
  
  // Write to output pins
  digitalWrite(ledPin_1_d, d);
  digitalWrite(ledPin_1_c, c);
  digitalWrite(ledPin_1_b, b);
  digitalWrite(ledPin_1_a, a);
}


// Start Access point / Get the SSID and Password :
//______________________________________________________
void startAccessPoint() {
	Serial.println("Access Point Web Server");

	while (gotNewSsidPass == false) {
		// compare the previous status to the current status
		if (status != WiFi.status()) {
			// it has changed update the variable
			status = WiFi.status();
			Serial.print("-------------- WiFi.status = ");
			Serial.println(WiFi.status());

			if (status == WL_AP_CONNECTED) {
				// a device has connected to the AP
				Serial.println("Device connected to AP");
			} else {
				// a device has disconnected from the AP, and we are back in listening mode
				Serial.println("Device disconnected from AP");
			}
		}
	
	WiFiClient client = server.available();   // listen for incoming clients

	if (client) {                             // if you get a client,
		Serial.println("new client");           // print a message out the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		
		while (client.connected()) {            // loop while the client's connected
			delayMicroseconds(10);                // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
			if (client.available()) {             // if there's bytes to read from the client,
				char c = client.read();             // read a byte, then
				Serial.write(c);                    // print it out to the serial monitor
				if (c == '\n') {                    // if the byte is a newline character

					// if the current line is blank, you got two newline characters in a row.
					// that's the end of the client HTTP request, so send a response:
					if (currentLine.length() == 0) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println();

						// the content of the HTTP response follows the header:
						client.print("<!DOCTYPE html><html>");
						client.print("<head>");
						client.print("<title>Nixie Clock Wifi Access Setup</title>");
						client.print("</head>");
						client.print("<p><h1>Nixie Clock Wifi Access Setup</h1><br></p>");
						client.print("<p>This clock accesses the internet to re-synchronize itself once a week, every monday at 9am.<br>Enter your Wifi network's name (SSID) and password so it can access it.</p>");
						client.print("<form>");
						client.print("<label for=\"ssid\">SSID : </label>");
						client.print("<input type=\"text\" id=\"ssid\" name =\"ssid\"><br>");
						client.print("<label for=\"password\">Password : </label>");
						client.print("<input type=\"text\" id=\"password\" name =\"password\"><br>");
						client.print("<input type=\"submit\" value=\"Submit\">");
						client.print("</form>");
						// client.print("<p style=\"font-size:7vw;\">Click <a href=\"/H\">here</a> turn the LED on<br></p>");
						// client.print("<p style=\"font-size:7vw;\">Click <a href=\"/L\">here</a> turn the LED off<br></p>");
						client.print("</html>");

						// The HTTP response ends with another blank line:
						client.println();
						// break out of the while loop:
						break;
					}
					else {      // if you got a newline, then clear currentLine:
						currentLine = "";
					}
				}
				else if (c != '\r') {    // if you got anything else but a carriage return character,
					currentLine += c;      // add it to the end of the currentLine
				}

				if (currentLine.endsWith("HTTP/1.1")) {
					submittedSsid = currentLine;
					submittedPassword = currentLine;

					// cut off the undesired characters
					submittedSsid.remove(0, 11);
					submittedSsid.remove(submittedSsid.indexOf("&"), submittedSsid.length());

					submittedPassword.remove(0, submittedPassword.indexOf("&")+10);
					submittedPassword.remove(submittedPassword.length()-9, submittedPassword.length());

					Serial.print("Submitted SSID : ");
					Serial.println(submittedSsid);
					Serial.print("Submitted Password : ");
					Serial.println(submittedPassword);
					Serial.print("");

					}
			}
		}
		
		// close the connection:
		client.stop();
		Serial.println("client disconnected");
		
		if (submittedSsid) {
			Serial.print("Submitted SSID : ");
			Serial.println(submittedSsid);
		}
		if (submittedPassword) {
			Serial.print("Submitted Password : ");
			Serial.println(submittedPassword);
		}
		
		// Write to EEPROM
		Serial.println("clearing eeprom");
		for (int i = 0; i < 96; ++i) {
			EEPROM.write(i, 0);
		}
		
		Serial.println("");
		Serial.println("writing eeprom ssid:");
		for (int i = 0; i < submittedSsid.length(); ++i)
		{
		  EEPROM.write(i, submittedSsid[i]);
		  Serial.print("Wrote: ");
		  Serial.println(submittedSsid[i]);
		}
		Serial.println("writing eeprom pass:");
		for (int i = 0; i < submittedPassword.length(); ++i)
		{
		  EEPROM.write(32 + i, submittedPassword[i]);
		  Serial.print("Wrote: ");
		  Serial.println(submittedPassword[i]);
		}
		Serial.print("");
		Serial.println("Wifi Info saved to EEPROM, switching to Client mode to connect and get the time");
		WiFi.disconnect();
		currentMode = CONNECTWIFI;
		gotNewSsidPass = true;
		}
	}
}


// Connect to the SSID saved in EEPROM and get the time :
//______________________________________________________
void connectAndGetTime() {
	// Read eeprom for ssid and pass
	readEEPROM();
	
	int ssidLength = esid.length() + 1;
	int passLength = epass.length() + 1;
	// Convert string to char
	const char* ssid; //[ssidLength]; // = esid.c_str();
	const char* password; //[ssidLength]; //= epass.c_str();
	
	ssid = esid.c_str();
	password = epass.c_str();
	
	Serial.print("Converted strings : ");
	Serial.print((String)ssid);
	Serial.print(", ");
	Serial.print((String)password);
	Serial.println(". - - - - - -");
	
	// Connect to Wi-Fi
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	delay(500);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	// Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT -1 = -3600
	// GMT 0 = 0
	timeClient.setTimeOffset(7200);
	// Future project : get the time offset via the webpage where the user submits the wifi info and save it to EEPROM.
	
	
	timeClient.update();
	// synchronize date and time
	time_t epochTime = timeClient.getEpochTime();
	//Get a time structure
	struct tm *ptm = gmtime ((time_t *)&epochTime); 
	
	printTime(); // Somehow this here helps to write correct values in the local time variables
	
	Serial.println("Local Time Variables :");
	// year :
	year = ptm->tm_year+1900;
	Serial.print("YEAR = ");
	Serial.println(year);
	// month :
	month = ptm->tm_mon+1;
	Serial.print("MONTH = ");
	Serial.println(month);
	// day : 
	day = ptm->tm_mday;
	Serial.print("DAY = ");
	Serial.println(day);

  // Are we in summer or in winter time ?
  // extremely dirty implementation - would need a better library to make this easier
  if ((month < 3) || (month > 10)) {
    isWinter = true;
    Serial.println("we are in winter time");
  }
  if (month == 3){
    // find out which day is the last sunday of march
    int temp =  day - timeClient.getDay(); // gives us the date of the previous sunday
    temp += 7;
    if (temp >= monthsDuration[3]) { 
     isWinter = false;
     Serial.println("we are in summer time");
    }
  }
  if (month == 10) {
    int temp =  day - timeClient.getDay(); // gives us the date of the previous sunday
    temp += 7;
    if (temp >= monthsDuration[10]) { 
     isWinter = true;
     Serial.println("we are in winter time");
    }
  }

	// hour :
	hours = timeClient.getHours();
  if (isWinter) hours -= 1;
	Serial.print("HOURS = ");
	Serial.println(hours);
	// minutes :
	minutes = timeClient.getMinutes();
	Serial.print("MINUTES = ");
	Serial.println(minutes);
	
	
	// updateDisplay();
	
	// seconds :
	secondsOffset = timeClient.getSeconds();
	secondsOffset % 60;
	// Serial.println(secondsOffset);
	// Serial.println(secs);
	Serial.print("true seconds offset : ");
	secondsOffset = secondsOffset - secs;
	Serial.println(secondsOffset);

	updateTime();
	
	Serial.println("Disconnecting WiFi");
	WiFi.disconnect();
	Serial.println("Getting in CLOCK mode");
	currentMode = CLOCK;
}

// Arduino Loop :
//______________________________________________________
void loop() {
	
unsigned long tenths;
unsigned long counter = 0;

secs = millis() / 1000;
secs %= 60;
tenths = millis() / 100;
tenths %= 10;

	switch (currentMode)
	{
		case SETUPWIFI :
		Serial.println("------------  SETUPWIFI ------------");
		// set Display code to SETUPWIFI mode
		SetSN74141Chips(9, 6);
		digitalWrite(ledPin_a_1, HIGH);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, LOW);
		//Start Access Point
		startAccessPoint();
		break;
		
		case CONNECTWIFI :
		Serial.println("------------  CONNECTWIFI ------------");
		// set Display code to CONNECTWIFI mode
		SetSN74141Chips(9, 4);
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, HIGH);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, LOW);
		// Activate Wifi Client and get the time
		connectAndGetTime();
		break;
		
		case GETTIME :
		// set Display code to GETTIME mode
		SetSN74141Chips(9, 2);
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, HIGH);
		digitalWrite(ledPin_a_4, LOW);
		break;
		
		case CLOCK :
		// Serial.println("------------  CLOCK ------------");
		
		/* SET DISPLAY */
		setDisplay(secs);
		setLamps(secs, tenths);

		/* TICKER */
		if (currentSec != secs)
		{
			currentSec = ticker(secs);
		}
		
		
		/* illumination des tubes */

		SetSN74141Chips(lamps[0],digits[0]);
		digitalWrite(ledPin_a_1, HIGH);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, LOW);
		delay(tubeRefresh);

		SetSN74141Chips(lamps[1],digits[1]);
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, HIGH);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, LOW);
		delay(tubeRefresh);

		SetSN74141Chips(lamps[2],digits[2]);
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, HIGH);
		digitalWrite(ledPin_a_4, LOW);
		delay(tubeRefresh);

		SetSN74141Chips(lamps[3],digits[3]);
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, HIGH);
		delay(tubeRefresh);

		// on éteint tout pour égaliser l'illumination
		digitalWrite(ledPin_a_1, LOW);
		digitalWrite(ledPin_a_2, LOW);
		digitalWrite(ledPin_a_3, LOW);
		digitalWrite(ledPin_a_4, LOW);
		break;
	}
}

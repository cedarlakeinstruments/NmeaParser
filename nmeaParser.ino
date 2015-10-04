#define SENTENCE_LEN 300
// Number of buoys
#define NUM_MESSAGES 25
#define MESSAGE_LEN  45
// Interval since last message came in
#define TRANSMIT_INTERVAL 1000 * 10 // Default is every 10 seconds
// Length of substrings
#define STRING_LEN 12
// Number of substrings
#define NUM_WORDS 16

enum offsets
{
    battery = 1,
    buoy = 3,
    latitude = 5,
    NS = 6,
    longitude = 7,
    EW = 8
};

// Message buffer
typedef struct tagRepeatMsg
{
    char msg[MESSAGE_LEN];
    char valid;
} REPEAT_MSG, *PREPEAT_MSG;

    // Storage for repeat messages
    REPEAT_MSG _msgs[NUM_MESSAGES];

    // Last transmit time
    unsigned long _lastTransmit = 0;

    void setup()
    {
        Serial.begin(4800);
	// Clear message buffer
	for (int i = 0; i < NUM_MESSAGES; i++)
	{
	    _msgs[i].valid = 0;
	    _msgs[i].msg[0] = 0;
	}
    }

    void loop()
    {
        // put your main code here, to run repeatedly:
	char nmeaStr[SENTENCE_LEN];
	char formattedStr[SENTENCE_LEN];

	// Read from device
	if (Serial.available())
	{
	    // Wait for rest of data
	    delay(50);
	    // Reads null-terminated serial data
	    Serial.readBytesUntil('\0', nmeaStr, SENTENCE_LEN);

	    parseNmea(nmeaStr, formattedStr);
	    // Send parsed data to output
	    if (formattedStr != NULL)
	    {
	        Serial.println(formattedStr);
		}
	    }
	    // Periodically send old messages
	    if (millis() > _lastTransmit + TRANSMIT_INTERVAL)
	    {
	        sendMessages();
	    }
	}

	// Separate string into tokens
	void split(char* inVal, char outVal[NUM_WORDS][STRING_LEN])
	{
	    int i = 0;
	    char *p = strtok(inVal, " ,/");
	    strcpy(&outVal[0][0], p);
	    while (p)
	    {
	        p = strtok(NULL, " ,/");
		if (p)
		{
		    strcpy(&outVal[++i][0], p);
		}
	    }
	}

	// Parse the NMEA sentence for the data we need
	void parseNmea(char* nmea, char* newStr)
	{
	    char words[NUM_WORDS][STRING_LEN];
	    split(nmea, words);
	    char tempstr[90];
	    // Convert battery voltage
	    char *end;
	    double batteryV = strtol(&words[battery][0], &end, 16);

	    sprintf(tempstr, "$RATLL,%d,%s,%s,%s,%s,B%3.1f,Q*", atoi(&words[buoy][0]), &words[latitude][0], &words[NS][0], &words[longitude][0], &words[EW][0], batteryV/10);
	    // Calculate checksum
	    unsigned char checksum = 0;
	    for (unsigned int j = 1; j < strlen(tempstr) - 1; j++)
	    {
	        // Checksum is XOR of all bytes past the initial $
		checksum ^= tempstr[j];
	    }
	    sprintf(newStr, "%s%02X", tempstr, checksum);
	    // Save this message for later
	    int num = atoi(words[buoy]);
	    if (num < NUM_MESSAGES)
	    {
	        // Buoy numbering is 1-based
		strncpy(_msgs[num - 1].msg, newStr, MESSAGE_LEN);
		_msgs[num - 1].valid = 1;
	    }
	}

	// Sends one message when called
	void sendMessages(void)
	{
	    static int i = 0;
	    for (i; i < NUM_MESSAGES; i++)
	    {
	        if (_msgs[i].valid)
		{
		    Serial.println(_msgs[i].msg);
		    _lastTransmit = millis();
		    i++;
		    break;
		}
	    }
	    if (i >= NUM_MESSAGES)
	    {
	    	i = 0;
	    }
	}

#define SENTENCE_LEN 300
// Number of buoys
#define NUM_MESSAGES 25
#define MESSAGE_LEN  45
// Interval since last message came in
#define TRANSMIT_INTERVAL 1000 * 10 // Default is every 10 seconds

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
    for (int i=0; i < NUM_MESSAGES; i++)
    {
        _msgs[i].valid = 0;
        _msgs[i].msg[0]= 0;
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  char nmeaStr[SENTENCE_LEN];
  char formattedStr[SENTENCE_LEN];
  
  // Read from device
  if (Serial.available() )
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

// Extract buoy# (chanca) from string
int buoyNum(char *nmea)
{
    int chanca = -1;
    // Extract the buoy #
    if (char *cPos = strstr(nmea, "chanca"))
    {
    	// Offset to chanca #
	cPos += 7;
	// Find the first '/' after that
	char *p = cPos;
	while (*p && *p != '/')
	{
            p++;
	}
	if (*p == '/')
	{
	    char chankaStr[5];
	    memset(chankaStr, NULL, 5);
	    memcpy(chankaStr, cPos, (p - cPos));
	    chanca = atoi(chankaStr);
	}
    }
    return chanca;
}

// Parse the NMEA sentence for the data we need
void parseNmea(char* nmea, char* newStr)
{
    int l = strlen(nmea);
    int commaCount = 0;
    int start = -1, end = -1;
    // Find the lat & long values
    for (int i = 0; i < l; i++)
    {
    	if (nmea[i] == ',')
	{
            commaCount++;
	    // Starts at the second comma
	    if (commaCount == 2)
	    {
	        start = i + 1;
	    }
	    else if (commaCount == 6)
	    {
	        // End is at the 6th comma
		end = i;
	    }
	}
    }
    // Create output string
    if (start > -1 && end > -1)
    {
        nmea[end] = NULL;
        char *subNmea = &nmea[start];
        //int newStrLen = end - start + 1 + 15;
        char tempstr[90];
        int num = buoyNum(nmea);
        // If buoy not found, do nothing
        if (num == -1)
        {
            *newStr = NULL;
            return;
        }
        sprintf(tempstr, "$RATLL,%d,%s,Q*", num, subNmea);
        // Calculate checksum
        unsigned char checksum = 0;
        for (unsigned int j = 1; j < strlen(tempstr)-1; j++)
        {
            // Checksum is XOR of all bytes past the initial $
            checksum ^= tempstr[j];
	}
	sprintf(newStr, "%s%02X", tempstr, checksum);
        // Save this message for later
        if (num < NUM_MESSAGES)
        {
            // Buoy numbering is 1-based
            strncpy(_msgs[num-1].msg, newStr, MESSAGE_LEN);
            _msgs[num-1].valid = 1;
        }
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


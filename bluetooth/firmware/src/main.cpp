#include <Arduino.h>
#include <BluetoothSerial.h>

// Check if Bluetooth configurations are enabled in the SDK
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define LED 2

// Bluetooth variables
BluetoothSerial SerialBT;

// Card Reader variables
#define MAX_BITS 100           // max number of bits
#define WEIGAND_WAIT_TIME 3000 // time to wait for another weigand pulse

unsigned char databits[MAX_BITS]; // stores all of the data bits
volatile unsigned int bitCount = 0;
unsigned char flagDone;       // goes low when data is currently being captured
unsigned int weigand_counter; // countdown until we assume there are no more bits

volatile unsigned long facilityCode = 0; // decoded facility code
volatile unsigned long cardCode = 0;     // decoded card code

// Breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
volatile unsigned long cardChunk1 = 0;
volatile unsigned long cardChunk2 = 0;

// Define reader input pins
#define DATA0 32 // card reader DATA0
#define DATA1 33 // card reader DATA1

/* #####----- Process interupts -----##### */
// interrupt that happens when INT0 goes low (0 bit)
void ISR_INT0()
{
  // Serial.print("0");
  bitCount++;
  flagDone = 0;

  if (bitCount < 23)
  {
    bitHolder1 = bitHolder1 << 1;
  }
  else
  {
    bitHolder2 = bitHolder2 << 1;
  }
  weigand_counter = WEIGAND_WAIT_TIME;
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  // Serial.print("1");
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;

  if (bitCount < 23)
  {
    bitHolder1 = bitHolder1 << 1;
    bitHolder1 |= 1;
  }
  else
  {
    bitHolder2 = bitHolder2 << 1;
    bitHolder2 |= 1;
  }

  weigand_counter = WEIGAND_WAIT_TIME;
}

/* #####----- Print bits function -----##### */
void printBits()
{
  if (bitCount >= 26)
  { // ignore data caused by noise
    Serial.print(bitCount);
    Serial.print(" bit card. ");
    Serial.print("FC = ");
    Serial.print(facilityCode);
    Serial.print(", CC = ");
    Serial.print(cardCode);
    Serial.print(", 44bit HEX = ");
    Serial.print(cardChunk1, HEX);
    Serial.println(cardChunk2, HEX);
  }
}

/* #####----- Process card format function -----##### */
void getCardNumAndSiteCode()
{
  unsigned char i;

  // bits to be decoded differently depending on card format length
  // see http://www.pagemac.com/projects/rfid/hid_data_formats for more info
  // also specifically: www.brivo.com/app/static_data/js/calculate.js
  switch (bitCount)
  {

  ///////////////////////////////////////
  // standard 26 bit format
  // facility code = bits 2 to 9
  case 26:
    for (i = 1; i < 9; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 10 to 23
    for (i = 9; i < 25; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 33 bit HID Generic
  case 33:
    for (i = 1; i < 8; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 8; i < 32; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 34 bit HID Generic
  case 34:
    for (i = 1; i < 17; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 17; i < 33; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 35 bit HID Corporate 1000 format
  // facility code = bits 2 to 14
  case 35:
    for (i = 2; i < 14; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 15 to 34
    for (i = 14; i < 34; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;
  }
  return;
}

/* #####----- Card value processing -----##### */
// Function to append the card value (bitHolder1 and bitHolder2) to the necessary array
// then translate that to the two chunks for the card value that will be output
void getCardValues()
{
  switch (bitCount)
  {
    // Example of full card value
    // |>   preamble   <| |>   Actual card value   <|
    // 000000100000000001 11 111000100000100100111000
    // |> write to chunk1 <| |>  write to chunk2   <|

  case 26:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 2)
      {
        bitWrite(cardChunk1, i, 1); // Write preamble 1's to the 13th and 2nd bits
      }
      else if (i > 2)
      {
        bitWrite(cardChunk1, i, 0); // Write preamble 0's to all other bits above 1
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 20)); // Write remaining bits to cardChunk1 from bitHolder1
      }
      if (i < 20)
      {
        bitWrite(cardChunk2, i + 4, bitRead(bitHolder1, i)); // Write the remaining bits of bitHolder1 to cardChunk2
      }
      if (i < 4)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i)); // Write the remaining bit of cardChunk2 with bitHolder2 bits
      }
    }
    break;

  case 27:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 3)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 3)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 19));
      }
      if (i < 19)
      {
        bitWrite(cardChunk2, i + 5, bitRead(bitHolder1, i));
      }
      if (i < 5)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 28:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 4)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 4)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 18));
      }
      if (i < 18)
      {
        bitWrite(cardChunk2, i + 6, bitRead(bitHolder1, i));
      }
      if (i < 6)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 29:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 5)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 5)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 17));
      }
      if (i < 17)
      {
        bitWrite(cardChunk2, i + 7, bitRead(bitHolder1, i));
      }
      if (i < 7)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 30:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 6)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 6)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 16));
      }
      if (i < 16)
      {
        bitWrite(cardChunk2, i + 8, bitRead(bitHolder1, i));
      }
      if (i < 8)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 31:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 7)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 7)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 15));
      }
      if (i < 15)
      {
        bitWrite(cardChunk2, i + 9, bitRead(bitHolder1, i));
      }
      if (i < 9)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 32:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 8)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 8)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 14));
      }
      if (i < 14)
      {
        bitWrite(cardChunk2, i + 10, bitRead(bitHolder1, i));
      }
      if (i < 10)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 33:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 9)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 9)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 13));
      }
      if (i < 13)
      {
        bitWrite(cardChunk2, i + 11, bitRead(bitHolder1, i));
      }
      if (i < 11)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 34:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 10)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 10)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 12));
      }
      if (i < 12)
      {
        bitWrite(cardChunk2, i + 12, bitRead(bitHolder1, i));
      }
      if (i < 12)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 35:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 11)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 11)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 11));
      }
      if (i < 11)
      {
        bitWrite(cardChunk2, i + 13, bitRead(bitHolder1, i));
      }
      if (i < 13)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 36:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 12)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 12)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 10));
      }
      if (i < 10)
      {
        bitWrite(cardChunk2, i + 14, bitRead(bitHolder1, i));
      }
      if (i < 14)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 37:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 9));
      }
      if (i < 9)
      {
        bitWrite(cardChunk2, i + 15, bitRead(bitHolder1, i));
      }
      if (i < 15)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;
  }
  return;
}

/* #####----- Write to Bluetooth function -----##### */
void writeBT()
{
  if (bitCount >= 26)
  { // ignore data caused by noise
    digitalWrite(LED_BUILTIN, HIGH);
    SerialBT.print(bitCount);
    SerialBT.print(" bit card (hex): ");
    SerialBT.print(cardChunk1, HEX);
    SerialBT.print(cardChunk2, HEX);
    SerialBT.print(", FC (dec) = ");
    SerialBT.print(facilityCode, DEC);
    SerialBT.print(", CC (dec) = ");
    SerialBT.print(cardCode, DEC);
    SerialBT.print(", BIN: ");
    for (int i = 19; i >= 0; i--)
    {
      SerialBT.print(bitRead(cardChunk1, i));
    }
    for (int i = 23; i >= 0; i--)
    {
      SerialBT.print(bitRead(cardChunk2, i));
    }
    Serial.println("Card data sent via Bluetooth");
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
}

/* #####----- Setup function -----##### */
void setup()
{
  pinMode(LED, OUTPUT); // LED

  // set tx/rx pins for card reader
  pinMode(DATA0, INPUT); // DATA0 (INT0)
  pinMode(DATA1, INPUT); // DATA1 (INT1)

  // Open serial comm for debug
  Serial.begin(115200);

  // Initialize Bluetooth
  SerialBT.begin("tusk");
  Serial.println("Bluetooth Started! Ready to pair...");

  // binds the ISR functions to the falling edge of INT0 and INT1
  attachInterrupt(DATA0, ISR_INT0, FALLING);
  attachInterrupt(DATA1, ISR_INT1, FALLING);
  weigand_counter = WEIGAND_WAIT_TIME;
}

/* #####----- Loop function -----##### */
void loop()
{
  // Wait to make sure that there have been no more data pulses before processing data
  if (!flagDone)
  {
    if (--weigand_counter == 0)
      flagDone = 1;
  }

  // if there are bits and the weigand counter went out
  if (bitCount > 0 && flagDone)
  {
    unsigned char i;

    getCardValues();
    getCardNumAndSiteCode();

    printBits();
    writeBT();

    // cleanup and get ready for the next card
    bitCount = 0;
    facilityCode = 0;
    cardCode = 0;
    bitHolder1 = 0;
    bitHolder2 = 0;
    cardChunk1 = 0;
    cardChunk2 = 0;

    for (i = 0; i < MAX_BITS; i++)
    {
      databits[i] = 0;
    }
  }
}
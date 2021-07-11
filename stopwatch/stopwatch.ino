const int SW0_PIN = 8;
const int SW1_PIN = 9;
const int SERVO_PIN = 11;
const int DIN_PIN = 3;
const int CS_PIN = 4;
const int CLK_PIN = 5;

#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Servo myservo;

void max7219_write_cmd(int digit, int command, int cmd_data);

// This is the code I wrote for the DRO attachged to the athe
// and mill


#define MAX_HIGH 1
#define MAX_LOW  0

const int CMD_NOOP    = 0;
const int CMD_DIGIT0  = 1;
const int CMD_DIGIT1  = 2;
const int CMD_DIGIT2  = 3;
const int CMD_DIGIT3  = 4;
const int CMD_DIGIT4  = 5;
const int CMD_DIGIT5  = 6;
const int CMD_DIGIT6  = 7;
const int CMD_DIGIT7  = 8;

const int CMD_DECODE_MODE = 0x9;
const int CMD_INTENSITY   = 0xa;
const int CMD_SCAN_LIMIT  = 0xb;
const int CMD_SHUTDOWN    = 0xc;
const int CMD_TEST        = 0xf;

const int DIGIT_CHAR_DASH  = 0xa;
const int DIGIT_CHAR_BLANK = 0xf;

void max7219_setup(void)
{

  digitalWrite(CS_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
  
  max7219_write_cmd(0, CMD_TEST,           0);
  max7219_write_cmd(0, CMD_INTENSITY,      1);
  max7219_write_cmd(0, CMD_DECODE_MODE, 0xFF);
  max7219_write_cmd(0, CMD_SHUTDOWN,       1);
  max7219_write_cmd(0, CMD_SCAN_LIMIT,     7);
}

//
// Write data to an address, n displays
//

#define NUM_DIGITS 1

int current_digit[NUM_DIGITS];

#define WAIT_7219 2

void max7219_write_cmd(int digit, int command, int cmd_data)
{
  int i,j;
  int digit_index;
  int data;
  
  for(digit_index=0; digit_index< NUM_DIGITS; digit_index++)
    {
      if( digit == digit_index )
	{
	  data = ((command & 0xf)<<8) | (cmd_data & 0xff);
	}
      else
	{
	  data = current_digit[digit_index];
	}
      
      // Write a binary pattern repeatedly
      for(i=15; i>=0; i--)
	{
	  for(j=0; j<WAIT_7219;j++)
	    {
	    }

	  // Clock out MSB first
	  if( data & (1 << i))
	    {
	      digitalWrite(DIN_PIN, MAX_HIGH);
	    }
	  else
	    {
	      digitalWrite(DIN_PIN, MAX_LOW);
	    }
	  
	  // Clock data in
	  digitalWrite(CLK_PIN, MAX_LOW);

	  for(j=0; j<WAIT_7219;j++)
	    {
	    }

	  digitalWrite(CLK_PIN, MAX_HIGH);
	}
      }
  
  // Load data
  digitalWrite(CS_PIN, MAX_LOW);
  for(j=0; j<WAIT_7219;j++)
    {
    }

  digitalWrite(CS_PIN, MAX_HIGH);
}

int d4;
#define MAX_DIGITS 8
#define DISPLAYED_DIGITS 6

//
// Displays an integer n, with leading zero suppression on or
// off and a decimal point position (-1 is no DP)
// Any display can be specified.
// Heartbeat on top DP of dislay 0

unsigned char hb = 0;

void max7219_write_int(int display, long int n, int lead_zero, int dp_pos)
{
  int dig;
  int v;
  int l0_flag = lead_zero ? 1:0;
  int digitbuffer[MAX_DIGITS];
  long int number = n;
  int end_l0_digit;

  if ( dp_pos == -1 )
    {
      // No DP so end digit is end of number
      end_l0_digit = 0;
    }
  else
    {
      // DP is present so last digit is the one the DP is on
      end_l0_digit = dp_pos;
    }

  // Blank buffer
  for(dig=0; dig<MAX_DIGITS; dig++)
    {
      digitbuffer[dig] = DIGIT_CHAR_BLANK;
    }

  // Process sign 
  if ( number < 0 )
    {

      digitbuffer[7] = DIGIT_CHAR_DASH;
      n = -n;
    }
  else
    {
      digitbuffer[7] = DIGIT_CHAR_BLANK;
    }


  // Digits displayed from MS to LS
  for(dig=0; dig<DISPLAYED_DIGITS; dig++)
    {
      digitbuffer[dig] = (n % 10);
      n /= 10;
    }

  // Process leading zeros
  if ( lead_zero )
    {
      // Scan all digits after sign digit
      for(dig=(DISPLAYED_DIGITS-1); dig>=0; dig--)
	{
	  v = digitbuffer[dig];

	  // Turn off leadin zeroing if we see a non zero diit, or we have reached the 
	  // end of the number
	  // We also ned to stop if there's a DP position, as we don't want to
	  // remove leadin zeros beyond the DP digit
	  if( (v != 0) || (dig == end_l0_digit))
	    {
	      l0_flag = 0;
	    }
	  
	  if( l0_flag && (v == 0))
	    {
	      v = DIGIT_CHAR_BLANK;
	    }

	  digitbuffer[dig] = v;	  
	}
    }

#if 0
  if( display == 0 )
    {
      digitbuffer[7] |= (hb % 8)>=3?0x80:0x00;
      hb++;
    }
#endif

  // Write digits to display
  for(dig=0; dig<MAX_DIGITS; dig++)
    {
      // Process DP
      if( dp_pos == dig )
	{
	  digitbuffer[dig] |= 0x80;
	}

      max7219_write_cmd(display, CMD_DIGIT0+dig, digitbuffer[dig]);
    }
}


void setup()
{
  
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println("Stopwatch");
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
   // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds


  pinMode(SW0_PIN, INPUT);
  pinMode(SW1_PIN, INPUT);

  pinMode(DIN_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  
  myservo.attach(SERVO_PIN);

  max7219_setup();
}

// Count will be displayed on the LED display
long count = 0;
long time_offset = 0;

// Angle of the servo
int angle = 0;

int last_sw0, last_sw1;
int run = 1;

#define NUM_C  3

void loop()
{  
  char line[40];
  float x,y;

  int r[NUM_C] =
    {
     display.height()/2-1,
     display.height()/4-1,
     display.height()/5,
    };
  
  int oy[NUM_C] =
    {
     display.height()/2,
     display.height()*3/4,
     display.height()*1/4,
    };
  
  int ox[NUM_C] =
    {
     display.height()/2,
     display.width()*4/5,
     display.width()*4/5,
    };
  
  int s[NUM_C] =
    {
     1800,
     180,
     18,
    };
  
  int sw0, sw1;
  int c;  // Display milliseconds
  //  count = millis()*10;

  
  sw0 = digitalRead(SW0_PIN);
  sw1 = digitalRead(SW1_PIN);

  sprintf(line, "%d %d, %d %d", last_sw0, sw0, last_sw1, sw1);
  
  Serial.println(line);

  long time = millis()/10;
  long split;
  
  myservo.write(count/100);

  // Attend to switches
  if( (sw0 == 0) && (last_sw0 == 1) )
    {
      time_offset = time;
      //count = 0;
    }

  count = time - time_offset;
  
  // Display milliseconds
  //count = millis()*10;
  
  if( (sw1 == 0) && (last_sw1 == 1) )
    {
      split = time-time_offset;
      run = 1-run;
    }

  if( !run )
    {
      count = split;
    }

  max7219_write_int(0, count, 1, 2);
  
  last_sw1 = sw1;
  last_sw0 = sw0;

  display.clearDisplay();
  
  for(c=0; c<NUM_C; c++)
    {
      x = r[c]*sin(((float)count)/s[c]*3.1415)+ox[c];
      y = r[c]*cos(((float)count)/s[c]*3.1415)+oy[c];

      display.drawCircle(ox[c], oy[c], r[c], SSD1306_WHITE);
      display.drawLine(ox[c], oy[c], x, y, SSD1306_WHITE);
    }
  display.display();

}

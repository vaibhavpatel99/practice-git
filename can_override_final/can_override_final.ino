#include <mcp_can.h>
#include <SPI.h>

int g = 9;

#define NUM_BUTTONS 100

const int SPI_CS_PIN = 53;
MCP_CAN CAN(SPI_CS_PIN);

const int DIGITAL_MAX = 36;
const int DIGITAL_MIN = 22;
const int LED_MAX = 52;

unsigned long msg_id = 0x3FF;
unsigned char msg_len = 3;
unsigned char msg_data[3] = {0, 0, 0};

int buttonState[NUM_BUTTONS] = {0};
int buttonPin[NUM_BUTTONS] = {0};
int knob1_pin = 14;
int knob2_pin = 15;

const long SEND_MSG_TIME = 20;
unsigned long msg_time = 0;

unsigned char knob_data(int potVal, int buttonState, int ledPin, int shift) {
  unsigned char msg = 0;

  if(buttonState == HIGH) {
    //led
    digitalWrite(ledPin, HIGH);
    if(potVal < 6) {
      //phase 1
      msg = msg | (1 << shift);
    } else if(potVal < 61) {
      //phase 2
      msg = msg | (B10 << shift);
    } else if(potVal < 171) {
      //phase 3
      msg = msg | (B11 << shift);
    } else if(potVal < 295) {
      //phase 4
      msg = msg | (B100 << shift);
    } else if(potVal < 428) {
      //phase 5
      msg = msg | (B101 << shift);
    } else if(potVal < 571) {
      //phase 6
      msg = msg | (B110 << shift);
    } else if(potVal < 709) {
      //phase 7
      msg = msg | (B111 << shift);
    } else if(potVal < 822) {
      //phase 8
      msg = msg | (B1000 << shift);
    } else if(potVal < 938) {
      //phase 9
      msg = msg | (B1001 << shift);
    } else if(potVal < 1015) {
      //phase 10
      msg = msg | (B1010 << shift);
    } else {
      //phase 11
      msg = msg | (B1011 << shift);
    }
  } else {
    digitalWrite(ledPin, LOW);
    msg = 0;
  }

  return msg;
}

unsigned char assign_data(int button1State, int button2State, int ledPin, int ledPin2, int on_shift, int off_shift) { 
   unsigned char msg = 0;
   
   if(button1State == HIGH) {
    //led 
    if(button2State == HIGH) {
      msg = msg | (1 << on_shift);
      digitalWrite(ledPin, HIGH);
      digitalWrite(ledPin2, LOW);
    } else {
      msg = msg | (1 << off_shift);
      digitalWrite(ledPin, LOW);
      digitalWrite(ledPin2, HIGH);
    }
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(ledPin2, LOW);
    msg = 0;
  }

  return msg;
}

void setup() {
  Serial.begin(9600);
  
  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
  Serial.println("CAN BUS Shield init ok!");
    
  int buttonState[DIGITAL_MAX-1] = {0};
//  for(int x = DIGITAL_MIN; x < DIGITAL_MAX; x++) {
//    
//    //int buttonPin[x] = {0};
//  }
  
  for(int i = DIGITAL_MIN; i < LED_MAX; i++) {
    buttonPin[i] = i;
  }

  pinMode(knob1_pin, INPUT);
  pinMode(knob2_pin, INPUT);

  for(int n = DIGITAL_MIN; n < DIGITAL_MAX; n++) {
    pinMode(buttonPin[n], INPUT);
  }

  for(int b = DIGITAL_MAX; b < LED_MAX; b++) {
    pinMode(buttonPin[b], OUTPUT);
  }
}

void loop() {
  int knob1 = analogRead(knob1_pin);
  int knob2 = analogRead(knob2_pin);

  buttonState[38] = HIGH;
  
  msg_data[0] = 0;
  msg_data[1] = 0;
  msg_data[2] = 0;
  
  for(int a = DIGITAL_MIN; a < DIGITAL_MAX; a++) {
    buttonState[a] = digitalRead(buttonPin[a]);
  }
  
  //switch 1
  if(buttonState[22] == HIGH) {
    digitalWrite(buttonPin[40], HIGH);
    if(buttonState[23] == HIGH) {
      msg_data[1] = msg_data[1] | (1 << 7);
    } else {
      msg_data[1] = msg_data[1] | (1 << 6);
    }
  } else {
    digitalWrite(buttonPin[40], LOW);
    msg_data[1] = msg_data[1] | (0 << 7);
  }
  msg_data[0] = assign_data(buttonState[22], buttonState[23], buttonPin[36], buttonPin[37], 0, 1);

  //switch 2
  msg_data[0] = msg_data[0] | (assign_data(buttonState[24], buttonState[25], buttonPin[38], buttonPin[39], 2, 3));

  //switch 3
  msg_data[0] = msg_data[0] | (assign_data(buttonState[26], buttonState[27], buttonPin[40], buttonPin[41], 4, 5));

  //switch 4
  msg_data[1] = assign_data(buttonState[28], buttonState[29], buttonPin[42], buttonPin[43], 0, 1);

  //switch 5
  msg_data[1] = msg_data[1] | (assign_data(buttonState[30], buttonState[31], buttonPin[44], buttonPin[45], 2, 3));

  //switch 6
  msg_data[1] = msg_data[1] | (assign_data(buttonState[32], buttonState[33], buttonPin[46], buttonPin[47], 4, 5));

  //knob 1
  msg_data[2] = knob_data(knob1, buttonState[34], buttonPin[48], 0);

  //knob 2
  msg_data[2] = msg_data[2] | (knob_data(knob2, buttonState[35], buttonPin[49], 4));

  if((millis() - msg_time) > SEND_MSG_TIME) {
    msg_time = millis();
    
    CAN.sendMsgBuf(msg_id, 0, msg_len, msg_data); 
  }
}

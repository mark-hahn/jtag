
// BOARD VERSION 1

#include "jtag.h"
#include <NeoHWSerial.h>

byte curState;

void printHexByte(byte num) {
   char tmp[16];
   sprintf(tmp, "0x%.2x", num);
   NeoSerial.print(tmp);
}
void printHex32(uint32_t num) {
   char tmp[16];
   uint16_t half = num >> 16;
   sprintf(tmp, "0x%.4x", half);
   NeoSerial.print(tmp);
   half = num & 0xffff;
   sprintf(tmp, "%.4x", half);
   NeoSerial.print(tmp);
}

void gotoState (byte newState) {
  if (newState == TLR) {
		volatile byte delay = 0;
    byte i = delay;
    setTms;
    for (i=0; i<5; i++) {
      delay = 0;
      clrTck;
      delay = 0;
      setTck;
    }
    curState = TLR;
    return;
  }
  while (curState != newState) {
    byte tms = (statePath[curState] >> newState) & 1;
    clrTck;
    if(tms) setTms else clrTms;
    uint8_t step = tmsStep[curState];
    curState = (tms ? step >> 4 : step) & 0xf;
    setTck;
  }
}

void reset() {
  curState = 0;
  pinMode(PROG, OUTPUT);
  pinMode(TCK,  OUTPUT);
  pinMode(TMS,  OUTPUT);
  pinMode(TDI,  OUTPUT);
  pinMode(TDO,  INPUT);
  gotoState(TLR);
  gotoState(RTI);
}

byte send8(byte di, bool last) {
  clrTms;
  byte i, dout = 0;
  for (i=1; i; i <<= 1) {
    if (di & i) {setTdi} else {clrTdi};
    if (last && i == 128) setTms;
    clrTck;
    setTck;
    dout = dout >> 1; dout |= (PIND & TDOMASK);
  }
  return dout;
}

byte sendIR(byte di) {
  gotoState(SI);
  byte dout = send8(di, 1);
	curState = E1I;
  gotoState(UI);
  return dout;
}

uint32_t sendDR(uint32_t di) {
  gotoState(SD);
  uint32_t dout;
  dout  = (uint32_t) send8( di        & 0xff, 0);
  dout |= (uint32_t) send8((di >>  8) & 0xff, 0) <<  8;
  dout |= (uint32_t) send8((di >> 16) & 0xff, 0) << 16;
  dout |= (uint32_t) send8((di >> 24) & 0xff, 1) << 24;
	curState = E1D;
  gotoState(TLR);
  return dout;
}

bool isXC9572  = false,
     isXC2C128 = false,
     ready     = false;

uint32_t checkId() {
	isXC9572 = isXC2C128 = ready = false;
  reset();
  sendIR(IDCODE);
  uint32_t id = sendDR(0);
	uint32_t idMasked = id & 0x0fffffff;
	if (idMasked == 0x09604093) isXC9572  = true;
	if (idMasked == 0x01111111) isXC2C128 = true;
	if (isXC9572 || isXC2C128) {
		ready = true;
		NeoSerial.print("Logic+: Ready with CPLD ");
		if (isXC9572)  NeoSerial.print("XC9572XL (");
		if (isXC2C128) NeoSerial.print("XC2C128 (");
		printHex32(id); NeoSerial.println(")");
	} else {
		NeoSerial.print("Logic+: Not ready, Logic+ shield not found (");
		printHex32(id); NeoSerial.println(")");
		NeoSerial.println("Check that shield is mounted and power light is on.");
	}
}

static void handleRxChar(uint8_t c) {
  NeoSerial.println(c);
}

void setup() {
  NeoSerial.attachInterrupt( handleRxChar );
  NeoSerial.begin( 115200 );
  NeoSerial.println("Logic+: Starting ...");
  checkId();
}

void loop() {
}

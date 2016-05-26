
// BOARD VERSION 1

byte curState;

// jtag instructions (XC9572XL)
#define IDCODE 0xfe

// pins
#define PROG 2
#define TCK  4
#define TMS  5
#define TDI  6
#define TDO  7
#define TDOMASK  0x80

// states
#define TLR 0
#define RTI 1
#define SDS 2
#define CD  3
#define SD  4
#define E1D 5
#define PD  6
#define E2D 7
#define UD  8
#define SIS 9
#define CI  10
#define SI  11
#define E1I 12
#undef PI
#define PI  13
#define E2I 14
#define UI  15

// high nibble: next state if TMS=1
// low nibble: next step if TMS=0
const uint8_t tmsStep[] = {
	0x01,	/* STATE_TLR		*/
	0x21,	/* STATE_RTI		*/
	0x93,	/* STATE_SELECT_DR_SCAN	*/
	0x54,	/* STATE_CAPTURE_DR	*/
	0x54,	/* STATE_SHIFT_DR	*/
	0x86,	/* STATE_EXIT1_DR	*/
	0x76,	/* STATE_PAUSE_DR	*/
	0x84,	/* STATE_EXIT2_DR	*/
	0x21,	/* STATE_UPDATE_DR	*/
	0x0a,	/* STATE_SELECT_IR_SCAN	*/
	0xcb,	/* STATE_CAPTURE_IR	*/
	0xcb,	/* STATE_SHIFT_IR	*/
	0xfd,	/* STATE_EXIT1_IR	*/
	0xed,	/* STATE_PAUSE_IR	*/
	0xfb,	/* STATE_EXIT2_IR	*/
	0x21,	/* STATE_UPDATE_IR	*/
};
// bit 0:  TMS to get to state 0
// bit 15: value of TMS to get to state 15
const uint16_t statePath[] = {
	0,	    /* TLR */
	0xfffd,	/* RTI */
	0xfe03,	/* SDS */
	0xffe7,	/* CD */
	0xffef,	/* SD */
	0xff0f,	/* E1D */
	0xffbf,	/* PD */
	0xff0f,	/* E2D */
	0xfefd,	/* UD */
	0x01ff,	/* SIS */
	0xf3ff,	/* CI */
	0xf7ff,	/* SI */
	0x87ff,	/* E1I */
	0xdfff,	/* PI */
	0x87ff,	/* E2I */
	0x7ffd,	/* UI */
};

#define setProg \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (PROG) );
#define clrProg \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (PROG) );
#define setTck  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TCK) );
#define clrTck \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TCK) );
#define setTms  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TMS) );
#define clrTms \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TMS) );
#define setTdi  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TDI) );
#define clrTdi \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (TDI) );

void printHexByte(byte num) {
   char tmp[16];
   char format[128];
   sprintf(tmp, "0x%.2x", num);
   Serial.print(tmp);
}

void printHex32(uint32_t num) {
   char tmp[16];
   char format[128];
   uint16_t half = num >> 16;
   sprintf(tmp, "0x%.4x", half);
   Serial.print(tmp);
   half = num & 0xffff;
   sprintf(tmp, "%.4x", half);
   Serial.print(tmp);
}
byte debug = 10;
void gotoState (byte newState) {
  // Serial.println("gotoState");
  // Serial.println(curState);
  // Serial.println(newState);

  if (newState == TLR) {
    byte i;
    volatile byte delay;
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
    // Serial.println(curState);
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
  gotoState(UD);
  return dout;
}

void setup() {
  Serial.begin(115200);
  Serial.println("JTAG Test");
  reset();
  printHexByte(sendIR(IDCODE)); Serial.println("");
  printHex32(sendDR(0));        Serial.println("");
  Serial.println("done");
}

void loop() {
}

/*
JTAG Test
0x01
0x59604093
done
*/

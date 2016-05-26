
// BOARD VERSION 1
byte curState = 0;

// pins
#define PROG 2
#define TCK  4
#define TMS  5
#define TDI  6
#define TDO  7
#define TCKMASK  0x10
#define TMSMASK  0x20
#define TDIMASK  0x40
#define TDOMASK  0x80

// states
#define TLR 0
#define RTI 1
#define SDS 2
#define CD  3
#define SD  4
#define E1D 5
#define PD  61
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

// jtag instructions (XC9572XL)
#define IDCODE 0xfe

#define setProg \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (2) );
#define clrProg \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (2) );
#define setTck  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (4) );
#define clrTck \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (4) );
#define setTms  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (5) );
#define clrTms \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (5) );
#define setTdi  \
  asm("sbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (6) );
#define clrTdi \
  asm("cbi %0,%1" : : "I" (_SFR_IO_ADDR(PORTD)) , "I" (6) );

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

void setState(byte newState) {
  byte bit = 0;
  switch(curState) {
    case TLR: bit = (newState !=  RTI); break;
    case RTI: bit = (newState !=  RTI); break;
    case SDS: bit = (newState !=  CD) ; break;
    case CD:  bit = (newState !=  SD) ; break;
    case SD:  bit = (newState !=  SD) ; break;
    case E1D: bit = (newState !=  PD) ; break;
    case PD:  bit = (newState !=  PD) ; break;
    case E2D: bit = (newState !=  SD) ; break;
    case UD:  bit = (newState !=  RTI); break;
    case SIS: bit = (newState !=  CI) ; break;
    case CI:  bit = (newState !=  SI) ; break;
    case SI:  bit = (newState !=  SI) ; break;
    case E1I: bit = (newState !=  PI) ; break;
    case PI:  bit = (newState !=  PI) ; break;
    case E2I: bit = (newState !=  SI) ; break;
    case UI:  bit = (newState !=  RTI); break;
  }
  clrTck;
  if(bit) setTms else clrTms;
  curState = newState;
  setTck;
}

void reset() {
  byte i;
  volatile byte delay;
  pinMode(PROG, OUTPUT);
  pinMode(TCK, OUTPUT);
  pinMode(TMS, OUTPUT);
  pinMode(TDI, OUTPUT);

  setTck;
  setTms;
  clrProg;
  for (i=0; i<10; i++) {
    delay = 0;
    clrTck;
    delay = 0;
    setTck;
  }
  curState = TLR;
  setState(RTI);
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
  setState(SDS);
  setState(SIS);
  setState(CI);
  setState(SI);
  byte dout = send8(di, 1);
  setState(UI);
  return dout;
}

uint32_t sendDR(uint32_t di) {
  setState(SDS);
  setState(CD);
  setState(SD);
  uint32_t dout;
  dout  = (uint32_t) send8( di        & 0xff, 0);
  dout |= (uint32_t) send8((di >>  8) & 0xff, 0) <<  8;
  dout |= (uint32_t) send8((di >> 16) & 0xff, 0) << 16;
  dout |= (uint32_t) send8((di >> 24) & 0xff, 1) << 24;
  setState(UD);
  return dout;
}
void setup() {
  Serial.begin(115200);
  Serial.println("JTAG Test");
  reset();
  printHexByte(sendIR(IDCODE)); Serial.println("");
  printHex32(sendDR(0));      Serial.println("");
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

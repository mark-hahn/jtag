// #define F_CPU 16000000UL	// 16 MHz
#define UART_BAUD 115200	  // baudrate

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

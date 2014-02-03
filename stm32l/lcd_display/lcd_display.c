/* Write something to the LCD Display */

#include <stdint.h>

#include "stm32l.h"

#define STACK_TOP 0x20000800

#define SEGA	0x8000
#define SEGB	0x4000
#define SEGC	0x2000
#define SEGD	0x1000
#define SEGE	0x0800
#define SEGF	0x0400
#define SEGG	0x0200
#define SEGH	0x0100
#define SEGJ	0x0080
#define SEGK	0x0040
#define SEGM	0x0020
#define SEGN	0x0010
#define SEGP	0x0008
#define SEGQ	0x0004
#define SEGCOLON	0x0002
#define SEGBAR		0x0002
#define SEGDP		0x0001

static short font[128]={
	0,
	0,
	0,
	0,
	0,
	0,	/* 5 */
	0,
	0,
	0,
	0,
	0,	/* 10 */
	0,
	0,
	0,
	0,
	0,	/* 15 */
	0,
	0,
	0,
	0,
	0,	/* 20 */
	0,
	0,
	0,
	0,
	0,	/* 25 */
	0,
	0,
	0,
	0,
	0,	/* 30 */
	0,
	0,	/* ' '	32 */
	0,	/* !	33 */
	0,      /* "	34 */
	0,	/* #	35 */
	0,	/* $	36 */
	0,	/* %	37 */
	0,	/* &	38 */
	0,	/* '	39 */
	0,	/* (	40 */
	0,	/* )	41 */
	0,	/* *	42 */
	0,	/* +	43 */
	0,	/* ,	44 */
	0,	/* -	45 */
	0,	/* .	46 */
	0,	/* /	47 */
	SEGA|SEGB|SEGC|SEGD|SEGE|SEGF|SEGK|SEGQ,	/* 0	48 */
	SEGB|SEGC|SEGK,					/* 1	49 */
	SEGA|SEGB|SEGM|SEGG|SEGE|SEGD,			/* 2	50 */
	SEGA|SEGB|SEGM|SEGC|SEGD|SEGG,			/* 3	51 */
	SEGF|SEGG|SEGM|SEGB|SEGC,			/* 4	52 */
	SEGA|SEGF|SEGG|SEGN|SEGD,			/* 5	53 */
	SEGA|SEGF|SEGE|SEGD|SEGC|SEGG|SEGM,		/* 6	54 */
	SEGA|SEGK|SEGP|SEGG|SEGM,			/* 7	55 */
	SEGA|SEGB|SEGC|SEGD|SEGE|SEGF|SEGG|SEGM,	/* 8	56 */
	SEGF|SEGA|SEGB|SEGC|SEGG|SEGM,			/* 9	57 */
	0,	/* :	58 */
	0,	/* ;	59 */
	0,	/* <	60 */
	0,	/* =	61 */
	0,	/* >	62 */
	0,	/* ?	63 */
	0,	/* @	64 */
	SEGB|SEGC|SEGK|SEGM|SEGQ,		/* A	65 */
	SEGA|SEGD|SEGE|SEGF|SEGG|SEGK|SEGN,	/* B	66 */
	SEGA|SEGD|SEGE|SEGF,			/* C	67 */
	SEGA|SEGB|SEGC|SEGD|SEGJ|SEGP,		/* D	68 */
	SEGA|SEGD|SEGE|SEGF|SEGG,		/* E	69 */
	SEGA|SEGE|SEGF|SEGG,			/* F	70 */
	SEGA|SEGC|SEGD|SEGE|SEGF|SEGM,		/* G	71 */
	SEGB|SEGC|SEGE|SEGF|SEGG|SEGM,		/* H	72 */
	SEGA|SEGD|SEGJ|SEGP,			/* I	73 */
	SEGB|SEGC|SEGD|SEGE,			/* J	74 */
	SEGF|SEGE|SEGG|SEGK|SEGN,		/* K	75 */
	SEGD|SEGE|SEGF,				/* L	76 */
	SEGB|SEGC|SEGE|SEGF|SEGH|SEGK,		/* M	77 */
	SEGE|SEGF|SEGB|SEGC|SEGH|SEGN,		/* N	78 */
	SEGA|SEGB|SEGC|SEGD|SEGE|SEGF,		/* O	79 */
	SEGA|SEGB|SEGE|SEGF|SEGG|SEGM,		/* P	80 */
	SEGA|SEGB|SEGC|SEGD|SEGE|SEGF|SEGQ,	/* Q	81 */
	SEGA|SEGB|SEGE|SEGF|SEGG|SEGM|SEGN,	/* R	82 */
	SEGA|SEGF|SEGG|SEGM|SEGC|SEGD,		/* S	83 */
	SEGA|SEGJ|SEGP,				/* T	84 */
	SEGB|SEGC|SEGD|SEGE|SEGF,		/* U	85 */
	SEGK|SEGQ|SEGE|SEGF,			/* V	86 */
	SEGF|SEGE|SEGQ|SEGN|SEGC|SEGB,		/* W	87 */
	SEGH|SEGK|SEGN|SEGQ,			/* X	88 */
	SEGH|SEGK|SEGP,				/* Y	89 */
	SEGA|SEGK|SEGQ|SEGD,			/* Z	90 */
};



static void delay(int length) {

	volatile int i;

	for(i=0;i<length;i++) asm("nop");

}


GPIO_TypeDef *gpioa=(GPIO_TypeDef *)GPIOA_BASE;
GPIO_TypeDef *gpiob=(GPIO_TypeDef *)GPIOB_BASE;
GPIO_TypeDef *gpioc=(GPIO_TypeDef *)GPIOC_BASE;
RCC_TypeDef *rcc=(RCC_TypeDef *)RCC_BASE;
PWR_TypeDef *pwr=(PWR_TypeDef *)PWR_BASE;
RTC_TypeDef *rtc=(RTC_TypeDef *)RTC_BASE;
LCD_TypeDef *lcd=(LCD_TypeDef *)LCD_BASE;

static void lcd_clock_init(void) {

	/* enable power to clock interface */
	rcc->APB1ENR |= RCC_APB1ENR_PWREN;

	/* Disable write protection */
	pwr->CR	|= PWR_CR_DBP;

	/* Use LSI as RTC clock */
	rcc->CSR |= RCC_CSR_RTCSEL_LSI;

	/* enable RTC clock */
	rcc->CSR |= RCC_CSR_RTCEN;

	/* Disable write-protection for RTC registers */
	rtc->WPR = 0xca;
	rtc->WPR = 0x53;

	/* Wait for MSI clock to be read */
	while( (rcc->CR & RCC_CR_MSIRDY) == 0 ) ;

	/* enable LCD clock */
	rcc->APB1ENR |= RCC_APB1ENR_LCDEN;

	/* enable SYSCFG clock */
	rcc->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	/* enable LSI (internal low-speed oscilator) and wait until ready */
	rcc->CSR |= RCC_CSR_LSION;
	while ( (rcc->CSR & RCC_CSR_LSIRDY) == 0 );

	/* Select LSI as LCD Clock Source */
	rcc->CSR &= ~RCC_CSR_RTCSEL_LSI;
	rcc->CSR |= RCC_CSR_RTCSEL_LSI;

	/* Enable RTC */
	rcc->CSR |= RCC_CSR_RTCEN;
}

void lcd_pin_init(void) {

	/* Enable GPIOA and GPIOB and GPIOC clocks*/
	rcc->AHBENR |= (AHBENR_GPIOAEN|AHBENR_GPIOBEN|AHBENR_GPIOCEN);

	/* GPIOA mode alterfunc (0b10) 15,10,9,8,3,2,1 */
#define GPIOA_MODE_MASK	 0xc03f00fc
#define GPIOA_MODE_ALTER 0x802a00a8
	gpioa->MODER&=~GPIOA_MODE_MASK;
	gpioa->MODER|=GPIOA_MODE_ALTER;
	/* output mode?  output speed? pull-up/down? */

	/* Port A, pins 15,10,9,8,3,2,1 */
	/* Alternate function mode 0xb (LCD) */
			/*76543210*/
#define LCD_A_L_MASK	0x0000fff0
#define LCD_A_L_ENABLE	0x0000bbb0
			/*fedcba98*/
#define	LCD_A_H_MASK	0xf0000fff
#define LCD_A_H_ENABLE	0xb0000bbb

	gpioa->AFR[0]&=~LCD_A_L_MASK;
	gpioa->AFR[0]|=LCD_A_L_ENABLE;

	gpioa->AFR[1]&=~LCD_A_H_MASK;
	gpioa->AFR[1]|=LCD_A_H_ENABLE;


	/* GPIOB mode alterfunc (0b10) 15,14,13,12,11,10,9,8,5,4,3 */
#define GPIOB_MODE_MASK	 0xffff0fc0
#define GPIOB_MODE_ALTER 0xaaaa0a80
	gpiob->MODER&=~GPIOB_MODE_MASK;
	gpiob->MODER|=GPIOB_MODE_ALTER;

	/* Port B, pins 15,14,13,12,11,10,9,8,5,4,3 */
	/* Alternate function mode 0xb (LCD) */
			/*76543210*/
#define LCD_B_L_MASK	0x00fff000
#define LCD_B_L_ENABLE	0x00bbb000
			/*fedcba98*/
#define	LCD_B_H_MASK	0xffffffff
#define LCD_B_H_ENABLE	0xbbbbbbbb

	gpiob->AFR[0]&=~LCD_B_L_MASK;
	gpiob->AFR[0]|=LCD_B_L_ENABLE;

	gpiob->AFR[1]&=~LCD_B_H_MASK;
	gpiob->AFR[1]|=LCD_B_H_ENABLE;


	/* GPIOC mode alterfunc (0b10) 11,10,9,8,7,6,3,2,1,0 */
#define GPIOC_MODE_MASK	 0x00fff0ff
#define GPIOC_MODE_ALTER 0x00aaa0aa
	gpioc->MODER&=~GPIOC_MODE_MASK;
	gpioc->MODER|=GPIOC_MODE_ALTER;

	/* Port C, pins 11,10,9,8,7,6,3,2,1,0 */
	/* Alternate function mode 0xb (LCD) */
			/*76543210*/
#define LCD_C_L_MASK	0xff00ffff
#define LCD_C_L_ENABLE	0xbb00bbbb
			/*fedcba98*/
#define	LCD_C_H_MASK	0x0000ffff
#define LCD_C_H_ENABLE	0x0000bbbb

	gpioc->AFR[0]&=~LCD_C_L_MASK;
	gpioc->AFR[0]|=LCD_C_L_ENABLE;

	gpioc->AFR[1]&=~LCD_C_H_MASK;
	gpioc->AFR[1]|=LCD_C_H_ENABLE;

}

void lcd_config(void) {

	/************/
	/* Setup CR */
	/************/

	/* Set Bias to 1/3 */
	lcd->CR&=~LCD_CR_BIAS_MASK;
	lcd->CR|=LCD_CR_BIAS_1_3;

	/* Set Duty to 1/4 */
	lcd->CR&=~LCD_CR_DUTY_MASK;
	lcd->CR|=LCD_CR_DUTY_1_4;

	/* Set enable mux segment */
	lcd->CR|=LCD_CR_MUX_SEG;

	/* select internal voltage */
	lcd->CR&=~LCD_CR_VSEL;

	/*************/
	/* Setup FCR */
	/*************/

	lcd->FCR = 0;	// reset

	/* See Table 60 in the manual */
	/* set to 30.12Hz: ps=4 div=1 duty = 1/4 */

	/* Set prescaler to 4 */
	lcd->FCR &= ~LCD_FCR_PS_MASK;
	lcd->FCR |= (4<<22);

	/* Set clock divider to 1 */
	lcd->FCR &=  ~LCD_FCR_DIV_MASK;
	lcd->FCR |= (1<<18);

	/* contrast = mean */
	lcd->FCR &= ~LCD_FCR_CC_MASK;
	lcd->FCR |= LCD_FCR_CC_LCD4;

	/* Pulse width.  Longer takes more energy */
	//lcd->FCR &= LCD_FCR_PON_MASK;
	lcd->FCR |= (7<<4) ;

	/* wait until FCRSF of LCD_SR set */
	while( !(lcd->SR & LCD_SR_FCRSF));

	/* set LCDEN */
	lcd->CR |= LCD_CR_LCDEN;

	/* wait for LCDEN in LCD_SR */
	while( !(lcd->SR & LCD_SR_ENS));

	/* wait for booster for RDY in LCD_SR */
	while( !(lcd->SR & LCD_SR_RDY));
}

void lcd_display(unsigned int *buffer) {

	int i;

	/* wait until protection gone */
	while((lcd->SR & LCD_SR_UDR)) ;

	for(i=0;i<16;i++) {
		lcd->RAM[i]=buffer[i];
	}

	/* request update of display */
	lcd->SR |= LCD_SR_UDR;

}

static unsigned char lookup1[6]={28,26,24,20,18,17};
static unsigned char lookup2[6]={1,7,9,11,13,15};
static unsigned char lookup3[6]={0,2,8,10,12,14};
static unsigned char lookup4[6]={29,27,25,21,19,16};

void lcd_convert(char *string, unsigned int *buffer) {

	int i;

	for(i=0;i<16;i++) buffer[i]=0;

	for(i=0;i<6;i++) {
		if (font[(int)string[i]]&SEGA) buffer[1*2]|=1<<lookup1[i];
		if (font[(int)string[i]]&SEGB) buffer[0*2]|=1<<lookup1[i];
		if (font[(int)string[i]]&SEGC) buffer[1*2]|=1<<lookup2[i];
		if (font[(int)string[i]]&SEGD) buffer[1*2]|=1<<lookup3[i];
		if (font[(int)string[i]]&SEGE) buffer[0*2]|=1<<lookup3[i];
		if (font[(int)string[i]]&SEGF) buffer[1*2]|=1<<lookup4[i];
		if (font[(int)string[i]]&SEGG) buffer[0*2]|=1<<lookup4[i];
		if (font[(int)string[i]]&SEGH) buffer[3*2]|=1<<lookup4[i];
		if (font[(int)string[i]]&SEGJ) buffer[3*2]|=1<<lookup1[i];
		if (font[(int)string[i]]&SEGK) buffer[2*2]|=1<<lookup1[i];
		if (font[(int)string[i]]&SEGM) buffer[0*2]|=1<<lookup2[i];
		if (font[(int)string[i]]&SEGN) buffer[3*2]|=1<<lookup3[i];
		if (font[(int)string[i]]&SEGP) buffer[2*2]|=1<<lookup3[i];
		if (font[(int)string[i]]&SEGQ) buffer[2*2]|=1<<lookup4[i];
		if (font[(int)string[i]]&SEGCOLON) buffer[1*2]|=1<<lookup3[i];
		if (font[(int)string[i]]&SEGDP) buffer[3*2]|=1<<lookup2[i];

	}
}

int main(void) {

	unsigned int lcd_buffer[16];

	lcd_clock_init();

	lcd_pin_init();

	lcd_config();

	/* busy wait forever */
	for(;;) {

	lcd_convert("VINCE ",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("WEAVER",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("ABCDEF",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("GHIJKL",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("MNOPQR",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("STUVWX",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("YZ0123",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);

	lcd_convert("456789",lcd_buffer);
	lcd_display(lcd_buffer);
	delay(500000);






	}

}

static void nmi_handler(void) {
	for(;;);
}

static void hardfault_handler(void) {
	for(;;);
}

extern unsigned long _etext,_data,_edata,_bss_start,_bss_end;

	/* Copy DATA and BSS segments into RAM */
void c_startup(void)	{

	unsigned long *src, *dst;

	/* Copy data segment */
	/* Using linker symbols */
	src = &_etext;
	dst = &_data;
	while(dst < &_edata) *(dst++) = *(src++);

	/* Zero out the BSS */
	src = &_bss_start;
	while(src < &_bss_end) *(src++) = 0;

	/* Call main() */
	main();

}


/* Vector Table */
unsigned int *myvectors[4]
__attribute__ ((section("vectors"))) = {
	(uint32_t *) STACK_TOP,		/* stack pointer      */
	(uint32_t *) c_startup,		/* code entry point   */
	(uint32_t *) nmi_handler,	/* NMI handler        */
	(uint32_t *) hardfault_handler	/* hard fault handler */
};

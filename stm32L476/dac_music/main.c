/* Lab#10 ADC base code */

#include <stdint.h>
#include "stm32l476xx.h"
#include <stdio.h>

#include "lcd.h"

#include "sine_table.h"
#include "freq_lookup.h"

#include "fighting.h"
#include "highwind.h"
#include "koro.h"

/* global variables */
volatile uint32_t TimeDelay;
volatile uint32_t pulse_width=0;
volatile uint32_t last_captured=0;
volatile uint32_t signal_polarity=0;
volatile uint32_t overflows=0;
static int song_offset=0;

#define MAX_SONGS 3
static int which_song=0;
static int song_length=HIGHWIND_SONG_LENGTH;
static unsigned char *len0=highwind_len0,*len1=highwind_len1,*len2=highwind_len2;
static unsigned char *channel0=highwind_channel0,*channel1=highwind_channel1,*channel2=highwind_channel2;

void change_song(int i) {

	if (i==0) {
		song_length=FIGHTING_SONG_LENGTH;
		len0=fighting_len0;
		len1=fighting_len1;
		len2=fighting_len2;
		channel0=fighting_channel0;
		channel1=fighting_channel1;
		channel2=fighting_channel2;
	}
	if (i==1) {
		song_length=HIGHWIND_SONG_LENGTH;
		len0=highwind_len0;
		len1=highwind_len1;
		len2=highwind_len2;
		channel0=highwind_channel0;
		channel1=highwind_channel1;
		channel2=highwind_channel2;
	}
	if (i==2) {
		song_length=KORO_SONG_LENGTH;
		len0=koro_len0;
		len1=koro_len1;
		len2=koro_len2;
		channel0=koro_channel0;
		channel1=koro_channel1;
		channel2=koro_channel2;
	}

	song_offset=0;
}


//void blah(int val) {
//	char string[20];
//
//	sprintf(string,"%d",val);
//}


/* advance definitions */
void System_Clock_Init(void);
void NVIC_SetPriority(int irq, int priority);
void NVIC_EnableIRQ(int irq);

int angle=0,count=0,which=0,countdown=22100;


int calculate_sine(int degrees) {
	int x;

	x=degrees%360;

	if (x<90) return sine_lookup[x];
	if (x<180) return sine_lookup[180-x];
	if (x<270) return 4096-sine_lookup[x-180];
	return 4096-sine_lookup[360-x];
}


struct asdr_type {
	int a;
	int d;
	int s;
	int r;
};

int calc_asdr(int len, struct asdr_type *asdr) {

	asdr->a=(len*15)/16;
	asdr->d=(len*14)/16;
	asdr->s=len/16;

	return 0;
}

int scale_output(int value,int scale) {

	int temp;

	temp=value-0x800;

	temp*=scale;
	temp/=16;

	temp+=0x800;

	return temp;
}



/* Interrupt Handlers */
void TIM4_IRQHandler(void) {

//	int stepsize=3576;
//	int f,stepsize;

	static int f0,stepsize0=0,angle0=0,l0=0,newstep0=0,c0_done=0,scale0=16;
	static int f1,stepsize1=0,angle1=0,l1=0,newstep1=0,c1_done=0,scale1=16;
	static int f2,stepsize2=0,angle2=0,l2=0,newstep2=0,c2_done=0,scale2=16;
	static int output1,output2,output3;
	static struct asdr_type asdr0={0,0,0,0},asdr1={0,0,0,0},asdr2={0,0,0,0};
	int output;

#define TEMPO 1600
#define FREQUENCY	20000
#define FIXED_CONSTANT	1024

	/* Check if capture happened */
	if ((TIM4->SR & TIM_SR_CC1IF)!=0) {


		if (song_offset%TEMPO==0) {

		which=song_offset/TEMPO;

		if (channel0[which]!=0) {
			f0=(FREQUENCY*FIXED_CONSTANT)/freq_lookup[channel0[which]];
			newstep0=360*FIXED_CONSTANT*FIXED_CONSTANT/f0;
                        l0=len0[which]*(TEMPO/6);
			c0_done=1;
			GPIOB->ODR|=1<<2;
		}
		else {
			GPIOB->ODR&=~(1<<2);
		}

		if (channel1[which]!=0) {
                        f1=(FREQUENCY*FIXED_CONSTANT)/freq_lookup[channel1[which]];
                        newstep1=360*FIXED_CONSTANT*FIXED_CONSTANT/f1;
                        l1=len1[which]*(TEMPO/6);
			c1_done=1;
			GPIOE->ODR|=1<<8;
                }
		else {
			GPIOE->ODR&=~(1<<8);
		}

		if (channel2[which]!=0) {
                        f2=(FREQUENCY*FIXED_CONSTANT)/freq_lookup[channel2[which]];
                        newstep2=360*FIXED_CONSTANT*FIXED_CONSTANT/f2;
                        l2=len2[which]*(TEMPO/6);
			c2_done=1;
                }
		}

		/* count down length */
		if (l0>0) {
                        l0--;
			if (l0==0) {
	                        newstep0=0;
        	                c0_done=1;
			}
                }

                if (l1>0) {
                        l1--;
			if (l1==0) {
				newstep1=0;
				c1_done=1;
			}
		}

		if (l2>0) {
                        l2--;
			if (l2==0) {
				newstep2=0;
				c2_done=1;
                	}
		}

		/* only change frequencies at a zero crossing */

		if ((angle0+stepsize0>=360*FIXED_CONSTANT) || (stepsize0==0)) {
			if (c0_done) {
				stepsize0=newstep0;
				if (stepsize0==0) angle0=0;
				calc_asdr(l0,&asdr0);
				c0_done=0;
			}
			if (l0>asdr0.a) {
				scale0=15;
			}
			else if (l0>asdr0.d) {
				scale0=16;
			}
			else if (l0>asdr0.s) {
				scale0=15;
			}
			else {
				scale0=10;
			}

                }

		if (angle0+stepsize0>=360*FIXED_CONSTANT) {
			angle0-=360*FIXED_CONSTANT;
		}
		angle0+=stepsize0;


		if ((angle1+stepsize1>=360*FIXED_CONSTANT) || (stepsize1==0)) {
			if (c1_done) {
				stepsize1=newstep1;
				if (stepsize1==0) angle1=0;
				calc_asdr(l1,&asdr1);
				c1_done=0;
			}
			if (l1>asdr1.a) {
				scale1=15;
			}
			else if (l1>asdr1.d) {
				scale1=16;
			}
			else if (l1>asdr1.s) {
				scale1=15;
			}
			else {
				scale1=10;
			}

                }

		if (angle1+stepsize1>=360*FIXED_CONSTANT) {
			angle1-=360*FIXED_CONSTANT;
		}
		angle1+=stepsize1;


		if ((angle2+stepsize2>=360*FIXED_CONSTANT) || (stepsize2==0)) {
			if (c2_done) {
				stepsize2=newstep2;
				if (stepsize2==0) angle2=0;
				calc_asdr(l2,&asdr2);
				c2_done=0;
			}
			if (l2>asdr2.a) {
				scale2=15;
			}
			else if (l2>asdr2.d) {
				scale2=16;
			}
			else if (l2>asdr2.s) {
				scale2=15;
			}
			else {
				scale2=10;
			}

                }

		if (angle2+stepsize2>=360*FIXED_CONSTANT) {
			angle2-=360*FIXED_CONSTANT;
		}
		angle2+=stepsize2;




//              printf("ANGLE0=%d\n",angle0);
//              printf("ANGLE1=%d (%x)\n",angle1,calculate_sine(angle1/1000));

                output1=calculate_sine(angle0/FIXED_CONSTANT);
		output1=scale_output(output1,scale0);

                output2=calculate_sine(angle1/FIXED_CONSTANT);
		output2=scale_output(output2,scale1);

                output3=calculate_sine(angle2/FIXED_CONSTANT);
		output3=scale_output(output3,scale2);

		output=(output1+output2+output3)/3;

                DAC->DHR12R2=output;

      //          DAC->DHR12R2=((output1+output2+output3));

		/* clear */
		TIM4->SR &= ~TIM_SR_CC1IF;

		song_offset++;
		if (song_offset>song_length*TEMPO) {
			which_song++;
			if (which_song>=MAX_SONGS) {
				which_song=0;
			}
			change_song(which_song);
			song_offset=0;
		}
	}

	/* Check if overflow happened */
	if ((TIM4->SR & TIM_SR_UIF)!=0) {
		TIM4->SR &= ~TIM_SR_UIF;
	}
}

void SysTick_Handler(void) {

	if (TimeDelay > 0) TimeDelay--;
}



static void TIM4_Init(void) {

	/* enable Timer 4 clock */
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

	/* Edge aligned mode */
	TIM4->CR1 &= ~TIM_CR1_CMS;

	/* Counting direction: 0=up, 1=down */
	TIM4->CR1 &= ~TIM_CR1_DIR; // up-counting

	/* Master mode selection */
	/* 100 = OC1REF as TRGO */
	TIM4->CR2 &= ~TIM_CR2_MMS;
	TIM4->CR2 |= TIM_CR2_MMS_2;

	/* Trigger interrupt enable */
	TIM4->DIER |= TIM_DIER_TIE;

	/* Update interrupt enable */
	TIM4->DIER |= TIM_DIER_UIE;

	/* OC1M: Output Compare 1 mode */
	/* 0110 = PWM mode 1 */
	TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

	/* FIXME */
	/* Prescaler. slow down the input clock by a factor of (1+prescaler) */
	TIM4->PSC=31;		// 16MHz / (1+15) = 1MHz

	/* Auto-reload, max */
	TIM4->ARR=24;		// 1MHz /25 = 40kHz

	/* Duty Ratio */
	TIM4->CCR1 = 12;	// 50%

	/* OC1 signal output on the corresponding output pin*/
	TIM4->CCER |= TIM_CCER_CC1E;

	/* Enable Timer */
	TIM4->CR1 |= TIM_CR1_CEN;

	/* Set highest priority intterupt */
	NVIC_SetPriority(TIM4_IRQn, 0);

	/* Set highest priority intterupt */
	NVIC_EnableIRQ(TIM4_IRQn);

}

/* Example 21-5 in book */

/* DAC Channel 2: DAC_OUT2 = PA5 */
void DAC2_Channel2_Init(void) {

	/* Enable DAC clock */
	RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;

	/* Disable DACs so we can program them */
	DAC->CR &= ~(DAC_CR_EN1 | DAC_CR_EN2);

	/* DAC mode control register */
	/* 000 = conected to external pin with buffer enabled */
	DAC->MCR &= ~DAC_MCR_MODE2;

	/* Enable trigger for DAC channel 2 */
	DAC->CR |= DAC_CR_TEN2;

#if 0
	/* Select software trigger */
	DAC->CR |= DAC_CR_TSEL2;
#endif
	/* Select TIM4_TRG0) as the trigger for DAC channel 2 */
	DAC->CR &= ~DAC_CR_TSEL2;
	DAC->CR |= (DAC_CR_TSEL2_0 | DAC_CR_TSEL2_2);

	/* Enable DAC Channel 2 */
	DAC->CR |= DAC_CR_EN2;

	/* Enable the clock of GPIO port A */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	/* Set I/O mode of pin A5 as analog */
	GPIOA->MODER |= 3U<<(2*5);

}


static void GPIOB_Clock_Enable(void) {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
}

static void GPIOB_Pin_Output_Init(int pin) {

        /* Set mode of pin 2 as output */
        /* 00 = input
           01 = output
           10 = alternate
           11 = analog (default) */

        GPIOB->MODER &= ~(3UL<<(pin*2));
        GPIOB->MODER |= 1UL<<(pin*2);

        /* set output type of pin2 as push-pull */
        /* 0 = push-pull (default)
           1 = open-drain */
        GPIOB->OTYPER &= ~(1<<pin);
        /* Set output speed of pin2 as low */
        /* 00 = Low speed
           01 = Medium speed
           10 = Fast speed
           11 = High speed */
        GPIOB->OSPEEDR &= ~(3UL<<(pin*2));

        /* Set pin 2 as no pull-up, no pull-down */
        /* 00 = no pull-up, no pull-down
           01 = pull-up
           10 = pull-down
           11 = reserved */
        GPIOB->PUPDR &=~(3UL<<(pin*2));
}


static void GPIOE_Clock_Enable(void) {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
}

static void GPIOE_Pin_Output_Init(int pin) {

        /* Set mode of pin 2 as output */
        /* 00 = input
           01 = output
           10 = alternate
           11 = analog (default) */

        GPIOE->MODER &= ~(3UL<<(pin*2));
        GPIOE->MODER |= 1UL<<(pin*2);

        /* set output type of pin2 as push-pull */
        /* 0 = push-pull (default)
           1 = open-drain */
        GPIOE->OTYPER &= ~(1<<pin);
        /* Set output speed of pin2 as low */
        /* 00 = Low speed
           01 = Medium speed
           10 = Fast speed
           11 = High speed */
        GPIOE->OSPEEDR &= ~(3UL<<(pin*2));
       /* Set pin 2 as no pull-up, no pull-down */
        /* 00 = no pull-up, no pull-down
           01 = pull-up
           10 = pull-down
           11 = reserved */
        GPIOE->PUPDR &=~(3UL<<(pin*2));
}



static void GPIOA_Clock_Enable(void) {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
}

static void GPIOA_Pin_Input_Init(int pin) {

        /* Set mode to input 00 */
        /* 00 = input
           01 = output
           10 = alternate
           11 = analog (default) */

        GPIOA->MODER &= ~(3UL<<(pin*2));

        /* Set output speed of pin2 as low */
        /* 00 = Low speed
           01 = Medium speed
           10 = Fast speed
           11 = High speed */
        //GPIOA->OSPEEDR &= ~(3UL<<(pin*2));

        /* Set pin 2 as pull-down */
/* 00 = no pull-up, no pull-down
           01 = pull-up
           10 = pull-down
           11 = reserved */
        GPIOA->PUPDR &=~(3UL<<(pin*2));
        GPIOA->PUPDR |= (2UL<<(pin*2));
}

int main(void) {

	System_Clock_Init();

	/* Enable GPIO-PA3 -- up on joystick */
	GPIOA_Clock_Enable();
	GPIOA_Pin_Input_Init(3);

	/* RED LED is GPIO-PB2 */
        GPIOB_Clock_Enable();
        GPIOB_Pin_Output_Init(2);

        /* GREEN LED is GPIO-PE8 */
        GPIOE_Clock_Enable();
        GPIOE_Pin_Output_Init(8);

	DAC2_Channel2_Init();

	change_song(0);

//	PA1_Analog_Pin_Init();
//	ADC_Init();
//	PE11_Tim1_Pin_Init();
//	TIM1_Init();
	TIM4_Init();
//	SysTick_Initialize(15999);	// 16MHz/16000 = 1kHz, (1ms)

	/* Set up LCD */
	LCD_Clock_Init();
	LCD_Pin_Init();
	LCD_Configure();

	volatile int d;


	asm volatile ( "cpsie i" );

	while(1) {

		if (which_song==0) {

			LCD_Display_String("STILL");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("MORE");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("FIGHTNG");
			for(d=0;d<1000000;d++) ;
		}


		if (which_song==1) {

			LCD_Display_String("HIGH");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("WIND");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("THEME");
			for(d=0;d<1000000;d++) ;
		}


		if (which_song==2) {

			LCD_Display_String("KORO");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("BEIN");
			for(d=0;d<1000000;d++) ;

			LCD_Display_String("IKI");
			for(d=0;d<1000000;d++) ;
		}

		if (GPIOA->IDR & (1<<3)) {
			which_song++;
			if (which_song>=MAX_SONGS) {
				which_song=0;
			}
			change_song(which_song);
		}



//		while((DAC->SR & DAC_SR_BWST2)!=0);

//		DAC->DHR12R2=output;

		/* Start software trigger */
//		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG2;

//		delay(10);
//		output++;
//		if (output>4096) output=0;
	}


}

/* Note: no need to touch the code beyond this point */


/* Set 16MHz HSI clock */
void System_Clock_Init(void) {

	/* Note, this code initializes the HSI 16MHz clock */

        /* Enable the HSI clock */
        RCC->CR |= RCC_CR_HSION;

	/* Wait until HSI is ready */
	while ( (RCC->CR & RCC_CR_HSIRDY) == 0 );

	/* Select HSI as system clock source  */
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;  /* 01: HSI16 oscillator used as system clock */

	/* Wait till HSI is used as system clock source */
//	while ((RCC->CFGR & RCC_CFGR_SWS) == 0 );

}

void NVIC_SetPriority(int irq, int priority) {

	if (irq<0) {
		// for -1, 0xff -> f -> -4 = b (11)
		SCB->SHP[(((uint8_t)irq)&0xf)-4]=(priority<<4)&0xff;
	}
	else {
		NVIC->IP[irq]=(priority<<4)&0xff;
	}

	return;
}

void NVIC_EnableIRQ(int irq) {

	NVIC->ISER[irq >> 5] = (1UL << (irq & 0x1F));

	return;
}



static void nmi_handler(void) {
	for(;;);
}

static void hardfault_handler(void) {
	for(;;);
}

extern unsigned long _etext,_data,_edata,_bss_start,_bss_end;

	/* Copy DATA and BSS segments into RAM */
void Reset_Handler(void)	{

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

#define STACK_LOCATION (0x20000000+(96*1024))

/* Vector Table */
uint32_t *myvectors[256]
__attribute__ ((section(".isr_vector"))) = {
	(uint32_t *) STACK_LOCATION,	/*   0:00  = stack pointer	*/
	(uint32_t *) Reset_Handler,	/* -15:04  = code entry point	*/
	(uint32_t *) nmi_handler,	/* -14:08  = NMI handler	*/
	(uint32_t *) hardfault_handler,	/* -13:0c = hard fault handler	*/
	(uint32_t *) nmi_handler,	/* -12:10 = MemManage		*/
	(uint32_t *) nmi_handler,	/* -11:14 = BusFault		*/
	(uint32_t *) nmi_handler,	/* -10:18 = UsageFault		*/
	(uint32_t *) nmi_handler,	/*  -9:1c = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -8:20 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -7:24 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -6:28 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -5:2c = SVC Handler		*/
	(uint32_t *) nmi_handler,	/*  -4:30 = Debugmon		*/
	(uint32_t *) nmi_handler,	/*  -3:34 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -2:38 = PendSV		*/
	(uint32_t *) SysTick_Handler,	/*  -1:3c = SysTick		*/
	(uint32_t *) nmi_handler,	/*   0:40 = WWDG		*/
	(uint32_t *) nmi_handler,	/*   1:44 = PVD_PVM		*/
	(uint32_t *) nmi_handler,	/*   2:48 = RTC_TAMP_STAMP	*/
	(uint32_t *) nmi_handler,	/*   3:4C = RTC_WKUP		*/
	(uint32_t *) nmi_handler,	/*   4:50 = FLASH		*/
	(uint32_t *) nmi_handler,	/*   5:54 = RCC			*/
	(uint32_t *) nmi_handler,	/*   6:58 = EXTI0		*/
	(uint32_t *) nmi_handler,	/*   7:5c = EXTI1		*/
	(uint32_t *) nmi_handler,	/*   8:60 = EXTI2		*/
	(uint32_t *) nmi_handler,	/*   9:64 = EXTI3		*/
	(uint32_t *) nmi_handler,	/*  10:68 = EXTI4		*/
	(uint32_t *) nmi_handler,	/*  11:6C = DMA1_CH1		*/
	(uint32_t *) nmi_handler,	/*  12:70 = DMA1_CH2		*/
	(uint32_t *) nmi_handler,	/*  13:74 = DMA1_CH3		*/
	(uint32_t *) nmi_handler,	/*  14:78 = DMA1_CH4		*/
	(uint32_t *) nmi_handler,	/*  15:7c = DMA1_CH5		*/
	(uint32_t *) nmi_handler,	/*  16:80 = DMA1_CH6		*/
	(uint32_t *) nmi_handler,	/*  17:84 = DMA1_CH7		*/
	(uint32_t *) nmi_handler,	/*  18:84 = ADC1_2		*/
	(uint32_t *) nmi_handler,	/*  19:88 = CAN1_TX		*/
	(uint32_t *) nmi_handler,	/*  20:90 = CAN1_RX0		*/
	(uint32_t *) nmi_handler,	/*  21:94 = CAN1_RX1		*/
	(uint32_t *) nmi_handler,	/*  22:98 = CAN1_SCE		*/
	(uint32_t *) nmi_handler,	/*  23:9C = EXTI9_5		*/
	(uint32_t *) nmi_handler,	/*  24:A0 = TIM1_BRK/TIM15	*/
	(uint32_t *) nmi_handler,	/*  25:A4 = TIM1_UP/TIM16	*/
	(uint32_t *) nmi_handler,	/*  26:A8 = TIM1_TRG_COM/TIM17	*/
	(uint32_t *) nmi_handler,	/*  27:AC = TIM1_CC		*/
	(uint32_t *) nmi_handler,	/*  28:B0 = TIM2		*/
	(uint32_t *) nmi_handler,	/*  29:B4 = TIM3		*/
	(uint32_t *) TIM4_IRQHandler,	/*  30:B8 = TIM4		*/
	(uint32_t *) nmi_handler,	/*  31:BC = I2C1_EV		*/
	(uint32_t *) nmi_handler,	/*  32:C0 = I2C1_ER		*/
	(uint32_t *) nmi_handler,	/*  33:C4 = I2C2_EV		*/
	(uint32_t *) nmi_handler,	/*  34:C8 = I2C2_ER		*/
	(uint32_t *) nmi_handler,	/*  35:CC = SPI1		*/
	(uint32_t *) nmi_handler,	/*  36:D0 = SPI2		*/
	(uint32_t *) nmi_handler,	/*  37:D4 = USART1		*/
	(uint32_t *) nmi_handler,	/*  38:D8 = USART2		*/
	(uint32_t *) nmi_handler,	/*  39:DC = USART3		*/
	(uint32_t *) nmi_handler,	/*  40:E0 = EXTI5_10		*/
	(uint32_t *) nmi_handler,	/*  41:E4 = RTC_ALART		*/
	(uint32_t *) nmi_handler,	/*  42:E8 = DFSDM1_FLT3		*/
	(uint32_t *) nmi_handler,	/*  43:EC = TIM8_BRK		*/
	(uint32_t *) nmi_handler,	/*  44:F0 = TIM8_UP		*/
	(uint32_t *) nmi_handler,	/*  45:F4 = TIM8_TRG_COM	*/
	(uint32_t *) nmi_handler,	/*  46:F8 = TIM8_CC		*/
	(uint32_t *) nmi_handler,	/*  47:FC = ADC3		*/
	(uint32_t *) nmi_handler,	/*  48:100 = FMC		*/
	(uint32_t *) nmi_handler,	/*  49:104 = SDMMC1		*/
	(uint32_t *) nmi_handler,	/*  50:108 = TIM5		*/
	(uint32_t *) nmi_handler,	/*  51:10C = SPI3		*/
	(uint32_t *) nmi_handler,	/*  52:110 = UART4		*/
	(uint32_t *) nmi_handler,	/*  53:114 = UART5		*/
	(uint32_t *) nmi_handler,	/*  54:118 = TIM6_DACUNDER	*/
	(uint32_t *) nmi_handler,	/*  55:11C = TIM7		*/
	(uint32_t *) nmi_handler,	/*  56:120 = DMA2_CH1		*/
	(uint32_t *) nmi_handler,	/*  57:124 = DMA2_CH2		*/
	(uint32_t *) nmi_handler,	/*  58:128 = DMA2_CH3		*/
	(uint32_t *) nmi_handler,	/*  59:12C = DMA2_CH4		*/
	(uint32_t *) nmi_handler,	/*  60:130 = DMA2_CH5		*/
	(uint32_t *) nmi_handler,	/*  61:134 = DFSDM1_FLT0	*/
	(uint32_t *) nmi_handler,	/*  62:138 = DFSDM1_FLT1	*/
	(uint32_t *) nmi_handler,	/*  63:13C = DFSDM1_FLT2	*/
	(uint32_t *) nmi_handler,	/*  64:140 = COMP		*/
	(uint32_t *) nmi_handler,	/*  65:144 = LPTIM1		*/
	(uint32_t *) nmi_handler,	/*  66:148 = LPTIM2		*/
	(uint32_t *) nmi_handler,	/*  67:14C = OTG_FS		*/
	(uint32_t *) nmi_handler,	/*  68:150 = DMA2_CH6		*/
	(uint32_t *) nmi_handler,	/*  69:154 = DMA2_CH7		*/
	(uint32_t *) nmi_handler,	/*  70:158 = LPUART1		*/
	(uint32_t *) nmi_handler,	/*  71:15C = QUADSPI		*/
	(uint32_t *) nmi_handler,	/*  72:160 = I2C3_EV		*/
	(uint32_t *) nmi_handler,	/*  73:164 = I2C3_ER		*/
	(uint32_t *) nmi_handler,	/*  74:168 = SAI1		*/
	(uint32_t *) nmi_handler,	/*  75:16C = SAI2		*/
	(uint32_t *) nmi_handler,	/*  76:170 = SWPMI1		*/
	(uint32_t *) nmi_handler,	/*  77:174 = TSC		*/
	(uint32_t *) nmi_handler,	/*  78:178 = LCD		*/
	(uint32_t *) nmi_handler,	/*  79:17C = AES		*/
	(uint32_t *) nmi_handler,	/*  80:180 = RNG		*/
	(uint32_t *) nmi_handler,	/*  81:184 = FPU		*/
};

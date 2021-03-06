#include "Board.h"
#include "stm32f4xx.h"
#include "Pins.h"
#include "i2c.h"
#include "Interrupts.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"
#include "Regulator.h"
#include "Manipulators.h"
#include "Dynamixel_control.h"

uint16_t adcData[10];
uint8_t pinType[10];
uint8_t extiType[10];
uint16_t extiFlag;

uint32_t * PWM_CCR[10] ={BTN1_CCR,BTN2_CCR,BTN3_CCR,BTN4_CCR,BTN5_CCR,
                          BTN6_CCR,BTN7_CCR,BTN8_CCR,BTN9_CCR,BTN10_CCR};  //�������� ��������� ������� ���
uint32_t  PWM_DIR[10] ={BTN1_DIR_PIN, BTN2_DIR_PIN,
                          BTN3_DIR_PIN,BTN4_DIR_PIN,
                          BTN5_DIR_PIN,BTN6_DIR_PIN,
                          BTN7_DIR_PIN,BTN8_DIR_PIN,
                          BTN9_DIR_PIN,BTN10_DIR_PIN};
uint32_t  GENERAL_PIN[10] ={GENERAL_PIN_0,GENERAL_PIN_1,
                            GENERAL_PIN_2,GENERAL_PIN_3,
                            GENERAL_PIN_4,GENERAL_PIN_5,
                            GENERAL_PIN_6,GENERAL_PIN_7,
                            GENERAL_PIN_8,GENERAL_PIN_9};
uint32_t  EXTI_PIN[10] ={EXTI1_PIN,EXTI2_PIN,
                         EXTI3_PIN,EXTI4_PIN,
                         EXTI5_PIN,EXTI6_PIN,
                         EXTI7_PIN,EXTI8_PIN,
                         EXTI9_PIN,EXTI10_PIN};
uint32_t  V12_PIN[6] ={PIN5_12V,PIN6_12V,
                            PIN3_12V,PIN4_12V,
                            PIN5_12V,PIN6_12V};


////////////////////////////////////////////////////////////////////////////////
//___________________________PWM CONTROS______________________________________//
////////////////////////////////////////////////////////////////////////////////
char setVoltage(char ch, float duty) // ���������� ���������� �� ������ ���������� ���������� -1,0 .. 1,0
{
    if (ch == 255)
        return 0;
    if (duty > 1 )duty = 1;
    if (duty < -1 )duty = -1;
    if (duty < 0)
    {
          *PWM_CCR[ch] = (int32_t)(MAX_PWM +  (duty * MAX_PWM));
          set_pin(PWM_DIR[ch]);
    }
  else
    {
          *PWM_CCR[ch] = (int32_t) (duty * MAX_PWM);
          reset_pin(PWM_DIR[ch]);
    }
    return 0;
}


char setVoltageMaxon (char ch, float pwm_dir , float duty) // ���������� ���������� �� ������ ���������� ���������� -1,0 .. 1,0
{
    if (ch == 255)
        return 0;
    if (duty > 0.9 ) duty = 0.9;
    if (duty < 0.1 ) duty = 0.1;
    if (pwm_dir > 0)
    {
            *PWM_CCR[ch] = (int32_t) (duty * MAX_PWM);
            set_pin(PWM_DIR[ch]);
    }
    if (pwm_dir < 0)
    {
          *PWM_CCR[ch] = (int32_t) (duty * MAX_PWM);
          reset_pin(PWM_DIR[ch]);
    }
    return 0;
}

char setSpeedMaxon(char ch, float targetSpeed) // V can be positive and negative
{
    float pwm_dir = 0;
    if (targetSpeed > 0)
    {
        pwm_dir = 1;
    }
    else
    {
        pwm_dir = -1;
    }
    float pwm =  (MAX_MAXON_PWM - MIN_MAXON_PWM) * REDUCTION * 60 * fabs(targetSpeed) / (MAX_RPM * 2.0 * PI * RO)   + MIN_MAXON_PWM;
    setVoltageMaxon(ch, pwm_dir,  pwm);
}


char setPWM(char ch, float duty) // ���������� ���������� �� ������ ���  0 .. 1,0
{
    if (duty > 1 ) duty = 1;
    if (duty < 0 ) duty = 0;
    *PWM_CCR[ch] = (int32_t)((duty * (MAX_PWM-1)));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//__________________________________EXTI______________________________________//
////////////////////////////////////////////////////////////////////////////////
void add_ext_interrupt(unsigned char pin, char edge)
{
 SYSCFG->EXTICR [((pin)&0xF) >> 0x02] |= (pin>>4) << ((((pin)&0xF) % 4) << 0x02);
 EXTI->IMR |= (1<<((pin)&0xF));

 switch (edge)
 	{
		case EXTI_FALLING_EDGE:
		  {
		  	 EXTI->FTSR |= (1<<((pin)&0xF));
		  	 EXTI->RTSR &= ~(1<<((pin)&0xF));
        break;
		  }

		case EXTI_RISING_EDGE:
		  {
             EXTI->FTSR &= ~(1<<((pin)&0xF));
		  	 EXTI->RTSR |= (1<<((pin)&0xF));
			 break;
		  }

		case EXTI_BOTH_EDGES:
		  {
		  	 EXTI->RTSR |= (1<<((pin)&0xF));
			 EXTI->FTSR |= (1<<((pin)&0xF));
			 break;
		  }
	}
}
void clear_ext_interrupt(unsigned char pin)
{
 SYSCFG->EXTICR [((pin)&0xF) >> 0x02] &= ~((pin>>4) << ((((pin)&0xF) % 4) << 0x02));
 EXTI->IMR &= ~(1<<((pin)&0xF));

}


/////////////////////////////////////////////////////////////////////////////
void initAll(void)
{

RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
PWR->CR|=PWR_CR_DBP;
__disable_irq();

initRegulators();
initCubeCatcherPID();

//___CLOCKS_________________________________________________________________
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);   // PORTA
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // PORTA
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); // PORTB
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); // PORTC
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // PORTD
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); // PORTE

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,  ENABLE); // TIM1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,  ENABLE); // TIM2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,  ENABLE); // TIM3
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,  ENABLE); // TIM4
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,  ENABLE); // TIM6
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,  ENABLE); // TIM7
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,  ENABLE); // TIM8
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9,  ENABLE); // TIM9
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE); // TIM10
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE); // TIM11
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE); // TIM12
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE); // TIM13
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,  ENABLE); // ADC1


  RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE); // USART1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE); // USART3

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,  ENABLE); // DMA1
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,  ENABLE); // DMA2

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE); // SYSCFG

//___GPIO___________________________________________________________________

  conf_pin(BTN1_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN2_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN3_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN4_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN5_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN6_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN7_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN8_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN9_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(BTN10_DIR_PIN, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(PWM_INHIBIT, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);


//___12V pin_____________________________________________________________

  conf_pin(PIN1_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);
  conf_pin(PIN2_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);
  conf_pin(PIN3_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);
  conf_pin(PIN4_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);
  conf_pin(PIN5_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);
  conf_pin(PIN6_12V, GENERAL, PUSH_PULL, HIGH_S, NO_PULL_UP);


//___PWM_TIM________________________________________________________________

  conf_pin(BTN1_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 1 ���
  conf_af(BTN1_PWM_PIN, AF2);
  conf_pin(BTN2_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 2 ���
  conf_af(BTN2_PWM_PIN, AF2);
  conf_pin(BTN3_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 3 ���
  conf_af(BTN3_PWM_PIN, AF2);
  conf_pin(BTN4_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 4 ���
  conf_af(BTN4_PWM_PIN, AF2);
  conf_pin(BTN5_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 5 ���
  conf_af(BTN5_PWM_PIN, AF3);
  conf_pin(BTN6_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 6 ���
  conf_af(BTN6_PWM_PIN, AF3);
  conf_pin(BTN7_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 7 ���
  conf_af(BTN7_PWM_PIN, AF3);
  conf_pin(BTN8_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 8 ���
  conf_af(BTN8_PWM_PIN, AF3);
  conf_pin(BTN9_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 9 ���
  conf_af(BTN9_PWM_PIN, AF9);
  conf_pin(BTN10_PWM_PIN, ALTERNATE, PUSH_PULL, FAST_S, NO_PULL_UP); // 10 ���
  conf_af(BTN10_PWM_PIN, AF9);


  timPWMConfigure(TIM4, 7, MAX_PWM, 1, 1, 1, 1);
  //timPWMConfigure(TIM11, 2*33600, MAX_PWM, 1, 0, 0, 0); Maxons
  timPWMConfigure(TIM11, 14, MAX_PWM, 1, 0, 0, 0);
  timPWMConfigure(TIM10, 14, MAX_PWM, 1, 0, 0, 0);
  timPWMConfigure(TIM9, 2*1667, MAX_PWM, 1, 1, 0, 0); // 50Hz
  //timPWMConfigure(TIM12, 2*33600, MAX_PWM, 1, 1, 0, 0); // 2.5kHz Maxons
  timPWMConfigure(TIM12, 7, MAX_PWM, 1, 1, 0, 0);

//___PID_TIM________________________________________________________________

  timPIDConfigure(TIM6, 13107, 64);// 20Hz
  NVIC_EnableIRQ(TIM6_DAC_IRQn);

//___REGULATOR_TIM__________________________________________________________

  timPIDConfigure(TIM7, 10, 255);// 33 kHz
  NVIC_EnableIRQ(TIM7_IRQn);
 //delay(1000000);

//___TRACK_TIM_______________________________________________________________

  timPIDConfigure(TIM13, 8400, 1000); // 10Hz
  NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
// delay(100000);

//___TIM_ENCODERS___________________________________________________________

  conf_pin(ENCODER1A_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 1 �
  conf_af(ENCODER1A_PIN, AF3);
  conf_pin(ENCODER1B_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 1 B
  conf_af(ENCODER1B_PIN, AF3);

  conf_pin(ENCODER2A_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 2 �
  conf_af(ENCODER2A_PIN, AF1);
  conf_pin(ENCODER2B_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 2 B
  conf_af(ENCODER2B_PIN, AF1);

  conf_pin(ENCODER3A_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 3 �
  conf_af(ENCODER3A_PIN, AF2);
  conf_pin(ENCODER3B_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 3 B
  conf_af(ENCODER3B_PIN, AF2);

  conf_pin(ENCODER4A_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 4 A
  conf_af(ENCODER4A_PIN, AF1);
  conf_pin(ENCODER4B_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);   //������� 4 B
  conf_af(ENCODER4B_PIN, AF1);

  timEncoderConfigure(TIM8);
  timEncoderConfigure(TIM1);
  timEncoderConfigure(TIM3);
  timEncoderConfigure(TIM2);

//___USART __________________________________________________________________

  conf_pin(RX3_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);    // RX
  conf_af(RX3_PIN, AF7);
  conf_pin(TX3_PIN, ALTERNATE, PUSH_PULL, LOW_S, PULL_UP);    // TX
  conf_af(TX3_PIN, AF7);
  uartInit(USART3, 1000000);                                      //�������� USART3
  NVIC_EnableIRQ(USART3_IRQn);             // ���������� ���������� ��� USART3
  USART3->CR3 |= USART_CR3_DMAT;
  //configUsart3DMA();                                 // ��������� DMA ��� USART3
  //configUsart2DMA();

//___ADC____________________________________________________________________

  conf_pin(GENERAL_PIN_0, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_1, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_2, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_3, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_4, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_5, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_6, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_7, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_8, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  conf_pin(GENERAL_PIN_9, ANALOG, PUSH_PULL, FAST_S, NO_PULL_UP);
  adcConfig();
  //NVIC_EnableIRQ(DMA2_Stream0_IRQn);

//___EXTI____________________________________________________________________
  conf_pin(EXTI1_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI2_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI3_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI4_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI5_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI6_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI7_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI8_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI9_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);
  conf_pin(EXTI10_PIN, INPUT, PUSH_PULL, FAST_S, PULL_UP);

  add_ext_interrupt(EXTI1_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI2_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI3_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI4_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI5_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI6_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI7_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI8_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI9_PIN, EXTI_BOTH_EDGES);
  add_ext_interrupt(EXTI10_PIN, EXTI_BOTH_EDGES);
//  NVIC_EnableIRQ(EXTI0_IRQn);
//  NVIC_EnableIRQ(EXTI1_IRQn);
//  NVIC_EnableIRQ(EXTI2_IRQn);
//  NVIC_EnableIRQ(EXTI3_IRQn);         // �������� ��������
//  NVIC_EnableIRQ(EXTI4_IRQn);
//  NVIC_EnableIRQ(EXTI9_5_IRQn);
//  NVIC_EnableIRQ(EXTI15_10_IRQn);

//___Dynamixel IO control____________________________________________________________________
  conf_pin(DYNAMIXEL_IO_CONTROL, GENERAL, PUSH_PULL, FAST_S, NO_PULL_UP);
  set_pin(DYNAMIXEL_IO_CONTROL); // Transmit mode

//___I2C_____________________________________________________________________
//conf_af(I2C_SDA_PIN, AF4);
//conf_pin(I2C_SDA_PIN, ALTERNATE, OPEN_DRAIN, FAST_S, NO_PULL_UP);    //I2C_SDA
//conf_af(I2C_SCL_PIN, AF4);
//conf_pin(I2C_SCL_PIN, ALTERNATE, OPEN_DRAIN, FAST_S, NO_PULL_UP);    //I2C_SCL
//
//RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);  //I2C2
//NVIC_EnableIRQ(I2C2_ER_IRQn);
//NVIC_EnableIRQ(I2C2_EV_IRQn);

closeWall();
initDynamixels();
openCubesCatcher();
closeCone();

__enable_irq();

}
////////////////////////////////////////////////////////////////////////////////

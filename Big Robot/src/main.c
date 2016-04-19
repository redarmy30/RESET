/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/
#include "stm32f4xx_conf.h"

#include "stm32f4xx.h"
#include "Board.h"  //���� �������������

#include "gpio.h" // ������ � ������� �����-������
#include "Pins.h" // ����������� ����� �� �����
#include "Interrupts.h"
#include "regulator.h"  // ���������� �����, ����������, �����������

#include "usart.h" //����� � ������������� ��������
#include "robot.h"  //����������� ������������ ������ � ��� �������� �������
#include "Manipulators.h"

// ����� � �����������
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END;

char command=0;

char mode;

int main(void)
{

   initAll();

      USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS
            USB_OTG_HS_CORE_ID,
#else
            USB_OTG_FS_CORE_ID,
#endif
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);

       //����� ��������� ������������� ������� � ������ ����������� �����������
       command = 0;
       outEnc.adress = 0x02;
       outEnc.sync = 0xAA;
       outEnc.Command =  ENC_SET_CUR_POS;
       outEnc.checkSum = packetCheck((char *) &outEnc,sizeof(outEnc) - 2);
       sendPacket((char *) &outEnc,sizeof(outEnc));
        //            __enable_irq();

     //   char * str ="mobile robot V1.0";

//    char ch = 5;
//    float duty1 = 0.12;
//    float duty2 = 0.045;

uint8_t numberOfCubesCatched1;
  while(1)
  {

        //closeCubesCatcher(&numberOfCubesCatched1);
        //openCubesCatcher();
// tVoltage(WHEELS[0], wheelsPidStruct[0].output);
  //setVoltage(1, duty1);  // ������
    //setVoltage(ch - 1, duty2); // ������

   }
}

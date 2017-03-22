#include "usart.h"	  
#include "Communcation.h"

extern void ReceiveParsing(uint8_t nData);

void UartSendData(uint8_t data)
{
    while((USART1->SR&0X40)==0)
    {
        
    };//ѭ������,ֱ���������   
    USART1->DR = (u8)data;
}


void USART1_IRQHandler(void)
{

	if(USART1->SR&(1<<5))//���յ�����
	{
        
        //PacketParsing(&DataPack,USART1->DR); 
        ReceiveParsing(USART1->DR);        
	}

} 

void UartSendString(const uint8_t *data, uint32_t cnt)
{
    uint32_t i = 0;
    
    
    for(i=0;i<cnt;i++)
    {
        UartSendData(data[i]);
    }
}
//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������
//CHECK OK
//091209
//#define       Usart1_Remap      0
void uart_init(u32 pclk2,u32 bound)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;	   
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
#ifdef  Usart1_Remap
    RCC->APB2ENR|=1<<3;   //ʹ��PORTB��ʱ�� 
    RCC->APB2ENR|=1;   //ʹ��PORTA��ʱ��
    AFIO->MAPR |= 1<<2;
    RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ��
    GPIOB->CRL&=0X00FFFFFF;//IO״̬����
	GPIOB->CRL|=0X8B000000;//IO״̬����
#else    
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ�� 
    GPIOA->CRH&=0XFFFFF00F;//IO״̬����
	GPIOA->CRH|=0X000008B0;//IO״̬����
#endif
    
	
		  
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ	   	   
	//����������
 	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.
#if EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<8;    //PE�ж�ʹ��
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(3,3,USART1_IRQn,2);//��2��������ȼ� 
#endif

}


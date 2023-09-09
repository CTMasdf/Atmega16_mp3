/*
 * mp3_made.c
 *
 * Created: 2022-06-20 오전 2:39:07
 * Author : CTM
 */ 

#include <stdio.h>				//studio 헤더파일
#include <avr/io.h>				//io 헤더파일
#include <util/delay.h>			//딜레이 헤더파일 | delay header file
#include <avr/interrupt.h>		//인터럽트 헤더파일 | interrupt header file

#include "I2C.h"				//I2c 헤더파일
#include "CLCD.h"				//CLCD 헤더파일
#include "DFplayer.h"			//DFplayer 헤더파일


/*************************출력핀 GPIO define | Output pin GPIO define*******************************/
#define		FND1_OFF					PORTC = PORTC | 0x40;	
#define		FND2_OFF					PORTC = PORTC | 0x10;	
#define		FND3_OFF					PORTC = PORTC | 0x08;	
#define		FND4_OFF					PORTD = PORTD | 0x80;	

#define		FND1_ON						PORTC = PORTC &~ (0x40);	
#define		FND2_ON						PORTC = PORTC &~ (0x10);	
#define		FND3_ON						PORTC = PORTC &~ (0x08);	
#define		FND4_ON						PORTD = PORTD &~ (0x80);	


//FND 배열 {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, P, L, A, Y, T}
volatile int num[15] = {0x7E, 0x48, 0x3D, 0x6D, 0x4B, 0x67, 0x77, 0x4E, 0x7f, 0x4F, 0x1F, 0x32, 0x5F, 0x6B, 0x33};
	
//fnd 카운트 변수
volatile unsigned int fndc;

// DFplayer 모듈 음악 PLAY, STOP 핀 변수
volatile unsigned int busy;

//엔코더 변수
volatile unsigned int en1, en2, s_en1, s_en2;

//소리 크기 변수 (초기 소리 값 15 <50%>)
volatile unsigned int sound_setting, sound_setting_1, sound_setting_c;
int sound_number = 15;

//볼륨값 lcd 창에 출력하는 배열
char volume_char[4];	
	
/*음량조절 엔코더 함수*/
void sound_encoder(){
	/*엔코더 함수 선언 | Encoder function declaration*/
	en1 = PINB & 0x02;
	en2 = PINB & 0x04;
		/************소리 크기 설정 엔코더 | table_set_encoder***********/
		if(en1 == 0 && en2 != 0){			//en1이 0, en2가 0이 아닐 때 | If en1 is 0 and en2 is non-zero
			s_en1 = 1;						//en1 = 1;
			s_en2 = 1;						//en2 = 1;
			sound_setting = 1;				//엔코더를 돌린 순간 sound_setting = 1 (음량조절 화면으로 이동)
		}
		else if(en1 != 0 && en2 == 0){		//en1이 0이 아니고 en2가 0일 때 | If en1 is not 0 and en2 is 0
			s_en1 = 0;						//en1 = 0;
			s_en2 = 1;						//en1 = 1;
			sound_setting = 1;				//엔코더를 돌린 순간 sound_setting = 1 (음량조절 화면으로 이동)
		}
	
	/************테이블_설정_엔코더_제어 | table_set_encoder_control***********/
	if (en1 != 0 && en2 != 0 && s_en2 == 1 && sound_setting == 1){
		s_en2 = 0;							
		sound_setting = 0;					
		
		/*엔코더를 정방향으로 돌렸을 때*/
		if(s_en1 == 0){
			sound_number++;								//소리 크기 값 증가
			sound_setting_1 = 1;						//세팅하는 동안 메인화면에 돌아가지 못하도록 하는 변수
			MP3_send_cmd(MP3_VOLUME, 0, sound_number);	//DFplayer 모듈에 볼륨 크기를 sound_number 값 만큼 보내기
			
			/*엔코더를 돌리고 있는 동안에는 메인화면으로 돌아가지 못하도록 카운터가 멈춤.
			lcd 창 잔상 때문에 200ms 후에 카운터 멈춤(메인함수 쪽 while문 안에 있는 코드 참고)*/
			if(sound_setting_c > 200){				
				sound_setting_c = 201;	
			}
		}
		
		/*엔코더를 역방향으로 돌렸을 때*/
		else {
			sound_number--;								//소리 크기 값 감소
			sound_setting_1 = 1;						//세팅하는 동안 메인화면에 돌아가지 못하도록 하는 변수
			MP3_send_cmd(MP3_VOLUME, 0, sound_number);	//DFplayer 모듈에 볼륨 크기를 sound_number 값 만큼 보내기
			
			/*엔코더를 돌리고 있는 동안에는 메인화면으로 돌아가지 못하도록 카운터가 멈춤.
			lcd 창 잔상 때문에 200ms 후에 카운터 멈춤(메인함수 쪽 while문 안에 있는 코드 참고)*/
			if(sound_setting_c > 200){
				sound_setting_c = 201;
			}
		}
	}
	
	/*sound_number 값이 30보다 클 때*/
	if(sound_number >= 30){	
		sound_number = 30;			
	}
	
	/*sound_number 값이 0보다 작을 때*/
	else if(sound_number <= 0){
		sound_number = 0;
	}
		
	/*엔코더를 돌린 순간 일정 시간 후 메인화면으로 돌아가는 카운트 증가*/
	if(sound_setting_1 == 1){
		sound_setting_c++;
	}
	
	/*엔코더로 음량 조절을 완료하고 2000ms가 되면 음량 변수 값 초기화*/
	if(sound_setting_c >= 2000){
		sound_setting_1 = 0;
		sound_setting_c = 0;
	}
}	

/*fnd STOP, PLAY 함수*/
void FND(){
	
	fndc++;	//fnd 카운트 증가
	
	/*FND에 Play 출력*/
	if(busy == 0){
		switch(fndc){
			case 1: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 2: PORTA = num[13];	//Y
			case 3: FND1_OFF; FND2_OFF; FND3_OFF; FND4_ON; break;
			
			case 4: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 5: PORTA = num[12];	//A
			case 6: FND1_OFF; FND2_OFF; FND3_ON; FND4_OFF;break;
			
			case 7: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 8: PORTA = num[11];	//L
			case 9: FND1_OFF; FND2_ON; FND3_OFF; FND4_OFF; break;
			
			case 10: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 11: PORTA = num[10];	//P
			case 12: FND1_ON; FND2_OFF; FND3_OFF; FND4_OFF; break;
			
			case 13: fndc = 1; break;
		}
	}
	
	/*FND에 STOP 출력*/
	else {
		switch(fndc){
			case 1: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 2: PORTA = num[10];	//P
			case 3: FND1_OFF; FND2_OFF; FND3_OFF; FND4_ON; break;
			
			case 4: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 5: PORTA = num[0];		//O
			case 6: FND1_OFF; FND2_OFF; FND3_ON; FND4_OFF;break;
			
			case 7: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 8: PORTA = num[14];	//T
			case 9: FND1_OFF; FND2_ON; FND3_OFF; FND4_OFF; break;
			
			case 10: FND1_OFF; FND2_OFF; FND3_OFF; FND4_OFF; break;
			case 11: PORTA = num[5];	//S
			case 12: FND1_ON; FND2_OFF; FND3_OFF; FND4_OFF; break;
			
			case 13: fndc = 1; break;
		}
	}
}


/*타이머 카운터0 | timer counter 0*/
ISR(TIMER0_OVF_vect)
{
	TCNT0=0x83;				//타이머 카운터0 1ms | Timer counter 0 1ms
	FND();					//FND 함수 선언
	sound_encoder();		//음량조절 엔코더 함수
	
}

int main(void)
{
	/*포트 입출력 설정*/
	DDRA = 0xff; DDRB = 0x00; DDRC = 0xff; DDRD = 0xff;
	
	i2c_lcd_init();						//lcd 창 초기화 
	i2c_lcd_write_string("HELLO!");		//lcd 창에 HELLO 출력
	_delay_ms(1000);					//1초 딜레이
	
	
	//UART 설정 통신속도 9600bps | UART setting baud rate 9600bps
	UCSRA=0x00;
	UCSRB=0x18;
	UCSRC=0x06;
	UBRRH=0x00;
	UBRRL=0x33;
	
	TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<WGM01) | (0<<CS02) | (1<<CS01) | (1<<CS00);
	TCNT0=0x83;		//타이머 카운터0 1ms | Timer counter 0 1ms
	OCR0=0x00;
	
	dfplayer_init();	//DFplayet 모듈 초기화 함수
	
	TIMSK = 0x01;	//TIMER_MSK = 0x01;(타이머 카운터0) | TIMER_MSK = 0x01; (timer counter 0)
	
	sei();	//인터럽트 허용 | Interrupt Allowed
	
    while (1) 
    {
		/*입력핀 GPIO 변수 | Input Pin GPIO Variables*/
		busy = PINB & 0x01;		
		
		
		/*메인화면
		음악이 재생중 일 때			음악이 재생하지 않을 때
		ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ			ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		|DFPLAYER        |			|DFPLAYER        |
		|Music Play		 |			|Music STOP		 |
		ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ			ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		*/		
		
		if(sound_setting_1 == 0){					//sound_setting_1 = 0일 때 메인화면
			i2c_lcd_goto_XY(0,0);					//커서 좌표를 (0,0)으로 이동
			i2c_lcd_write_string("DFPLAYER");		//lcd 창에 "DFPLAYER" 출력
			if(busy == 1){							//음악이 재생하고 있지 않을 때
				i2c_lcd_goto_XY(1,0);				//커서 좌표를 (1,0)으로 이동
				i2c_lcd_write_string("Music STOP");	//lcd 화면에 "Music STOP" 출력
			}
			else{									//음악이 재생하고 있을 때
				i2c_lcd_goto_XY(1,0);				//커서 좌표를 (1,0)으로 이동
				i2c_lcd_write_string("Music Play");	//lcd 화면에 "Music Play" 출력
			}
		}
		
		/*음량 조절 화면
		음악이 재생중 일 때			음악이 재생하지 않을 때
		ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ			ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		|Music Play      |			|Music STOP      |
		|sound : 'value' |			|sound : 'value' |
		ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ			ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
		*/
		
		if(sound_setting_1 == 1){					//sound_setting_1 = 1일 때 음량 조절 화면3
			
			/*lcd 창 잔상 때문에 엔코더 돌린 즉시 200ms 동안 음량조절 lcd 화면에 출력하는 코드*/
			if(sound_setting_c <= 200){				
				i2c_lcd_command(0x01);			//lcd 화면 초기화
				i2c_lcd_goto_XY(0,0);			//lcd 커서 좌표 (0,0)으로 이동
				
				if(busy == 1){								//음악이 재생하지 않을 때
					i2c_lcd_write_string("Music STOP");		//lcd 화면에 "Music STOP" 출력
				}
				else {										//음악이 재생되고 있을 때					
					i2c_lcd_write_string("Music PLAY");		//lcd 화면에 "Music Play" 출력
				}
				i2c_lcd_goto_XY(1,0);						//lcd 커서 좌표 (1,0)으로 이동
				i2c_lcd_write_string("sound : ");			//lcd 화면에 "sound : " 출력
			}
			
			/*200ms가 지나고 난 후 엔코더로 음량조절한 값을 lcd 화면에 출력하는 코드*/
			else if(sound_setting_c > 200 && sound_setting_c < 1800){
				if(sound_number >= 30){							//음량 크기 값이 30, 최대치를 넘으면
					sprintf(volume_char, "%3d", sound_number);	//volume_char 배열에 sound_number값 받기
					i2c_lcd_goto_XY(1,8);						//lcd 화면 커서를 (1,8)로 이동
					i2c_lcd_write_string(volume_char);			//lcd 화면에 음량 값 출력
					i2c_lcd_goto_XY(1,13);						//lcd 화면 커서를 (1,13)로 이동
					i2c_lcd_write_string("MAX");				//lcd 화면에 "MAX" 출력
				}
				else{											//음량 크기 값이 30을 넘지 않으면
					sprintf(volume_char, "%3d", sound_number);	//volume_char 배열에 sound_number값 받기
					i2c_lcd_goto_XY(1,8);						//lcd 화면 커서를 (1,8)로 이동
					i2c_lcd_write_string(volume_char);			//lcd 화면에 음량 값 출력
					i2c_lcd_goto_XY(1,13);						//lcd 화면 커서를 (1,13)로 이동
					i2c_lcd_write_string("     ");				//lcd 화면에 공백 출력
				}
			}
			
			/*lcd 잔상 때문에 1800ms가 될 때 lcd 화면을 초기화 한다. 
			(2000ms가 되면 메인화면으로 복귀) <- sound_encoder 함수 참고*/
			else if(sound_setting_c >= 1800){
				i2c_lcd_command(0x01);	//lcd 화면 초기화   
			}
		}
    }
}


#include <xc.h>
// CONFIG1
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#define _XTAL_FREQ       20000000

#include "lcd.h"
#include "pwm.h"

#define     up    PORTAbits.RA2
#define     dw    PORTAbits.RA3

#define     ss    PORTAbits.RA0
#define     inv   PORTAbits.RA1

long    gt,xung,luu,td;
long    dem,ext;


float   e,e1,e2,a,b,ga,kp,kd,ki,out,lastout,t;
long    dat,pwmduty,chay,duty;
int     tt;
int     tt_ss,tt_inv;

void __interrupt() ISR(void)
{
    
    if(INTCONbits.TMR0IF==1)
    {
       INTCONbits.TMR0IF=0;
       TMR0 = 6;
       //ei();
       dem++;
       chay++;
       if(dem>500) //sau 0.2msx500 = 100ms thi lay xung 1 lan
       {
           dem=0;
           INTCONbits.INTE = 0;
           INTCONbits.TMR0IE=0;
           luu=xung;
           xung=0;
           INTCONbits.INTE = 1;
           INTCONbits.TMR0IE=1;
           tt=1;
       }
    }
    if(INTCONbits.INTF == 1)
    {
        xung++;   //flip the bit
        INTCONbits.INTF = 0;
    }
}

void nn_ss()
{
   if(ss==0)
   {
       __delay_ms(20);
       if(ss==0)
       {
           tt_ss++;
           if(tt_ss>1)        tt_ss=0;
           if(tt_ss==1)         // cho chay   
           {
               if(tt_inv==1)        
               {               
                   PWM2_Stop(); 
                   PORTCbits.RC1=0;
                   PWM1_Duty(pwmduty);
                   PWM1_Start();   
               }
               else                 
               {
                   PWM1_Stop(); 
                   PORTCbits.RC2=0;
                   PWM2_Duty(pwmduty);
                   PWM2_Start();   
               }
           }
           else              
           {
               PWM1_Stop(); 
               PORTCbits.RC2=0; 
               PWM2_Stop(); 
               PORTCbits.RC1=0; 
           }
           while(ss==0);
       }
   }
}

void nn_inv()
{
   if((inv)==0)
   {
      __delay_ms(20);
      if((inv)==0)
      {
         tt_inv++;
         if(tt_inv>1)       tt_inv=0;
         if(tt_inv==1)        
            {
               PWM2_Stop(); 
               PORTCbits.RC1=0;
               PWM1_Duty(pwmduty);
               PWM1_Start();   
            }
            else                 
            {
               PWM2_Duty(pwmduty);
               PWM1_Stop(); 
               PORTCbits.RC2=0;
               PWM2_Duty(pwmduty);
               PWM2_Start();   
            }
         while(inv==0);
      }
   }
}

void nn_up()
{
   if((up)==0)
   {
      __delay_ms(20);
      if((up)==0)
      {
         dat=dat+10; 
         if(dat>280)    dat=280;
         while((up)==0);
      }
   }
}

void nn_dw()
{
   if((dw)==0)
   {
      __delay_ms(20);
      if((dw)==0)
      {
         dat=dat-10;
         if(dat<0)         dat=0;
         while((dw)==0);
      }
   }
}
void tinh_pid()
{
    if(tt==1)
    {
        tt=0;
        td=luu*600/7; //600 = 60*(1/t) voi t = 0.1
        xung=0;
        e=dat-td;
        a=2*t*kp+ki*t*t+2*kd;
        b=t*t*ki-4*kd-2*t*kp;
        ga=2*kd;

        out=(a*e+b*e1+ga*e2+2*t*lastout)/(2*t);

        lastout=out;
        e2=e1;
        e1=e;

        if(out>999)         out=999;
        if(out<0)           out=0;
   
        pwmduty=out;
    }
    if(tt_inv==1)        PWM1_Duty(pwmduty);
    else                 PWM2_Duty(pwmduty);
}


void main(void) 
{
    TRISD=0;
    TRISC=0;
    TRISB=0x01;
    TRISA=0xFF;
    
    ANSEL = 0x00;   //disable all analog ports
    ANSELH = 0x00;
    
    PWM1_Init(1250);// tan so
    
    INTCONbits.GIE=1;           // global
    INTCONbits.PEIE=1;          // 
    INTCONbits.TMR0IE=1;            // timer0 interrup
    INTCONbits.TMR0IF=0;
    
    TMR0 = 6;   
    ei();
    OPTION_REGbits.T0CS=0;
    OPTION_REGbits.T0SE=0;
    OPTION_REGbits.PSA=0;
    //20M/4 = 5M/4=1.25M = f -> T = 1/1.25M = 0.8us
    //Tmax = 0.8 x 256 = 0.2048ms
    //Timer can 0.2ms = (Tmax*(256-TMR0))/256 -> TMR0 = 6
    //bo chia timer 
    OPTION_REGbits.PS0=1; //1/4
    OPTION_REGbits.PS1=0;
    OPTION_REGbits.PS2=0;
    
    //khai bao ngat ngoai
    INTCONbits.INTF = 0;        //reset the external interrupt flag
    OPTION_REGbits.INTEDG = 1;  //interrupt on the rising edge
    INTCONbits.INTE = 1;        //enable the external interrupt
    
    Lcd_Init();
    tt_ss=0;
    tt_inv=1;
    xung=0;
    luu=0;
    td=0;

    e=0;
    e1=0;
    e2=0;
    out=0;
    lastout=0;
    dat=0;
    t=0.1;
    kp=6;
    kd=0.1;
    ki=2.5; 
    
    chay=0;
    while(1)
    {
        tinh_pid();
        if(tt_ss==1)         // cho chay   
         {
            if(tt_inv==1)        
            {
               PWM2_Stop(); 
               PORTCbits.RC1=0;
               PWM1_Duty(pwmduty);
               PWM1_Start();   
            }
            else                 
            {
               PWM1_Stop(); 
               PORTCbits.RC2=0;
               PWM2_Duty(pwmduty);
               PWM2_Start();   
            }
         }
         else
         {
            PWM1_Stop(); 
            PORTCbits.RC2=0; 
            PWM2_Stop(); 
            PORTCbits.RC1=0; 
         }
        if(chay>2500)
        {
            chay=0;
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String("TD :");
            Lcd_Write_Char(td/100%10+0x30);
            Lcd_Write_Char(td/10%10+0x30);
            Lcd_Write_Char(td%10+0x30);
            Lcd_Write_String("(v/p) ");

            Lcd_Set_Cursor(2,1);
            Lcd_Write_String("DAT:");
            Lcd_Write_Char(dat/100%10+0x30);
            Lcd_Write_Char(dat/10%10+0x30);
            Lcd_Write_Char(dat%10+0x30);
            Lcd_Write_String("(v/p)  ");
        }
        if(tt_ss==1)
        {
           Lcd_Set_Cursor(1,14);
           Lcd_Write_String("ON ");
        }
        else
        {
           Lcd_Set_Cursor(1,14);
           Lcd_Write_String("OFF");
        }

        if(tt_inv==1)
        {
           Lcd_Set_Cursor(2,15);
           Lcd_Write_String("T ");
        }
        else
        {
           Lcd_Set_Cursor(2,15);
           Lcd_Write_String("N ");
        }
        nn_ss();
        if(tt_ss==1)      
        {
           nn_inv();
           nn_up();
           nn_dw();
        }
    }
}
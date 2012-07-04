#include <msp430.h>
#define interrupt(x) void __attribute__((interrupt (x)))

/**************************
 * LCD CONNECTIONS
 * D4 - D7 -> P1.0 - P1.3
 * EN -> P1.4
 * RS -> P1.5
 **************************/

#define LCD_PIN P1IN
#define LCD_POUT P1OUT
#define LCD_PDIR P1DIR

#define LCD_SHIFT 0 // LCD pins D4-D7 start on bit LCD_SHIFT of LCD_PIN/POUT
#define LCD_EN BIT4
#define LCD_RS BIT5

#define LCD_DATA (0x0F<<LCD_SHIFT)

#define LED1 BIT0
#define LED2 BIT6

#define HIGH4(x) (0x0F&((x)>>4))
#define LOW4(x) (0x0F&(x))

#define lcd_cmd(x) lcd_send((x),0); delay_us(100);
#define lcd_data(x) lcd_send((x),1); delay_us(10);
#define lcd_clear() lcd_cmd(0x01);

void delay_ms(int x) {
  // ACLK div 8 ~= 4 kHz
  BCSCTL1 |= DIVA_3;
  TA0CCR0 = x;
  TA0CTL = TASSEL_1 | ID_2 | MC_1 | TACLR | TAIE; // ACLK div 4 ~ 1kHz
  LPM3;
}

void delay_us(int x) {
  TA0CCR0 = x;
  TA0CTL = TASSEL_2 | MC_1 | TACLR | TAIE; // SMCLK ~ 1MHz
  LPM1;
}

interrupt(TIMER0_A1_VECTOR) timer0_a1_isr() {
  TA0CTL = 0;
  LPM4_EXIT;
}

void lcd_nib(char data, char mode) {
  LCD_POUT = mode ? (LCD_POUT | LCD_RS) : (LCD_POUT & ~(LCD_RS));
  LCD_POUT &= ~LCD_EN;
  LCD_POUT = (LCD_POUT & ~(LCD_DATA)) | ((data<<LCD_SHIFT) & LCD_DATA);
  LCD_POUT |= LCD_EN;
  delay_us(2);
  LCD_POUT &= ~LCD_EN;
}

void lcd_send(char data, char mode) {
  lcd_nib(HIGH4(data), mode);
  mode?delay_us(200):delay_ms(5);
  lcd_nib(LOW4(data), mode);
  mode?delay_us(200):delay_ms(5);
}

inline void lcd_init() {
  LCD_PDIR |= LCD_DATA | LCD_RS | LCD_EN;
  LCD_POUT &= ~(LCD_DATA | LCD_RS | LCD_EN);
  delay_ms(50);
  lcd_nib(0x3,0);
  delay_ms(5);
  lcd_nib(0x3,0);
  delay_us(200);
  lcd_nib(0x3,0);
  delay_us(200);
  lcd_nib(0x2,0);
  delay_ms(5);
  
  lcd_cmd(0x28);
  lcd_cmd(0x08);
  lcd_clear();
  lcd_cmd(0x06);
  lcd_cmd(0x0C);
}

void lcd_write(char *d) {
  while(*d) {
    lcd_data(*d);
    d++;
  }
}

int main() {
  
  WDTCTL = WDTPW | WDTHOLD;
  DCOCTL = 0;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  _BIS_SR(GIE);
  
  lcd_init();
  
  lcd_write("Hello World!");
  
  LPM4;
  
  return 0;
  
}

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>

static uint8_t TARGET_ADDR = 0x27;


static void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

/*static void cmd(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = b;
}

static void data(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = 0x200 | b;
}

static void init_lcd(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~((3<<(2*12)) | (3<<(2*13)) | (3<<(2*15)) );
    GPIOB->MODER |=   (2<<(2*12)) | (2<<(2*13)) | (2<<(2*15));
    GPIOB->AFR[1] &= ~( (0xf<<(4*(12-8))) | (0xf<<(4*(13-8))) | (0xf<<(4*(15-8))) );

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;

    SPI2->CR1 &= ~SPI_CR1_BR;
    SPI2->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
    SPI2->CR1 |= SPI_CR1_BR;
    SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI2->CR1 |= SPI_CR1_SPE;

    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}

//===========================================================================
// Display a string on line 1 by writing to SPI directly.
static void display1(const char *s) {
    cmd(0x02); // put the cursor on the beginning of the first line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 2 by writing to SPI directly.
static void display2(const char *s) {
    cmd(0xc0); // put the cursor on the beginning of the second line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}
*/
//===========================================================================
// Initialize I2C1 to 400 kHz
void i2c_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER |= 2<<(2*6) | 2<<(2*7);
    GPIOB->AFR[0] |= 1<<(4*6) | 1<<(4*7);

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    //RCC->CFGR3 |= RCC_CFGR3_I2C1SW;  // set for 48MHz sysclk

    // I2C CR1 Config
    I2C1->CR1 &= ~I2C_CR1_PE;           // Disable to perform reset.
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;       // 0: Analog noise filter enabled.
    I2C1->CR1 &= ~I2C_CR1_ERRIE;        // Errors interrupt disable
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;    // Enable clock stretching

    // From table 83. p642 of FRM.  Set for 400 kHz with 8MHz clock.
    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;// Clear prescaler
    I2C1->TIMINGR |= 0 << 28;           // Set prescaler to 0
    I2C1->TIMINGR |= 3 << 20;           // SCLDEL
    I2C1->TIMINGR |= 1 << 16;           // SDADEL
    I2C1->TIMINGR |= 3 << 8;            // SCLH
    I2C1->TIMINGR |= 9 << 0;            // SCLL

    // I2C Own address 1 register (I2C_OAR1)
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 =   I2C_OAR1_OA1EN | (0x1<<1);// Set 7-bit own address 1

    I2C1->CR2 &= ~I2C_CR2_ADD10;        // 0 = 7-bit mode; 1 = 10-bit mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;       // Enable the auto end
    I2C1->CR2 |= I2C_CR2_NACK;          // For slave mode: set NACK

    I2C1->CR1 |= I2C_CR1_PE;            // Enable I2C1
}

//===========================================================================
// Clear NACK condition.
int i2c_checknack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        return 1;
    return 0;
}

//===========================================================================
// Clear NACK condition.
void i2c_clearnack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        I2C1->ICR |= I2C_ICR_NACKCF;
}

//===========================================================================
// Clear STOP condition.
int i2c_checkstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return 1;
    return 0;
}

//===========================================================================
// Clear STOP condition.
void i2c_clearstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        I2C1->ICR |= I2C_ICR_STOPCF;
}

//===========================================================================
// Look at the ISR and display the error.
/*void display_i2c_error(void) {
    if (I2C1->ISR & I2C_ISR_BERR)
        display1("Bus error");
    else if (I2C1->ISR & I2C_ISR_ARLO)
        display1("Arbitration Lost");
    else if (I2C1->ISR & I2C_ISR_TIMEOUT)
        display1("I2C TIMEOUT");
    else if (I2C1->ISR & I2C_ISR_BUSY)
        display1("I2C BUSY");
    else if (I2C1->ISR & I2C_ISR_PECERR)
        display1("I2C PEC Error");
    else if (I2C1->ISR & I2C_ISR_STOPF)
        display1("I2C Stopped");
    else if (I2C1->ISR & I2C_ISR_NACKF)
        display1("I2C NACK");
    else
        display1("???");
}
*/
//===========================================================================
// Generate a start bit.
void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir) {
    // dir: 0 = master requests a write transfer
    // dir: 1 = master requests a read transfer
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES |
                I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN;  // Read from slave
    else
        tmpreg &= I2C_CR2_RD_WRN;  // Write to slave
    tmpreg |= ((devaddr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

//===========================================================================
// Generate a stop bit.
void i2c_stop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return;
    // Master: Generate STOP bit after current byte has been transferred.
    I2C1->CR2 |= I2C_CR2_STOP;
    // Wait until STOPF flag is reset
    while( (I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF; // Write  to clear STOPF flag
}

//===========================================================================
// Check wait for the bus to be idle.
void i2c_waitidle(void) {
    while ( (I2C1->ISR ^ I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

//===========================================================================
// Send multiple bytes to a target slave.
// - Wait for idle.
// - Send data, byte by byte.
// - After all bytes sent, hardware sets the TC flag.
//   Software waits until TC is set.
//   Hardware automatically clears TC flag when START bit or STOP bit in
//   CR2 is set.
int8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size) {
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    // Last argument is dir: 0 = sending data to the slave.
    i2c_start(devaddr, size, 0);

    for(i=0; i<size; i++) {
        // TXIS bit is set by hardware when the TXDR register is empty and the
        // data to be transmitted must be written in the TXDR register.  It is
        // cleared when the next data to be sent is written in the TXDR reg.
        // The TXIS flag is not set when a NACK is received.
        int count = 0;
        while( (I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }

        // TXIS is cleared by writing to the TXDR register.
        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    // Wait until TC flag is set or the NACK flag is set.
    while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ( (I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}


void command(char cmd) {
	i2c_senddata(TARGET_ADDR, cmd, sizeof(cmd));

}
//===========================================================================
// Receive multiple bytes from a target slave.
/*int8_t i2c_recvdata(uint8_t devaddr, void *pdata, uint8_t size) {
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    i2c_start(devaddr, size, 1); // 1 = receiving from the slave
    for(i=0; i<size; i++) {
        // Wait until RXNE flag is set
        int count = 0;
        while ( (I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        udata[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
    }

    int x=0;
    while((I2C1->ISR & I2C_ISR_TC) == 0)
        x++;  // Wait until TCR flag is set

    i2c_stop();
    return 0;
}



//===========================================================================
// Continually read the first four bytes of the EEPROM
void test_eeprom(void) {
    int count = 0;
    for(;;) {
        uint8_t buf[] = {0,0};
        i2c_senddata(0x50, buf, sizeof buf);
        count += 1;
        char line[4];
        if (i2c_recvdata(0x50, line, sizeof line) != 0) {
            display_i2c_error();
        } else {
            char msg[16];
            sprintf(msg, "Read #%d", count);
            display1(msg);
        }
        nano_wait(1000000);  // Wait a ms before reading again.
    }
}

//===========================================================================
// Write 32 bytes to a region of the EEPROM.
void write_eeprom(int addr, const char *msg) {
	char newdata[34];

	newdata[0] = (addr >> 8) & 0xf;
	newdata[1] = addr & 0x0ff;

	int msg_loc;
	for(msg_loc = 0; msg_loc < 32; msg_loc++) {
		if(msg[msg_loc] != '\0') {
			newdata[msg_loc+2] = msg[msg_loc];
		}
		if(msg[msg_loc] == '\0') {
			newdata[msg_loc+2] = '\0';
			break;
		}

	}
	for(msg_loc++; msg_loc < 32; msg_loc++) {
		newdata[msg_loc+2] = ' ';

	}

	i2c_senddata(0x50, newdata, sizeof newdata);
	nano_wait(5000000);

	uint8_t buf[] = {0,0};
	i2c_senddata(0x50, buf, sizeof buf);
	nano_wait(5000000);


}

//===========================================================================
// Read a <255-byte string from an specified address of the EEPROM.
void display_eeprom(int addr, int len) {
    if (len > 255)
        return;
    // Do a zero-byte write to set the current address...
    char buf[255+16] = {0};
    buf[0] = (addr >> 8) & 0x0f;
    buf[1] = addr & 0x0ff;
    i2c_senddata(0x50, buf, 2);

    // Put a string on the front end, so we can slide the string in.
    strcpy(buf, "                ");
    // Read the string.
    if (i2c_recvdata(0x50, &buf[16], len) < 0) {
        display_i2c_error();
        for(;;);
    }
    buf[len+16] = '\0';
    for(;;) {
        int x;
        for(x=0; x<255+16; x+=1) {
            if (buf[x] == 0)
                break;
            display1(&buf[x]);
            nano_wait(100000000);
        }
    }
}*/


//===========================================================================
// The main() function.
int main(void)
{
    //init_lcd();
    //display1("Initializing I2C");
    i2c_init();

    //init_eeprom();
   // test_eeprom();
    //write_eeprom(0,"This is a test of theROM.   ");
    //write_eeprom(32,"This is only a test.");
    //display_eeprom(0,255);
    command(0x01);
    //command(0x)
    for(;;);
}

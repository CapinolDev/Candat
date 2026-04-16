    #include "pico/stdlib.h"
    #include "hardware/spi.h"
    #include <stdio.h>
    #include <string.h>


    #define SPI_PORT spi0
    #define PIN_SCK  2   
    #define PIN_MOSI 3   
    #define PIN_MISO 4   
    #define PIN_CS   5   
    #define PIN_RST  6   
    #define PIN_IRQ  7   

    #define REG_FIFO                 0x00
    #define REG_OP_MODE              0x01
    #define REG_FRF_MSB              0x06
    #define REG_FRF_MID              0x07
    #define REG_FRF_LSB              0x08
    #define REG_PA_CONFIG            0x09
    #define REG_FIFO_ADDR_PTR        0x0D
    #define REG_FIFO_TX_BASE_ADDR    0x0E
    #define REG_IRQ_FLAGS            0x12
    #define REG_PAYLOAD_LENGTH       0x22


    #define REG_FIFO_RX_CURRENT_ADDR 0x10
    #define REG_RX_NB_BYTES          0x13
    #define MODE_RX_CONTINUOUS       0x05
    #define IRQ_RX_DONE_MASK         0x40

    #define MODE_LONG_RANGE_MODE     0x80
    #define MODE_SLEEP               0x00
    #define MODE_STDBY               0x01
    #define MODE_TX                  0x03

    void lora_write_reg(uint8_t reg, uint8_t val) {
        uint8_t data[2] = {reg | 0x80, val}; 
        gpio_put(PIN_CS, 0);
        spi_write_blocking(SPI_PORT, data, 2);
        gpio_put(PIN_CS, 1);
    }

    uint8_t lora_read_reg(uint8_t reg) {
        
        uint8_t tx[2] = {reg & 0x7F, 0x00}; 
        
        uint8_t rx[2];
        
        gpio_put(PIN_CS, 0);
        
        spi_write_read_blocking(SPI_PORT, tx, rx, 2);
        
        gpio_put(PIN_CS, 1);
        
        return rx[1];
    }
    void lora_send_f(const char* msg) {
        
        uint8_t len = strlen(msg);
        
        
        lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
        
        lora_write_reg(REG_FIFO_ADDR_PTR, 0x00);
        
        lora_write_reg(REG_PAYLOAD_LENGTH, len);
        
        gpio_put(PIN_CS, 0);
        
        uint8_t reg = REG_FIFO | 0x80;
        
        spi_write_blocking(SPI_PORT, &reg, 1);
        
        spi_write_blocking(SPI_PORT, (const uint8_t*)msg, len);
        
        gpio_put(PIN_CS, 1);
        
        lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
        
        while((lora_read_reg(REG_IRQ_FLAGS) & 0x08) == 0) {
        
            sleep_ms(1);
        }
        
        lora_write_reg(REG_IRQ_FLAGS, 0x08);
        
    }

    int lora_receive_f(char* buffer, int max_len) {
        
        uint8_t current_mode = lora_read_reg(REG_OP_MODE);
        
        if ((current_mode & 0x07) != MODE_RX_CONTINUOUS) {
            lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
        }
        
        uint8_t irq = lora_read_reg(REG_IRQ_FLAGS);
        
        if (irq & IRQ_RX_DONE_MASK) {
            lora_write_reg(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);

            uint8_t len = lora_read_reg(REG_RX_NB_BYTES);
            uint8_t current_addr = lora_read_reg(REG_FIFO_RX_CURRENT_ADDR);
            
            lora_write_reg(REG_FIFO_ADDR_PTR, current_addr);

            if (len > max_len) len = max_len;
            
            gpio_put(PIN_CS, 0);
            uint8_t reg = REG_FIFO; 
            spi_write_blocking(SPI_PORT, &reg, 1);
            spi_read_blocking(SPI_PORT, 0x00, (uint8_t*)buffer, len);
            gpio_put(PIN_CS, 1);

            return (int)len;
        }

        return 0;
    }

    void pico_sleep(int ms) {
        sleep_ms(ms);
    }
    void pico_print(const char* msg) {
        printf("%s\r\n", msg); 
    }
    int pico_read_char() {
        return getchar_timeout_us(0); 
    }
    void _gfortran_set_args(int argc, char** argv) {
        (void)argc; (void)argv;
    }
    void _gfortran_set_options(int n, int *opt) {
        (void)n; (void)opt;
    }


    extern void fortran_main(void); 

    int main() {
    // --- 1. System & Console Setup ---
    stdio_init_all();
    
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1); // Solid LED means C is running

    // Wait for terminal connection
    while (!stdio_usb_connected()) { 
        sleep_ms(10); 
    }
    printf("\r\n--- PICO LORA BOOT ---\r\n");

    // --- 2. Hardware Pin Mapping ---
    // Reset Radio First
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_put(PIN_RST, 0);
    sleep_ms(10);
    gpio_put(PIN_RST, 1);
    sleep_ms(10);

    // Init SPI and set functions for GP2, GP3, GP4
    spi_init(SPI_PORT, 1000 * 1000); // 1mega haze lol
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    
    // Chip Select is handled manually
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1); 

    // --- 3. Radio Verification ---
    uint8_t version = lora_read_reg(0x42);
    printf("LoRa Chip Version: 0x%02X\r\n", version);

    if (version != 0x12) {
        printf("CRITICAL ERROR: LoRa chip not responding on GP2-GP5!\r\n");
        while(1) { // SOS Blink
            gpio_put(25, 1); sleep_ms(100); 
            gpio_put(25, 0); sleep_ms(100); 
        }
    }

    // --- 4. Radio Configuration ---
    // Force LoRa mode and Standby
    lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    sleep_ms(10);
    lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    
    // Frequency: 433 MHz (0x6C4000)
    lora_write_reg(REG_FRF_MSB, 0x6C);
    lora_write_reg(REG_FRF_MID, 0x40);
    lora_write_reg(REG_FRF_LSB, 0x00);
    
    // Power & Buffer Setup
    lora_write_reg(REG_PA_CONFIG, 0xCF);
    lora_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    
    printf("Radio Ready. Handing over to Fortran...\r\n");
    gpio_put(25, 0); // Blink off before handoff

    // --- 5. Fortran Handoff ---
    fortran_main();

    // Should never reach here
    while(1) { tight_loop_contents(); }
}

    int ftruncate(int fd, long length) {
        return 0; 
    }

    bool __sync_bool_compare_and_swap_4(uint32_t *ptr, uint32_t oldval, uint32_t newval) {
        if (*ptr == oldval) {
            *ptr = newval;
            return true;
        }
        return false;
    }
    void testPrint() {
    printf("SUCCESS\r\n"); 

    }
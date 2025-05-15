#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/buzzer.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_JOYSTICK_X 26
#define ADC_JOYSTICK_Y 27
#define LED_BLUE 12
#define LED_GREEN  11
#define tam_quad 10

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;
} joystick_data_t;

typedef struct 
{
    joystick_data_t data;
    bool alerta_ativo;
} status_t;

QueueHandle_t xQueueJoystickData;
QueueHandle_t xQueueStatus;

void vModoTask(void *params){

    status_t status_atual;
    joystick_data_t joydata;

    while (true){
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE){
            uint16_t agua = joydata.y_pos;
            uint16_t chuva = joydata.x_pos;
            if (agua >= 2866 || chuva >= 3276){
                status_atual.data = joydata;
                status_atual.alerta_ativo = true;
            } else {
                status_atual.data = joydata;
                status_atual.alerta_ativo = false;
            }
        xQueueSend(xQueueStatus, &status_atual, 0); 
        vTaskDelay(pdMS_TO_TICKS(100));   
        }
    }
}

void vJoystickTask(void *params)
{
    adc_gpio_init(ADC_JOYSTICK_Y);
    adc_gpio_init(ADC_JOYSTICK_X);
    adc_init();

    joystick_data_t joydata;

    while (true)
    {
        adc_select_input(0); // Volume de água
        joydata.y_pos = adc_read();

        adc_select_input(1); //Volume de chuva
        joydata.x_pos = adc_read();

        xQueueSend(xQueueJoystickData, &joydata, 0); // Envia o valor do joystick para a fila
        vTaskDelay(pdMS_TO_TICKS(100));              // 10 Hz de leitura
    }
}

void vDisplayTask(void *params)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    status_t status_atual;

    char pctAgua_str[32];
    char pctChuva_str[32];

    bool cor = true;
    while (true){
        if (xQueueReceive(xQueueStatus, &status_atual, portMAX_DELAY) == pdTRUE){
               uint pct_agua = (status_atual.data.y_pos * 100) / 4095;
               uint pct_chuva = (status_atual.data.x_pos * 100) / 4095;
               sprintf(pctAgua_str, "Nvl agua: %d", pct_agua);
                sprintf(pctChuva_str, "Nvl chuva: %d", pct_chuva);

               if (status_atual.alerta_ativo){
                    ssd1306_fill(&ssd, false);
                    ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);
                    ssd1306_line(&ssd, 3, 20, 122, 20, true);
                    ssd1306_line(&ssd, 3, 40, 122, 40, true);
                    
                    ssd1306_draw_string(&ssd, "Modo: Enchente", 5, 5);
                    ssd1306_draw_string(&ssd, "ALERTA!", 38, 26);
                    ssd1306_draw_string(&ssd, pctAgua_str, 5, 42);
                    ssd1306_draw_string(&ssd, pctChuva_str, 5, 52);
               } else {
                    ssd1306_fill(&ssd, false);
                    ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);
                    ssd1306_line(&ssd, 3, 20, 122, 20, true);
                    ssd1306_line(&ssd, 3, 40, 122, 40, true);
                    
                    ssd1306_draw_string(&ssd, "Modo: Normal", 5, 5);
                    ssd1306_draw_string(&ssd, pctAgua_str, 5, 42);
                    ssd1306_draw_string(&ssd, pctChuva_str, 5, 52);
               }
               ssd1306_send_data(&ssd);
        }                       
        vTaskDelay(pdMS_TO_TICKS(50));         
    }
}


void vLedGreenTask(void *params)
{
    gpio_set_function(LED_GREEN, GPIO_FUNC_PWM);   // Configura GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(LED_GREEN); // Obtém o slice de PWM
    pwm_set_wrap(slice, 100);                     // Define resolução (0–100)
    pwm_set_chan_level(slice, PWM_CHAN_B, 0);     // Duty inicial
    pwm_set_enabled(slice, true);                 // Ativa PWM

    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            // Brilho proporcional ao desvio do centro
            int16_t desvio_centro = (int16_t)joydata.x_pos - 2000;
            if (desvio_centro < 0)
                desvio_centro = -desvio_centro;
            uint16_t pwm_value = (desvio_centro * 100) / 2048;
            pwm_set_chan_level(slice, PWM_CHAN_B, pwm_value);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}

void vLedBlueTask(void *params)
{
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);   // Configura GPIO como PWM
    uint slice = pwm_gpio_to_slice_num(LED_BLUE); // Obtém o slice de PWM
    pwm_set_wrap(slice, 100);                     // Define resolução (0–100)
    pwm_set_chan_level(slice, PWM_CHAN_A, 0);     // Duty inicial
    pwm_set_enabled(slice, true);                 // Ativa PWM

    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            // Brilho proporcional ao desvio do centro
            int16_t desvio_centro = (int16_t)joydata.y_pos - 2048;
            if (desvio_centro < 0)
                desvio_centro = -desvio_centro;
            uint16_t pwm_value = (desvio_centro * 100) / 2048;
            pwm_set_chan_level(slice, PWM_CHAN_A, pwm_value);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}


// Modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Ativa BOOTSEL via botão
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    // Cria a fila para compartilhamento de valor do joystick
    xQueueJoystickData = xQueueCreate(5, sizeof(joystick_data_t));
    xQueueStatus = xQueueCreate(5, sizeof(status_t));

    // Criação das tasks
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);
    xTaskCreate(vModoTask, "Modo Task", 256, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 512, NULL, 1, NULL);
    xTaskCreate(vLedGreenTask, "LED red Task", 256, NULL, 1, NULL);
    xTaskCreate(vLedBlueTask, "LED blue Task", 256, NULL, 1, NULL);
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}

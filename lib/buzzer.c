#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

static int buzzer_pin;
static uint slice_num;
static uint channel;
bool alarme_ativo = false;
static uint32_t ultima_execucao = 0;
static bool estado_buzzer = false;

void buzzer_init(int pin) {
    buzzer_pin = pin;

    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    channel = pwm_gpio_to_channel(buzzer_pin);
    
    pwm_set_clkdiv(slice_num, 125.0);  
    pwm_set_wrap(slice_num, 1000);     
    pwm_set_chan_level(slice_num, channel, 500); 
    
    printf("Buzzer inicializado no pino %d (PWM slice %d, canal %d)\n", 
           pin, slice_num, channel);
}


void buzzer_desliga(int pin){
    pwm_set_enabled(slice_num, false);
    gpio_put(pin, 0);
}

void tocar_alarme(){
    alarme_ativo = true;
}

void desligar_alarme() {
    alarme_ativo = false;
    buzzer_desliga(buzzer_pin); 
}

void alarme_loop() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (!alarme_ativo) {
        if (estado_buzzer) {
            buzzer_desliga(buzzer_pin);
            estado_buzzer = false;
        }
        return;
    }

    // Verifica se passou o intervalo de tempo
    if (current_time - ultima_execucao >= 350) {
        estado_buzzer = !estado_buzzer;
        ultima_execucao = current_time;
        
        if (estado_buzzer) {
            // Liga 
            uint32_t wrap = 1000000 / 700;  
            pwm_set_wrap(slice_num, wrap);
            pwm_set_chan_level(slice_num, channel, wrap / 2);
            pwm_set_enabled(slice_num, true);
        } else {
            // Desliga o buzzer
            buzzer_desliga(buzzer_pin);
        }
    }
}
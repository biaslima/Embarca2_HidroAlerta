#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init(int pin);
void tocar_frequencia(int frequencia, int duracao_ms);
void buzzer_desliga(int pin);
void tocar_alarme();
void alarme_loop();
void desligar_alarme();
#endif
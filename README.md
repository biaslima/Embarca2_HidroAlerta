# 🌊 HidroAlerta

**Sistema de Monitoramento Visual e Sonoro para Prevenção de Enchentes**

HidroAlerta é um sistema embarcado baseado na plataforma BitDogLab com RP2040 que simula o monitoramento de níveis de chuva e água. O projeto oferece resposta visual e sonora em tempo real, sendo ideal para estudos de sistemas reativos, multitarefa com FreeRTOS e interfaces sensoriais acessíveis.

---

## 🧠 Funcionalidades

- **Leitura de sensores simulados** via joystick analógico (níveis de chuva e água).
- **Exibição OLED** com nome do modo, porcentagem de água e chuva.
- **LED RGB** que transita de verde para vermelho conforme o risco aumenta.
- **Matriz de LEDs** que mostra o nível de enchente subindo (modo normal) e pisca em vermelho no alerta.
- **Buzzer** com sons distintos:
  - Grave para **água**
  - Agudo para **chuva**
  - Médio para **ambos**
- **Multitarefa com FreeRTOS** e comunicação por filas.
- **Reinício via botão B (modo BOOTSEL)**.

---

## 🖥️ Tecnologias e Bibliotecas

- **RP2040 (BitDogLab)**
- **FreeRTOS**
- **SSD1306 I2C OLED driver**
- **WS2812 (matriz RGB 5x5)**
- **PWM para buzzer e LED RGB**
- **PIO para controle da matriz**
- **C (Pico SDK)**

---

## 📦 Como compilar e executar

1. Clone este repositório:
   ```bash
   git clone https://github.com/seuusuario/hidroalerta.git
Compile com o toolchain da Raspberry Pi Pico (CMake + SDK).

Faça o upload do .uf2 gerado para a BitDogLab via modo BOOTSEL.

🧪 Simulação dos sensores
Eixo Joystick	Representa
X (ADC1)	Chuva (%)
Y (ADC0)	Água (%)

✍️ Autora
Anna Beatriz Silva Lima
Residência Tecnológica CEPEDI - Sistemas Embarcados
📍 Feira de Santana - BA

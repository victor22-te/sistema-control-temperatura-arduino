/*
 * GccApplication1.c
 *
 * Created: 09/10/2025 09:36:39 p. m.
 * Author : victo
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// Puertos L293D
#define EN PD0          // Activador
#define IN_1 PD1        // Sentido de giro 1
#define IN_2 PD2        // Sentido de giro 2
#define Calefactor PD3  // Puerto para el calefactor

// Constantes para el ADC (Registro ADMUX)
#define ADC_MUX_MASK 0xF0  // Máscara para preservar los bits no relacionados con el canal
#define ADC_CHANNEL_MASK 0x0F  // Máscara para seleccionar el canal

// UART - Configuración del baud rate
#define BAUD 2400
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

// Voltaje de referencia
#define VREF 5

// Canal de lectura
uint8_t Canal_ADC = 0; //Cambiar al canal que se requiera

// Variables a usar
char data[32];

// UART inicialización
void init_UART(void);

// Iniciamos el ADC
void init_ADC(void);                                                           

// Función para la lectura del ADC en 16 bits
uint16_t ADC_Read(uint8_t Canal_ADC);

// Función que envía caracteres por UART
void UART_sendChar(char data);

// Función para enviar cadenas por UART
void UART_puts(const char* data);

// Función para leer la temperatura
float Read_Temperature();

void control_init(void);

	

// Codigo principal 
int main(void) {
	init_ADC();
	init_UART();
	control_init();

	while (1) {
		float temperatura = Read_Temperature();

		sprintf(data, "La temperatura actual es de:  %.2foC\n", temperatura); // oC = °C
		UART_puts(data);
		_delay_ms(900);

		if (temperatura >= 40) {
			PORTD |= (1 << EN) | (1 << IN_1); // Habilitar motor en una dirección
			PORTD &= ~(1 << IN_2);            // Deshabilitar la otra dirección
			UART_puts("La temperatura actual excede los 40oC\n");
			UART_puts("Ventilador Activado.\n");
			} else if (temperatura <= 20) {
			PORTD &= ~(1 << EN);              // Deshabilitar motor
			PORTD |= (1 << Calefactor);       // Encender calefactor
			UART_puts("La temperatura actual esta por debajo de los 20oC\n");
			UART_puts("Calefactor encendido\n");
			} else {
			PORTD &= ~((1 << EN) | (1 << Calefactor)); // Apagar motor y calefactor
		}
	}
}

// UART inicialización
void init_UART(void) {
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)(UBRR_VALUE);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Iniciamos el ADC
void init_ADC(void) {
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Inicio del ADC con prescaler de 128 
	                                                                    // Con el ADEN se inicia en 1 o se apaga en 0
	                                                                    // Con los ADPS se configura el prescaler																		
	ADMUX = (1 << REFS0);                                               // Referencia a 5 V
	                                                                    // Si se requiere referencia de 1.1 agregar ---> (1 << REFS1)
}

/*
// Leer valor del ADC (10 bits)
uint16_t ADC_Read(void) {
	ADMUX = (ADMUX & 0xF0) | (Canal_ADC & 0x0F);                        // Seleccionar el canal
	ADCSRA |= (1 << ADSC);                                              // Iniciar la conversión
	while (ADCSRA & (1 << ADSC));                                       // Esperar a que la conversión termine
		                                                                // Cuando ADSC pasa a 0 termino la conversión 
	return ADCL; // Leer el valor del ADC
}*/
// Función para la lectura del ADC en 16 bits
uint16_t ADC_Read(uint8_t Canal_ADC) {
	// Verificar que el canal esté dentro del rango válido
	if (Canal_ADC > 7) {
		Canal_ADC = 0; // Usar el canal 0 por defecto si el canal no es válido
	}

	// Seleccionar el canal
	ADMUX = (ADMUX & ADC_MUX_MASK) | (Canal_ADC & ADC_CHANNEL_MASK);
	_delay_us(10);                            // Pequeño retardo para la estabilización del ADC
	ADCSRA |= (1 << ADSC);                    // Iniciar la conversión
	while (ADCSRA & (1 << ADSC)) {            // Esperar a que la conversión termine
	}
	ADCSRA |= (1 << ADIF);                    // Limpiar la bandera de fin de conversión
	return ADCL|(uint16_t)(ADCH<<8);                // Leer el valor de 10 bits del ADC
}

// Función que envía caracteres por UART
void UART_sendChar(char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

// Función para enviar cadenas por UART
void UART_puts(const char* data) {
	while (*data) {
		UART_sendChar(*data++);
	}
}

// Función para leer la temperatura
float Read_Temperature() {
	uint16_t adc_value = ADC_Read(Canal_ADC);
	float V = ((float)adc_value * VREF) / 1024;   // Convertir ADC a voltaje
	float a = V + 8.87;                           // factor para temperatura
	float b = 3.25;                               // constante
	float T_C = (a / b) *100 - 273.15;  
	return T_C;                                   // Temperatura en Celcius  
}


void control_init(void){
	
	DDRD |= (1 << EN) | (1 << IN_1) | (1 << IN_2); // Para el ventilador
	DDRD |= (1 << Calefactor);                     // Para el calefactor (foco)
	PORTD &= ~((1 << EN) | (1 << IN_1) | (1 << IN_2) | (1 << Calefactor)); // Apagar todo inicialmente
	
}
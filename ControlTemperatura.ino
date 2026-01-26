/*
 * ============================================================================
 * SISTEMA DE CONTROL DE TEMPERATURA CON ARDUINO MEGA
 * ============================================================================
 * 
 * Componentes:
 * - Arduino Mega
 * - Sensor DHT11 (Temperatura y Humedad)
 * - LCD 16x2 o 20x4 (modo 4 bits)
 * - Teclado matricial 4x4
 * - Ventilador 12V (enfriamiento)
 * - Calefactor 12V (calentamiento)
 * - Módulo relé o transistores para control de cargas
 * 
 * Funcionalidades:
 * - Lectura de temperatura y humedad en tiempo real
 * - Control automático de temperatura
 * - Ajuste de temperatura deseada mediante teclado
 * - Visualización en LCD de temperatura actual, deseada, humedad, fecha y hora
 * 
 * Autor: Sistema generado para PI
 * Fecha: 06/01/2026
 * ============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Bonezegei_DHT11.h>

// ============================================================================
// CONFIGURACIÓN DE PINES
// ============================================================================

// DHT11
#define DHT_PIN 2

// LCD I2C
#define LCD_ADDRESS 0x27  // Direccion I2C (cambiar a 0x3F si no funciona)
#define LCD_COLS 16
#define LCD_ROWS 2

// Control de actuadores (Relés o transistores)
#define PIN_VENTILADOR_FRIO 3      // Ventilador de aire frio (solo enfria)
#define PIN_CALEFACTOR 9           // Calefactor para calentar
#define PIN_VENTILADOR_CALIENTE 10 // Ventilador que distribuye aire caliente

// Teclado matricial 4x4
#define FILAS 4
#define COLUMNAS 4

// Pines para el teclado matricial
byte pinesFilas[FILAS] = {30, 31, 32, 33};
byte pinesColumnas[COLUMNAS] = {34, 35, 36, 37};

// ============================================================================
// CONFIGURACIÓN DEL TECLADO
// ============================================================================
char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// ============================================================================
// OBJETOS GLOBALES
// ============================================================================
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);
Bonezegei_DHT11 dht(DHT_PIN);

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

// Temperatura
float temperaturaActual = 0.0;
float humedadActual = 0.0;
float temperaturaDeseada = 25.0;  // Temperatura inicial deseada
float histeresis = 1.0;           // Margen para evitar oscilaciones

// Control de tiempo
unsigned long tiempoAnteriorLectura = 0;
unsigned long tiempoAnteriorLCD = 0;

const unsigned long INTERVALO_LECTURA = 2000;   // Leer sensor cada 2 segundos
const unsigned long INTERVALO_LCD = 500;        // Actualizar LCD cada 500ms

// Estados del sistema
bool modoAjuste = false;
String temperaturaInput = "";
int pantalla = 0;  // 0 = principal, 1 = ajuste temperatura, 2 = ajuste hora

// Estado de actuadores
bool ventiladorFrioActivo = false;
bool ventiladorCalienteActivo = false;
bool calefactorActivo = false;

// Caracteres personalizados para LCD
byte iconoTermometro[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b01110
};

byte iconoGota[8] = {
  0b00100,
  0b00100,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b10001,
  0b01110
};

byte iconoVentilador[8] = {
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
  0b00000
};

byte iconoCalor[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b01010,
  0b00100,
  0b01010,
  0b00100,
  0b00000
};

// ============================================================================
// FUNCIÓN SETUP
// ============================================================================
void setup() {
  // Inicializar comunicación serial para debug
  Serial.begin(9600);
  Serial.println("Sistema de Control de Temperatura");
  Serial.println("==================================");
  
  // Inicializar LCD I2C
  lcd.init();
  lcd.backlight();  // Encender luz de fondo
  
  // Crear caracteres personalizados
  lcd.createChar(0, iconoTermometro);
  lcd.createChar(1, iconoGota);
  lcd.createChar(2, iconoVentilador);
  lcd.createChar(3, iconoCalor);
  
  // Mostrar mensaje de bienvenida
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("CONTROL DE TEMP.");
  lcd.setCursor(4, 1);
  lcd.print("INICIANDO...");
  
  // Inicializar sensor DHT11
  dht.begin();
  
  // Configurar pines de salida para actuadores
  pinMode(PIN_VENTILADOR_FRIO, OUTPUT);
  pinMode(PIN_VENTILADOR_CALIENTE, OUTPUT);
  pinMode(PIN_CALEFACTOR, OUTPUT);
  
  // Asegurar que actuadores estén apagados al inicio
  // Relevadores: HIGH = ON, LOW = OFF
  digitalWrite(PIN_VENTILADOR_FRIO, LOW);
  digitalWrite(PIN_VENTILADOR_CALIENTE, LOW);
  digitalWrite(PIN_CALEFACTOR, LOW);
  
  // Esperar inicialización
  delay(2000);
  lcd.clear();
  
  Serial.println("Sistema inicializado correctamente");
}

// ============================================================================
// FUNCIÓN LOOP PRINCIPAL
// ============================================================================
void loop() {
  unsigned long tiempoActual = millis();
  
  // Leer teclado constantemente
  leerTeclado();
  
  // Leer sensor DHT11 cada 2 segundos
  if (tiempoActual - tiempoAnteriorLectura >= INTERVALO_LECTURA) {
    tiempoAnteriorLectura = tiempoActual;
    leerSensor();
    controlarTemperatura();
  }
  
  // Actualizar LCD
  if (tiempoActual - tiempoAnteriorLCD >= INTERVALO_LCD) {
    tiempoAnteriorLCD = tiempoActual;
    actualizarLCD();
  }
}

// ============================================================================
// LECTURA DEL SENSOR DHT11
// ============================================================================
void leerSensor() {
  // Usar getData() para leer el sensor
  if (dht.getData()) {
    temperaturaActual = dht.getTemperature();
    humedadActual = (float)dht.getHumidity();
    
    Serial.print("Temp: ");
    Serial.print(temperaturaActual);
    Serial.print("C | Humedad: ");
    Serial.print(humedadActual);
    Serial.println("%");
  } else {
    Serial.println("Error al leer el sensor DHT11");
  }
}

// ============================================================================
// CONTROL DE TEMPERATURA
// ============================================================================
void controlarTemperatura() {
  // Control con histéresis para evitar oscilaciones
  
  if (temperaturaActual > temperaturaDeseada + histeresis) {
    // Temperatura muy alta: AMBOS ventiladores ON para enfriamiento rapido
    activarVentiladorFrio(true);
    activarVentiladorCaliente(true);  // Ayuda a enfriar mas rapido
    activarCalefactor(false);
    Serial.println("Enfriando RAPIDO - Ambos ventiladores ON");
  } 
  else if (temperaturaActual < temperaturaDeseada - histeresis) {
    // Temperatura muy baja: calefactor + ventilador caliente para distribuir
    activarVentiladorFrio(false);
    activarVentiladorCaliente(true);  // Distribuye el aire caliente
    activarCalefactor(true);
    Serial.println("Calentando - Calefactor + Ventilador caliente ON");
  } 
  else {
    // Temperatura en rango aceptable: desactivar todo
    activarVentiladorFrio(false);
    activarVentiladorCaliente(false);
    activarCalefactor(false);
    Serial.println("Temperatura estable - Sistema en espera");
  }
}

// ============================================================================
// CONTROL DE ACTUADORES
// Relevadores: HIGH = ON, LOW = OFF
// ============================================================================
void activarVentiladorFrio(bool estado) {
  ventiladorFrioActivo = estado;
  digitalWrite(PIN_VENTILADOR_FRIO, estado ? HIGH : LOW);
}

void activarVentiladorCaliente(bool estado) {
  ventiladorCalienteActivo = estado;
  digitalWrite(PIN_VENTILADOR_CALIENTE, estado ? HIGH : LOW);
}

void activarCalefactor(bool estado) {
  calefactorActivo = estado;
  digitalWrite(PIN_CALEFACTOR, estado ? HIGH : LOW);
}

// ============================================================================
// LECTURA DEL TECLADO
// ============================================================================
void leerTeclado() {
  char tecla = teclado.getKey();
  
  if (tecla) {
    Serial.print("Tecla presionada: ");
    Serial.println(tecla);
    
    if (!modoAjuste) {
      // Modo normal - verificar teclas de función
      switch (tecla) {
        case 'A':  // Entrar a modo ajuste de temperatura
          modoAjuste = true;
          pantalla = 1;
          temperaturaInput = "";
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Set Temp: ");
          lcd.setCursor(0, 1);
          lcd.print("#=OK *=Cancel");
          break;
          
        case 'C':  // Subir temperatura deseada +1
          temperaturaDeseada += 1.0;
          if (temperaturaDeseada > 40) temperaturaDeseada = 40;
          Serial.print("Temp deseada: ");
          Serial.println(temperaturaDeseada);
          break;
          
        case 'D':  // Bajar temperatura deseada -1
          temperaturaDeseada -= 1.0;
          if (temperaturaDeseada < 10) temperaturaDeseada = 10;
          Serial.print("Temp deseada: ");
          Serial.println(temperaturaDeseada);
          break;
      }
    } else {
      // Modo ajuste activo
      if (pantalla == 1) {
        procesarAjusteTemperatura(tecla);
      }
    }
  }
}

// ============================================================================
// PROCESAR AJUSTE DE TEMPERATURA
// ============================================================================
void procesarAjusteTemperatura(char tecla) {
  if (tecla >= '0' && tecla <= '9') {
    if (temperaturaInput.length() < 2) {
      temperaturaInput += tecla;
      lcd.setCursor(10, 0);
      lcd.print(temperaturaInput);
      lcd.print("C ");
    }
  }
  else if (tecla == '#') {
    // Confirmar
    if (temperaturaInput.length() > 0) {
      float nuevaTemp = temperaturaInput.toFloat();
      if (nuevaTemp >= 10 && nuevaTemp <= 40) {
        temperaturaDeseada = nuevaTemp;
        Serial.print("Nueva temperatura deseada: ");
        Serial.println(temperaturaDeseada);
      } else {
        Serial.println("Temperatura fuera de rango (10-40)");
      }
    }
    modoAjuste = false;
    pantalla = 0;
    lcd.clear();
  }
  else if (tecla == '*') {
    // Cancelar
    modoAjuste = false;
    pantalla = 0;
    lcd.clear();
  }
}


// ============================================================================
// ACTUALIZAR LCD (16x2 - pantalla fija)
// ============================================================================
void actualizarLCD() {
  if (modoAjuste) return;  // No actualizar si estamos en modo ajuste
  
  // Linea 1: Temperatura actual y humedad
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperaturaActual, 1);
  lcd.print("C");
  lcd.print(" H:");
  lcd.print((int)humedadActual);
  lcd.print("%  ");
  
  // Linea 2: Temperatura deseada y estado
  lcd.setCursor(0, 1);
  lcd.print("Set:");
  lcd.print(temperaturaDeseada, 0);
  lcd.print("C ");
  
  // Estado del sistema
  if (ventiladorFrioActivo && ventiladorCalienteActivo) {
    lcd.print("ENFRI!");
  } else if (calefactorActivo) {
    lcd.print("CALENT");
  } else {
    lcd.print("ESTAB ");
  }
}
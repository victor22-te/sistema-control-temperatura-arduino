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
#define PIN_VENTILADOR_FRIO 8      // Ventilador de aire frio (solo enfria)
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

// Fecha y hora (simulada - se incrementa con millis)
int hora = 0;
int minuto = 0;
int segundo = 0;
int dia = 6;
int mes = 1;
int anio = 2026;

// Control de tiempo
unsigned long tiempoAnteriorLectura = 0;
unsigned long tiempoAnteriorReloj = 0;
unsigned long tiempoAnteriorLCD = 0;

const unsigned long INTERVALO_LECTURA = 2000;   // Leer sensor cada 2 segundos
const unsigned long INTERVALO_RELOJ = 1000;     // Actualizar reloj cada segundo
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
  // NOTA: Lógica invertida para optoacopladores PC817 (HIGH = OFF, LOW = ON)
  digitalWrite(PIN_VENTILADOR_FRIO, HIGH);
  digitalWrite(PIN_VENTILADOR_CALIENTE, HIGH);
  digitalWrite(PIN_CALEFACTOR, HIGH);
  
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
  
  // Actualizar reloj cada segundo
  if (tiempoActual - tiempoAnteriorReloj >= INTERVALO_RELOJ) {
    tiempoAnteriorReloj = tiempoActual;
    actualizarReloj();
  }
  
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
// NOTA: Lógica invertida para optoacopladores PC817 (HIGH = OFF, LOW = ON)
// ============================================================================
void activarVentiladorFrio(bool estado) {
  ventiladorFrioActivo = estado;
  digitalWrite(PIN_VENTILADOR_FRIO, estado ? LOW : HIGH);  // Invertido para PC817
}

void activarVentiladorCaliente(bool estado) {
  ventiladorCalienteActivo = estado;
  digitalWrite(PIN_VENTILADOR_CALIENTE, estado ? LOW : HIGH);  // Invertido para PC817
}

void activarCalefactor(bool estado) {
  calefactorActivo = estado;
  digitalWrite(PIN_CALEFACTOR, estado ? LOW : HIGH);  // Invertido para PC817
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
          
        case 'B':  // Entrar a modo ajuste de hora
          modoAjuste = true;
          pantalla = 2;
          temperaturaInput = "";
          mostrarMenuHora();
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
      } else if (pantalla == 2) {
        procesarAjusteHora(tecla);
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
// MENÚ AJUSTE DE HORA
// ============================================================================
int campoHora = 0;  // 0=hora, 1=minuto, 2=dia, 3=mes, 4=año
String valorInput = "";

void mostrarMenuHora() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch (campoHora) {
    case 0: lcd.print("Hora (0-23):"); break;
    case 1: lcd.print("Min (0-59):"); break;
    case 2: lcd.print("Dia (1-31):"); break;
    case 3: lcd.print("Mes (1-12):"); break;
    case 4: lcd.print("Anio:"); break;
  }
  
  lcd.setCursor(0, 1);
  lcd.print("#=Sig *=Salir");
}

void procesarAjusteHora(char tecla) {
  if (tecla >= '0' && tecla <= '9') {
    if (valorInput.length() < 4) {
      valorInput += tecla;
      lcd.setCursor(0, 2);
      lcd.print(valorInput);
      lcd.print("    ");
    }
  }
  else if (tecla == '#') {
    // Guardar valor y pasar al siguiente campo
    int valor = valorInput.toInt();
    bool valido = true;
    
    switch (campoHora) {
      case 0: 
        if (valor >= 0 && valor <= 23) hora = valor;
        else valido = false;
        break;
      case 1: 
        if (valor >= 0 && valor <= 59) minuto = valor;
        else valido = false;
        break;
      case 2: 
        if (valor >= 1 && valor <= 31) dia = valor;
        else valido = false;
        break;
      case 3: 
        if (valor >= 1 && valor <= 12) mes = valor;
        else valido = false;
        break;
      case 4: 
        if (valor >= 2024 && valor <= 2030) anio = valor;
        else valido = false;
        break;
    }
    
    if (valido) {
      campoHora++;
      valorInput = "";
      
      if (campoHora > 4) {
        // Terminado
        campoHora = 0;
        modoAjuste = false;
        pantalla = 0;
        lcd.clear();
        Serial.println("Fecha/hora actualizada");
      } else {
        mostrarMenuHora();
      }
    } else {
      lcd.setCursor(0, 2);
      lcd.print("Valor invalido!");
      delay(1000);
      valorInput = "";
      mostrarMenuHora();
    }
  }
  else if (tecla == '*') {
    // Cancelar
    campoHora = 0;
    valorInput = "";
    modoAjuste = false;
    pantalla = 0;
    lcd.clear();
  }
}

// ============================================================================
// ACTUALIZAR RELOJ
// ============================================================================
void actualizarReloj() {
  segundo++;
  if (segundo >= 60) {
    segundo = 0;
    minuto++;
    if (minuto >= 60) {
      minuto = 0;
      hora++;
      if (hora >= 24) {
        hora = 0;
        dia++;
        // Simplificado - no considera días por mes
        if (dia > 31) {
          dia = 1;
          mes++;
          if (mes > 12) {
            mes = 1;
            anio++;
          }
        }
      }
    }
  }
}

// ============================================================================
// ACTUALIZAR LCD (16x2 - informacion alternada)
// ============================================================================
// Variable para alternar pantallas
static int pantallaInfo = 0;
static unsigned long tiempoAlternar = 0;

void actualizarLCD() {
  if (modoAjuste) return;  // No actualizar si estamos en modo ajuste
  
  // Alternar pantalla cada 3 segundos
  if (millis() - tiempoAlternar >= 3000) {
    tiempoAlternar = millis();
    pantallaInfo = (pantallaInfo + 1) % 3;
    lcd.clear();
  }
  
  switch (pantallaInfo) {
    case 0:  // Pantalla 1: Temperatura actual y deseada
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(temperaturaActual, 1);
      lcd.print("C");
      lcd.print(" H:");
      lcd.print((int)humedadActual);
      lcd.print("%");
      
      lcd.setCursor(0, 1);
      lcd.print("Set:");
      lcd.print(temperaturaDeseada, 0);
      lcd.print("C ");
      
      // Estado
      if (ventiladorFrioActivo && ventiladorCalienteActivo) {
        lcd.print("ENFRI!");
      } else if (calefactorActivo) {
        lcd.print("CALENT");
      } else {
        lcd.print("ESTAB ");
      }
      break;
      
    case 1:  // Pantalla 2: Fecha y hora
      lcd.setCursor(0, 0);
      lcd.print(formatearNumero(dia));
      lcd.print("/");
      lcd.print(formatearNumero(mes));
      lcd.print("/");
      lcd.print(anio);
      
      lcd.setCursor(0, 1);
      lcd.print("Hora: ");
      lcd.print(formatearNumero(hora));
      lcd.print(":");
      lcd.print(formatearNumero(minuto));
      lcd.print(":");
      lcd.print(formatearNumero(segundo));
      break;
      
    case 2:  // Pantalla 3: Instrucciones
      lcd.setCursor(0, 0);
      lcd.print("A=SetT B=Hora");
      lcd.setCursor(0, 1);
      lcd.print("C=T+1  D=T-1");
      break;
  }
}

// ============================================================================
// FORMATEAR NÚMERO CON CEROS A LA IZQUIERDA
// ============================================================================
String formatearNumero(int num) {
  if (num < 10) {
    return "0" + String(num);
  }
  return String(num);
}
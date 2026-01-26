/*
 * ============================================================================
 * PRUEBA DE LCD I2C - HOLA MUNDO
 * ============================================================================
 * Código simple para verificar que la LCD I2C funciona correctamente
 * 
 * Conexiones LCD I2C 20x4:
 * - GND -> GND
 * - VCC -> 5V
 * - SDA -> Pin 20 (Arduino Mega)
 * - SCL -> Pin 21 (Arduino Mega)
 * 
 * Direccion I2C comun: 0x27 o 0x3F
 * Si no funciona con 0x27, cambia a 0x3F
 * ============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Direccion I2C de la LCD (prueba 0x27 o 0x3F si no funciona)
#define LCD_ADDRESS 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

// Crear objeto LCD I2C
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void setup() {
  // Iniciar comunicacion serial para debug
  Serial.begin(9600);
  Serial.println("Iniciando prueba de LCD I2C...");
  Serial.print("Direccion I2C: 0x");
  Serial.println(LCD_ADDRESS, HEX);
  
  // Inicializar LCD I2C
  lcd.init();
  
  // Encender luz de fondo
  lcd.backlight();
  
  // Limpiar pantalla
  lcd.clear();
  
  // Mostrar mensaje en cada linea
  lcd.setCursor(0, 0);
  lcd.print("Linea 1: Hola Mundo!");
  
  lcd.setCursor(0, 1);
  lcd.print("Linea 2: Prueba LCD");
  
  lcd.setCursor(0, 2);
  lcd.print("Linea 3: I2C 0x27");
  
  lcd.setCursor(0, 3);
  lcd.print("Linea 4: OK!");
  
  Serial.println("Mensaje enviado a LCD I2C");
}

void loop() {
  // Parpadeo simple para ver si el Arduino esta corriendo
  static unsigned long prevTime = 0;
  static int contador = 0;
  
  if (millis() - prevTime >= 1000) {
    prevTime = millis();
    contador++;
    
    // Actualizar contador en LCD (esquina inferior derecha)
    lcd.setCursor(14, 3);
    lcd.print(contador);
    lcd.print("s ");
    
    Serial.print("Contador: ");
    Serial.println(contador);
  }
}

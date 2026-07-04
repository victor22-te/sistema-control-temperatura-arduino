# 🌡️ Sistema de Control de Temperatura con Arduino

Sistema inteligente de control de temperatura para habitaciones con Arduino Mega, sensor DHT11, ventiladores y calefactor, con interfaz de usuario mediante LCD y teclado matricial.

## 📋 Descripción

Este proyecto implementa un sistema automático de control de temperatura que mantiene una habitación a la temperatura deseada mediante el uso de ventiladores de enfriamiento y un calefactor. El sistema cuenta con las siguientes características:

- **Control automático con histéresis**: Evita oscilaciones constantes de encendido/apagado
- **Monitoreo en tiempo real**: Temperatura y humedad visibles en LCD
- **Interfaz de usuario intuitiva**: Teclado matricial 4x4 para ajustes
- **Modo de enfriamiento rápido**: Activa ambos ventiladores cuando la temperatura es muy alta
- **Modo de calentamiento**: Calefactor con ventilador para distribuir el calor

## ⚙️ Componentes

| Componente | Cantidad | Descripción |
|------------|----------|-------------|
| Arduino Mega 2560 | 1 | Microcontrolador principal |
| Sensor DHT11 | 1 | Sensor de temperatura y humedad |
| LCD I2C 16x2 | 1 | Display para visualización |
| Teclado Matricial 4x4 | 1 | Interfaz de usuario |
| Ventilador de Enfriamiento 12V | 1 | Ventilador de aire frío |
| Ventilador con Calefactor 12V | 2 | Calefactor + ventilador de distribución |
| Módulo Relé | 2-3 canales | Control de cargas de 12V |
| Fuente 12V | 1 | Alimentación para actuadores |
| Resistencia 4.7kΩ | 1 | Pull-up para DHT11 |

## 🔌 Conexiones Principales

### Sensor DHT11
- **VCC** → 5V Arduino
- **DATA** → Pin 2 (con resistencia 4.7kΩ a 5V)
- **GND** → GND Arduino

### LCD I2C
- **SDA** → Pin 20 (SDA)
- **SCL** → Pin 21 (SCL)
- **VCC** → 5V
- **GND** → GND

### Teclado Matricial 4x4
- **Filas**: Pines 30, 31, 32, 33
- **Columnas**: Pines 34, 35, 36, 37

### Actuadores
- **Ventilador Frío**: Pin 3 → IN1 Módulo Relé
- **Calefactor**: Pin 9 → IN2 Módulo Relé
- **Ventilador Caliente**: Pin 10 → IN3 Módulo Relé

> **Nota**: Ver `CONEXIONES.md` para diagramas detallados de todas las conexiones.

## 🎮 Uso del Sistema

### Teclas de Función

| Tecla | Función |
|-------|---------|
| **A** | Entrar al modo de ajuste de temperatura |
| **C** | Incrementar temperatura deseada (+1°C) |
| **D** | Decrementar temperatura deseada (-1°C) |
| **#** | Confirmar valor |
| **\*** | Cancelar ajuste |
| **0-9** | Ingresar valores numéricos |

### Modos de Operación

#### 1. Modo Normal
- Muestra temperatura actual, humedad y estado del sistema
- Control automático según temperatura deseada

#### 2. Modo de Ajuste
- Presiona **A** para entrar
- Ingresa la temperatura deseada (10-40°C)
- Presiona **#** para confirmar o **\*** para cancelar

## 🧠 Lógica de Control

El sistema utiliza control con **histéresis** (margen de ±1°C) para evitar ciclos rápidos:

```
Si temperatura > (deseada + 1°C):
  → Enfriamiento RÁPIDO: Ambos ventiladores ON
  
Si temperatura < (deseada - 1°C):
  → Calentamiento: Calefactor + Ventilador distribución ON
  
Si temperatura dentro del rango:
  → Sistema en espera: Todo OFF
```

## 📊 Visualización en LCD

```
┌─────────────────┐
│T:25.0C H:60%    │  ← Temp actual y humedad
│Set:25C ESTAB    │  ← Temp deseada y estado
└─────────────────┘
```

Estados posibles:
- **ESTAB**: Temperatura estable (dentro del rango)
- **ENFRI!**: Sistema enfriando activamente
- **CALENT**: Sistema calentando activamente

## 📦 Librerías Necesarias

Instalar desde el IDE de Arduino:

```
- Wire (incluida por defecto)
- LiquidCrystal_I2C
- Keypad by Mark Stanley
- Bonezegei_DHT11
```

### Instalación de Librerías

1. Abrir Arduino IDE
2. Ir a **Sketch → Include Library → Manage Libraries**
3. Buscar e instalar cada librería
4. Copiar `Bonezegei_DHT11.cpp` y `Bonezegei_DHT11.h` a la carpeta del proyecto

## 🚀 Instalación y Configuración

1. **Clonar el repositorio**
   ```bash
   git clone <URL-del-repositorio>
   ```

2. **Abrir el proyecto**
   - Abrir `ControlTemperatura.ino` en Arduino IDE

3. **Configurar LCD** (si es necesario)
   - Si la pantalla no muestra texto, cambiar la dirección I2C:
   ```cpp
   #define LCD_ADDRESS 0x27  // Cambiar a 0x3F si no funciona
   ```

4. **Verificar conexiones**
   - Revisar todas las conexiones según `CONEXIONES.md`
   - Conectar fuente de 12V para ventiladores/calefactor
   - **Importante**: Conectar GND de fuente 12V con GND de Arduino

5. **Cargar el código**
   - Conectar Arduino Mega por USB
   - Seleccionar placa: **Tools → Board → Arduino Mega 2560**
   - Seleccionar puerto COM correcto
   - Subir el código

6. **Prueba inicial**
   - Abrir Monitor Serial (9600 baud)
   - Verificar lecturas del sensor
   - Probar teclas de función

## 🔧 Configuración Avanzada

### Ajustar Histéresis
```cpp
float histeresis = 1.0;  // Cambiar valor para más o menos sensibilidad
```

### Rangos de Temperatura
```cpp
if (nuevaTemp >= 10 && nuevaTemp <= 40)  // Modificar rango permitido
```

### Intervalos de Lectura
```cpp
const unsigned long INTERVALO_LECTURA = 2000;  // Leer sensor cada 2s
const unsigned long INTERVALO_LCD = 500;       // Actualizar LCD cada 0.5s
```

## 📁 Estructura del Proyecto

```
sistema-control-temperatura-arduino/
├── ControlTemperatura.ino    # Código principal
├── Bonezegei_DHT11.cpp       # Implementación librería DHT11
├── Bonezegei_DHT11.h         # Header librería DHT11
├── CONEXIONES.md             # Diagramas de conexiones detallados
├── README.md                 # Este archivo
├── .gitignore                # Archivos ignorados por Git
├── PruebaLCD.ino             # Código de prueba para LCD
├── checkp.ino                # Checkpoint del código
└── main.c                    # Código auxiliar
```

## ⚠️ Notas Importantes

1. **Seguridad Eléctrica**
   - Verificar todas las conexiones antes de energizar
   - Usar módulos relé con optoacoplador
   - GND común entre Arduino y fuente de 12V

2. **Sensor DHT11**
   - Necesita resistencia pull-up de 4.7kΩ
   - Algunos módulos ya la incluyen integrada
   - Tiempo mínimo entre lecturas: 2 segundos

3. **LCD I2C**
   - Verificar dirección I2C (0x27 o 0x3F)
   - Usar escáner I2C si hay problemas de detección

4. **Protección**
   - Usar diodos de protección con cargas inductivas
   - Considerar fusibles en la línea de 12V

## 🐛 Solución de Problemas

| Problema | Solución |
|----------|----------|
| LCD no muestra nada | Ajustar potenciómetro de contraste / Cambiar dirección I2C |
| Sensor no lee | Verificar resistencia pull-up / Revisar conexiones |
| Teclado no responde | Verificar conexiones de filas y columnas |
| Ventilador no activa | Revisar módulo relé / Verificar alimentación 12V |
| Lecturas erráticas | Añadir capacitor 100nF entre VCC y GND del sensor |

## 📝 Registro de Cambios

### Versión 1.0 (06/01/2026)
- ✅ Implementación inicial del sistema
- ✅ Control automático con histéresis
- ✅ Interfaz LCD con iconos personalizados
- ✅ Teclado matricial para ajustes
- ✅ Modo de enfriamiento rápido (dual ventilador)
- ✅ Sistema de calentamiento con distribución de aire

## 👨‍💻 Autor

**Proyecto Integrador** - Sistema de Control de Temperatura  
Fecha de creación: Enero 2026

## 📄 Licencia

Este proyecto es de código abierto y está disponible para uso educativo.

---

💡 **Tip**: Para un mejor rendimiento, asegúrate de que la habitación esté bien ventilada y los sensores estén ubicados lejos de corrientes de aire directas.

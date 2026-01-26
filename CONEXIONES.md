# Sistema de Control de Temperatura - Diagrama de Conexiones

## 📋 Lista de Componentes
| Componente | Cantidad |
|------------|----------|
| Arduino Mega 2560 | 1 |
| Sensor DHT11 | 1 |
| LCD 20x4 o 16x2 | 1 |
| Teclado Matricial 4x4 | 1 |
| Ventilador 12V | 1 |
| Calefactor 12V | 1 |
| Módulo Relé 2 canales (o transistores TIP120) | 1 |
| Fuente 12V para ventilador/calefactor | 1 |
| Potenciómetro 10K (contraste LCD) | 1 |
| Resistencia 4.7K (pull-up DHT11) | 1 |

---

## 🔌 CONEXIONES DEL DHT11

```
    DHT11
    ┌─────┐
    │     │
    │ 1 2 3 │  (Vista frontal)
    └──┬──┘
       │
       │
  Pin 1 (VCC) ──────────> 5V Arduino
  Pin 2 (DATA) ─────────> Pin 2 Arduino (con resistencia 4.7K a 5V)
  Pin 3 (GND) ──────────> GND Arduino
```

---

## 📺 CONEXIONES DEL LCD (Modo 4 bits)

```
  LCD Pin    Función         Arduino Mega
  ────────────────────────────────────────
  1  (VSS)   GND         ──> GND
  2  (VDD)   +5V         ──> 5V
  3  (V0)    Contraste   ──> Potenciómetro 10K (centro)
  4  (RS)    Registro    ──> Pin 22
  5  (RW)    Read/Write  ──> GND (solo escritura)
  6  (EN)    Enable      ──> Pin 23
  7  (D0)    No usado
  8  (D1)    No usado
  9  (D2)    No usado
  10 (D3)    No usado
  11 (D4)    Datos       ──> Pin 24
  12 (D5)    Datos       ──> Pin 25
  13 (D6)    Datos       ──> Pin 26
  14 (D7)    Datos       ──> Pin 27
  15 (A)     Backlight+  ──> 5V (con resistencia 220Ω)
  16 (K)     Backlight-  ──> GND
```

### Potenciómetro de Contraste:
```
  5V ────┬──── GND
         │
    ┌────┴────┐
    │   10K   │
    └────┬────┘
         │
         └────> LCD Pin 3 (V0)
```

---

## ⌨️ CONEXIONES DEL TECLADO MATRICIAL 4x4

```
  Teclado 4x4 (vista frontal de pines)
  ┌────────────────────────────────┐
  │  1   2   3   A  │
  │  4   5   6   B  │
  │  7   8   9   C  │
  │  *   0   #   D  │
  └────────────────────────────────┘
      │   │   │   │   │   │   │   │
     R1  R2  R3  R4  C1  C2  C3  C4

  Pin Teclado    Arduino Mega
  ─────────────────────────────
  R1 (Fila 1)    Pin 30
  R2 (Fila 2)    Pin 31
  R3 (Fila 3)    Pin 32
  R4 (Fila 4)    Pin 33
  C1 (Col 1)     Pin 34
  C2 (Col 2)     Pin 35
  C3 (Col 3)     Pin 36
  C4 (Col 4)     Pin 37
```

---

## 🌀 CONEXIONES DEL VENTILADOR (12V)

### Opción 1: Con Módulo Relé
```
  Arduino Pin 8 ──────> IN1 del módulo relé
  5V Arduino ─────────> VCC del módulo relé
  GND Arduino ────────> GND del módulo relé

  Ventilador 12V:
  - Cable + ──> COM del relé
  - Cable - ──> GND de fuente 12V
  
  Fuente 12V:
  - +12V ────> NO (Normally Open) del relé
  - GND ─────> Cable - del ventilador
```

### Opción 2: Con Transistor TIP120
```
              ┌─────────────────┐
  Pin 8 ──────┤ Base (via 1KΩ)  │
              │     TIP120      │
  GND ────────┤ Emisor          │──> GND
              │ Colector        │──> Ventilador (-)
              └─────────────────┘
              
  Ventilador (+) ──> +12V
  
  ⚠️ IMPORTANTE: Agregar diodo 1N4007 en paralelo con el ventilador
     (cátodo al +12V) para protección contra picos inductivos
```

---

## 🔥 CONEXIONES DEL CALEFACTOR (12V)

### Mismo esquema que el ventilador:
```
  Arduino Pin 9 ──────> IN2 del módulo relé (o base de TIP120)
  
  Conexiones idénticas al ventilador pero usando
  el segundo canal del relé o un segundo TIP120
```

---

## 📊 DIAGRAMA GENERAL DE CONEXIONES

```
                         ┌─────────────────────────────────────┐
                         │           ARDUINO MEGA               │
                         │                                      │
    DHT11                │  Pin 2  ─── DATA DHT11              │
    ┌───┐                │                                      │
    │   │ ──────────────>│  5V     ─── VCC (todos los módulos) │
    └───┘                │  GND    ─── GND (todos los módulos) │
                         │                                      │
    LCD 20x4             │  Pin 22 ─── RS                      │
    ┌──────────┐         │  Pin 23 ─── EN                      │
    │          │ <──────>│  Pin 24-27 ─ D4-D7                  │
    └──────────┘         │                                      │
                         │                                      │
    Teclado 4x4          │  Pin 30-33 ─ Filas                  │
    ┌──────────┐         │  Pin 34-37 ─ Columnas               │
    │1 2 3 A   │ <──────>│                                      │
    │4 5 6 B   │         │                                      │
    │7 8 9 C   │         │  Pin 8  ─── Control Ventilador      │
    │* 0 # D   │         │  Pin 9  ─── Control Calefactor      │
    └──────────┘         │                                      │
                         └─────────────────────────────────────┘
                                      │
                                      │
                    ┌─────────────────┴─────────────────┐
                    │                                   │
               Módulo Relé                         Módulo Relé
               (Ventilador)                        (Calefactor)
                    │                                   │
                    │                                   │
              ┌─────┴─────┐                       ┌─────┴─────┐
              │ Ventilador │                       │ Calefactor │
              │    12V     │                       │    12V     │
              └───────────┘                       └───────────┘
```

---

## 🎮 TECLAS DE FUNCIÓN

| Tecla | Función |
|-------|---------|
| **A** | Entrar al menú de ajuste de temperatura |
| **B** | Entrar al menú de ajuste de fecha/hora |
| **C** | Subir temperatura deseada (+1°C) |
| **D** | Bajar temperatura deseada (-1°C) |
| **#** | Confirmar/Siguiente |
| **\*** | Cancelar/Salir |
| **0-9** | Ingresar valores numéricos |

---

## 📱 VISUALIZACIÓN EN LCD

```
┌────────────────────┐
│06/01/2026 00:06:31 │  <- Fecha y hora
│🌡 Actual: 25.0°C   │  <- Temperatura actual
│Deseada:25.0💧 60%  │  <- Temp deseada y humedad
│~ ESTABLE     A=SET │  <- Estado del sistema
└────────────────────┘
```

---

## ⚠️ NOTAS IMPORTANTES

1. **Alimentación**: El Arduino Mega puede alimentarse por USB o con una fuente externa de 7-12V en el jack DC.

2. **Fuente de 12V**: Usar una fuente de 12V separada para el ventilador y calefactor. Conectar el GND de esta fuente al GND del Arduino.

3. **Módulo Relé**: Usar un módulo relé con optoacoplador para aislar el Arduino de la carga de 12V.

4. **DHT11**: El sensor necesita una resistencia pull-up de 4.7K-10K entre DATA y VCC. Algunos módulos ya la incluyen.

5. **Contraste LCD**: Ajustar el potenciómetro hasta ver el texto claramente en la pantalla.

6. **Si usas LCD 16x2**: Cambiar en el código:
   ```cpp
   lcd.begin(16, 2);  // En lugar de lcd.begin(20, 4);
   ```

---

## 📚 LIBRERÍAS NECESARIAS

Instalar desde el Gestor de Librerías de Arduino IDE:
- `LiquidCrystal` (incluida por defecto)
- `Keypad` por Mark Stanley
- `DHT sensor library` por Adafruit

Para instalar:
1. Abrir Arduino IDE
2. Ir a **Sketch > Include Library > Manage Libraries**
3. Buscar e instalar cada librería

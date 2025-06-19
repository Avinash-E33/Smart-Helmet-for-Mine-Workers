Avinash:
#include <Wire.h>
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// Button for Emergency Alert
#define BUTTON_PIN 2  // GPIO for button
unsigned long pressStartTime = 0;
bool buttonPressed = false;
bool alertTriggered = false;

// Sensor Pins (ESP32-S2)
#define THERMISTOR_PIN 34
#define MQ5_PIN 20  
#define MQ7_PIN 19
#define MQ135_PIN 13

// I2S Configuration for INMP441 Mic
const int I2S_WS = 4;
const int I2S_SD_IN = 15;
const int I2S_SD_OUT = 9;
const int I2S_SCK = 5;

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 1024;
const int THRESHOLD_DB = 40;

int16_t audioBuffer[BUFFER_SIZE];
int16_t processedBuffer[BUFFER_SIZE];
float dbLevel;
bool isAboveThreshold = false;

// Thermistor Constants
const float BETA = 3950;
const float R0 = 10000;
const float T0_KELVIN = 298.15;

// Sensor Values
float temperature = 0;
float lpgConcentration = 0;
float coConcentration = 0;
float alcoholConcentration = 0;
float benzeneConcentration = 0;
float ammoniaConcentration = 0;

// Function to Calculate dB Level
float calculateDbLevel(int16_t* buffer, int size) {
    float sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (float)buffer[i] * (float)buffer[i];
    }
    float rms = sqrt(sum / size);
    return (rms > 0) ? 20 * log10(rms) : -100;
}

// Function to Generate Anti-Wave
void generateAntiWaveSignal(int16_t* inputBuffer, int16_t* outputBuffer, int size) {
    if (dbLevel > THRESHOLD_DB) {
        for (int i = 0; i < size; i++) {
            outputBuffer[i] = -inputBuffer[i];
        }
    } else {
        for (int i = 0; i < size; i++) {
            outputBuffer[i] = inputBuffer[i];
        }
    }
}

// Function to Configure I2S (RX Mode for Mic)
void setupI2S_RX() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 2,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_IN
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-S2 Sound & Gas Monitoring with Emergency Alert");

    pinMode(MQ5_PIN, INPUT);
    pinMode(MQ7_PIN, INPUT);
    pinMode(MQ135_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Internal pull-up resistor

    setupI2S_RX();  // Initialize I2S for microphone input
}

void loop() {
    // Emergency Button Logic
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!buttonPressed) {
            pressStartTime = millis();
            buttonPressed = true;
            alertTriggered = false;
        }
        if (!alertTriggered && millis() - pressStartTime >= 5000) {
            Serial.println("🚨 Emergency Alert Triggered!");
            alertTriggered = true;
        }
    } else {
        buttonPressed = false;
    }

    // Read Audio Data from INMP441 Mic
    size_t bytesRead = 0;
    if (i2s_read(I2S_NUM_0, audioBuffer, BUFFER_SIZE * sizeof(int16_t), &bytesRead, portMAX_DELAY) != ESP_OK) {
        Serial.println("I2S Read Error!");
        return;
    }

    // Process Audio Data
    dbLevel = calculateDbLevel(audioBuffer, BUFFER_SIZE);
    isAboveThreshold = (dbLevel > THRESHOLD_DB);
    generateAntiWaveSignal(audioBuffer, processedBuffer, BUFFER_SIZE);

    static unsigned long lastPlotTime = 0;
    if (millis() - lastPlotTime >= 100) {
        lastPlotTime = millis();
        Serial.print(audioBuffer[0]);  
        Serial.print(",");
        Serial.println(processedBuffer[0]);
    }

    // Read Sensor Values Every Minute
    static unsigned long lastSensorReadTime = 0;
    if (millis() - lastSensorReadTime >= 60000) {
        lastSensorReadTime = millis();

// Measure Temperature
        int thermistorValue = analogRead(THERMISTOR_PIN);
        float resistance = (1023.0 / thermistorValue - 1) * R0;
        temperature = (1 / (log(resistance / R0) / BETA + 1 / T0_KELVIN)) + 26; // Convert to Celsius

        // Measure LPG Concentration with MQ5
        lpgConcentration = analogRead(MQ5_PIN);
        lpgConcentration = map(lpgConcentration, 0, 1023, 300, 10000);

        // Measure CO Concentration with MQ7
        coConcentration = analogRead(MQ7_PIN);
        coConcentration = map(coConcentration, 0, 1023, 20, 2000);

        // Measure Alcohol, Benzene, and Ammonia Concentrations with MQ135
        int mq135Reading = analogRead(MQ135_PIN);
        alcoholConcentration = map(mq135Reading, 0, 1023, 10, 300);
        benzeneConcentration = map(mq135Reading, 0, 1023, 10, 1000);
        ammoniaConcentration = map(mq135Reading, 0, 1023, 10, 300);

        // Send to Serial Monitor
        Serial.println("\n=== SENSOR READINGS ===");
        Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");

        Serial.print("LPG Concentration: "); Serial.print(lpgConcentration); Serial.println(" ppm");
        if (lpgConcentration > 500) Serial.println("🚨 LPG detected!");
        else Serial.println("✅ LPG not detected.");

        Serial.print("CO Concentration: "); Serial.print(coConcentration); Serial.println(" ppm");
        if (coConcentration > 100) Serial.println("🚨 CO detected!");
        else Serial.println("✅ CO not detected.");

        Serial.print("Alcohol Concentration: "); Serial.print(alcoholConcentration); Serial.println(" ppm");
        if (alcoholConcentration > 100) Serial.println("🚨 Alcohol detected!");
        else Serial.println("✅ Alcohol not detected.");

        Serial.print("Benzene Concentration: "); Serial.print(benzeneConcentration); Serial.println(" ppm");
        if (benzeneConcentration > 100) Serial.println("🚨 Benzene detected!");
        else Serial.println("✅ Benzene not detected.");

        Serial.print("Ammonia Concentration: "); Serial.print(ammoniaConcentration); Serial.println(" ppm");
        if (ammoniaConcentration > 50) Serial.println("🚨 Ammonia detected!");
        else Serial.println("✅ Ammonia not detected.");

        Serial.println("=======================\n");
    }

    vTaskDelay(pdMS_TO_TICKS(10));
}
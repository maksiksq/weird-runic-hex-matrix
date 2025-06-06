//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 01_Showing_Text
//----------------------------------------Including the libraries.
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
//----------------------------------------

//----------------------------------------Defines the connected PIN between P5 and ESP32.
#define R1_PIN 19
#define G1_PIN 13
#define B1_PIN 18
#define R2_PIN 5
#define G2_PIN 12
#define B2_PIN 17

#define A_PIN 16
#define B_PIN 14
#define C_PIN 4
#define D_PIN 27
#define E_PIN 32  //--> required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32.

#define LAT_PIN 26
#define OE_PIN 15
#define CLK_PIN 2
//----------------------------------------

//----------------------------------------Defines the P5 Panel configuration.
#define PANEL_RES_X 64  //--> Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64  //--> Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1   //--> Total number of panels chained one to another
//----------------------------------------


// Initialize MatrixPanel_I2S_DMA as "dma_display".
MatrixPanel_I2S_DMA *dma_display = nullptr;

//----------------------------------------Variable for color.
// color565(0, 0, 0); --> RGB color code. Use the "color picker" to use or find another color code.
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);
//----------------------------------------

// image decl
  const uint16_t testImage[16] = {
  0xF800, 0x07E0, 0x001F, 0xFFFF,  // Red, Green, Blue, White
  0xFFFF, 0x001F, 0x07E0, 0xF800,  // White, Blue, Green, Red
  0x001F, 0xF800, 0xFFFF, 0x07E0,  // Blue, Red, White, Green
  0x07E0, 0xFFFF, 0xF800, 0x001F   // Green, White, Red, Blue
};
//

// Image decl but yes

uint8_t rawData[] = {0x01, 0x02, 0x03, 0x04}; 
uint16_t processedData[2099];

void convertUint8ToUint16() {
  const size_t rawLength = sizeof(rawData);

  // Calculate number of 16-bit values
  size_t numU16 = rawLength / 2;

  // conversion loop

  for (size_t i = 0; i < numU16; i++) {
    processedData[i] = (uint16_t)rawData[2 * i] | ((uint16_t)rawData[2 * i + 1] << 8);
  }

  for (size_t i = 0; i < numU16; i++) {
    Serial.println(processedData[i], HEX);
  }
}

//

//________________________________________________________________________________BLE part

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <string>

//
// _ unused
#define LED_PIN 25

BLECharacteristic *pCharacteristic;
BLEServer *pServer;
BLEAdvertising *pAdvertising;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void startAdvertising() {
  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("Advertising started");
}

class MyCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Client disconnected, restarting advertising...");
      delay(500); // surely we won't be having issues that require real throttling, right? right??
      startAdvertising();
    }
};

bool runImage = false;

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    const uint8_t* data = (const uint8_t*)value.c_str();
    size_t length = value.length();

    Serial.println("Received this initially:");
    for (size_t i = 0; i < length; i++) {
      Serial.printf("%2d: 0x%02X\n", i, data[i]);
    }

    Serial.printf("Received %d bytes\n", length);

    if (length >= 32) { 
      for (size_t i = 0; i < 32; i++) {
        uint8_t r = data[i * 4 + 0];
        uint8_t g = data[i * 4 + 1];
        uint8_t b = data[i * 4 + 2];

        // Convert to RGB565
        uint16_t rgb565 = ((r & 0xF8) << 8) |
                          ((g & 0xFC) << 3) |
                          (b >> 3);

        processedData[i] = rgb565;
      }

      // Print processed data
      Serial.println("Processed data:");
      for (size_t i = 0; i < 16; i++) {
        Serial.printf("%2d: 0x%04X\n", i, processedData[i]);
      }

      runImage = true;
    } else {
      Serial.println("Not enough data received.");
    }
  }
};
//

//________________________________________________________________________________VOID SETUP()
void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  delay(1000);

  // Initialize the connected PIN between Panel P5 and ESP32.
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //----------------------------------------Module configuration.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   //--> module width.
    PANEL_RES_Y,   //--> module height.
    PANEL_CHAIN,   //--> Chain length.
    _pins          //--> pin mapping.
  );
  delay(10);

  mxconfig.driver = HUB75_I2S_CFG::ICN2038S;    // or try FM6126A if that doesn't work
  delay(10);

  //----------------------------------------

  // Set I2S clock speed.
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;  // I2S clock speed, better leave as-is unless you want to experiment.
  delay(10);

  //----------------------------------------Display Setup.
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(35); //--> 0-255.
  //----------------------------------------
  
  dma_display->clearScreen();
  
  dma_display->fillScreen(myWHITE);
  delay(1000);
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);
  
  dma_display->clearScreen();
  delay(1000);

  //

  pinMode(LED_PIN, OUTPUT);

  BLEDevice::init("ESP32_LED_Control");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();
  startAdvertising();

  Serial.println("Waiting for a client to connect...");
}
//________________________________________________________________________________

int ii = 0;

//________________________________________________________________________________VOID LOOP()
void loop() {
  Serial.println("Looping alive");
  Serial.println(ii++);
  dma_display->setTextSize(1);    
  dma_display->setTextWrap(false);

  //----------------------------------------
  dma_display->setCursor(10, 0);
  dma_display->setTextColor(dma_display->color565(255, 153, 0));
  dma_display->println("UTEH");

  dma_display->setCursor(36, 0);
  dma_display->setTextColor(dma_display->color565(255, 0, 255));
  dma_display->print("STR");
  //----------------------------------------

  //----------------------------------------
  dma_display->setCursor(11, 8);
  dma_display->setTextColor(dma_display->color565(0, 152, 158));
  dma_display->println("ARDUINO");
  //----------------------------------------

  //----------------------------------------
  dma_display->setCursor(16, 17);
  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->print("P5");

  dma_display->setCursor(30, 17);
  dma_display->setTextColor(dma_display->color565(255, 0, 0));
  dma_display->print("R");
  
  dma_display->setTextColor(dma_display->color565(0, 255, 0));
  dma_display->print("G");

  dma_display->setTextColor(dma_display->color565(0, 0, 255));
  dma_display->print("B");
  //----------------------------------------

  //----------------------------------------
  dma_display->setCursor(16, 25);
  dma_display->setTextColor(dma_display->color565(255, 0, 102));
  dma_display->print("ESP");

  dma_display->setCursor(36, 27);
  dma_display->setTextColor(dma_display->color565(241, 197, 7));
  dma_display->print("32");
  //----------------------------------------

  delay(1000);

  if (runImage) {
    dma_display->drawRGBBitmap(0, 0, processedData, 4, 4);
  }
}
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





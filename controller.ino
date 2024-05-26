// Libs: Keypad, LiquidCrystal_I2C
// for cyrilic in attr of lcd: "variant": "A02"
// https://robotclass.ru/tutorials/displej-1602-i2c-s-russkimi-bukvami/
// https://rxtx.su/mikrokontrollery/pamyat-v-esp32/


#define RELAY 12

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <nvs.h>
#include <nvs_flash.h>

const byte ROW_NUM    = 4;
const byte COLUMN_NUM = 4;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {23, 4, 15, 19};
byte pin_column[COLUMN_NUM] = {18, 5, 17, 16};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

#define N_DIGITS 8
char digits[N_DIGITS + 1];
int n = 0;

#define OPEN_PERIOD 5
#define ERROR_PERIOD 4

#define RESET_CODE "AD25C911"

#define N_CODES 10

char codes[N_CODES][N_DIGITS+1] = {
  "00000011",
  "00000012",
  "00000013",
  "00000014",
  "00000015",
  "00000016",
  "00000017",
  "00000018",
  "00000019",
  "0000001A",
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

void clearNVS() {
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err_t err = nvs_flash_init();
    ESP_ERROR_CHECK(err);
}

void initializeNVS() {
    // Serial.println("initializeNVS");

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        clearNVS();
        return;
    }

    ESP_ERROR_CHECK(err);
}

void createNVS() {
    // Serial.println("createNVS");
    esp_err_t err;
    nvs_handle_t nvs;

    // Open the NVS partition
    err = nvs_open("storage", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        // Serial.println("Error opening NVS:");
        // Serial.println(esp_err_to_name(err));

        // Handle the error (e.g., erase and try again)
        ESP_ERROR_CHECK(nvs_flash_erase());
        initializeNVS();
    } else {
        // NVS opened successfully, close the handle
        nvs_close(nvs);
    }
}

bool keyExists(char *key) {
    nvs_handle_t nvs;
    esp_err_t err;
    bool result;

    // Open the NVS partition
    err = nvs_open("storage", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        //printf("Error opening NVS: %s\n", esp_err_to_name(err));
        return false;
    }

    // Retrieve an 8-bit signed integer from NVS
    int8_t value;
    err = nvs_get_i8(nvs, key, &value);
    if (err == ESP_OK) {
        //printf("Value retrieved from NVS: %d\n", value);
        result = true;
    } else {
        //printf("Error retrieving value from NVS: %s\n", esp_err_to_name(err));
        result = false;
    }

    // Close the NVS handle
    nvs_close(nvs);
    return result;
}

void setKey(char *key) {
    nvs_handle_t nvs;
    esp_err_t err;

    // Open the NVS partition
    err = nvs_open("storage", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        //printf("Error opening NVS: %s\n", esp_err_to_name(err));
        return;
    }

    // Set an 8-bit signed integer in NVS
    int8_t value_to_set = 1;
    err = nvs_set_i8(nvs, key, value_to_set);
    if (err == ESP_OK) {
        //printf("Value set in NVS: %d\n", value_to_set);
    } else {
        //printf("Error setting value in NVS: %s\n", esp_err_to_name(err));
    }

    // Commit the changes
    nvs_commit(nvs);

    // Close the NVS handle
    nvs_close(nvs);
}

uint8_t letterZ[8] = {
  0b11110,
  0b00001,
  0b00001,
  0b00110,
  0b00001,
  0b00001,
  0b11110,
  0b00000,
};
uint8_t letterD[8] = {
  0b01111,
  0b00101,
  0b00101,
  0b01001,
  0b10001,
  0b11111,
  0b10001,
  0b10001,
};
uint8_t letterI[8] = {
  0b10001,
  0b10001,
  0b10011,
  0b10101,
  0b11001,
  0b10001,
  0b10001,
  0b00000,
};
uint8_t letterP[8] = {
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b00000,
};
uint8_t letterL[8] = {
  0b01111,
  0b00101,
  0b00101,
  0b00101,
  0b00101,
  0b10101,
  0b01001,
  0b00000,
};
uint8_t letterSoft[8] = {
  0b01000,
  0b01000,
  0b01000,
  0b01110,
  0b01001,
  0b01001,
  0b01110,
  0b00000,
};
uint8_t letterY[8] = {
  0b10001,
  0b10001,
  0b10001,
  0b11001,
  0b10101,
  0b10101,
  0b11001,
  0b00000,
};
uint8_t letterYI[8] = {
  0b01010,
  0b00100,
  0b10001,
  0b10011,
  0b10101,
  0b11001,
  0b10001,
  0b00000,
};

void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  Serial.begin(115200);
  createNVS();

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, letterZ);
  lcd.createChar(1, letterD);
  lcd.createChar(2, letterI);
  lcd.createChar(3, letterP);
  lcd.createChar(4, letterL);
  lcd.createChar(5, letterSoft);
  lcd.createChar(6, letterY);
  lcd.createChar(7, letterYI);

  reset();
}

void loop() {
  enter_code();
}

void enter_code() {
  char key = keypad.getKey();

  if (key) {
    // Serial.println(key);
    
    if (key == '#') {
      reset();
    }
    
    if (n >= N_DIGITS) return;

    if ((key >= 'A' && key <= 'D') || (key >= '0' && key <= '9')) {
      digits[n] = key;
      n++;
      digits[n] = '\0';

      print_code();
      
      if (n == N_DIGITS) {
        check_code();
      }
    }
  }
}

void print_code() {
  lcd.setCursor(5, 1);
  lcd.print(digits);
}

void print_enter() {
  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.print("Enter code from");
  lcd.print("BBE\x01\x02TE KO\x01 \x02");
  lcd.write((byte)'\x00');
  lcd.setCursor(0, 1);
  lcd.print("SMS:");
}

void reset() {
  // mode = MODE_ENTER;
  digitalWrite(RELAY, LOW);

  n = 0;
  digits[0] = '\0';
  print_enter();
}

void check_code() {
  if (!strcmp(digits, RESET_CODE)) {
    clearNVS();
    reset();
    return;
  }
  for (int i = 0; i < N_CODES; ++i) {
    if (!strcmp(digits, codes[i])) {
      if (keyExists(digits)) break;
      setKey(digits);
      open();
      return;
    }
  }
  show_error();
}

void show_error() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HE\x03PAB\x02\x04\x05H\x06\x07 KO\x01");
  // lcd.setCursor(0, 1);
  // lcd.print("Try another one.");

  delay(ERROR_PERIOD * 1000);
  reset();
}

void open() {
  digitalWrite(RELAY, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("\x03PAB\x02\x04\x05H\x06\x07 KO\x01.");
  lcd.setCursor(0, 1);
  char msg[25];
  sprintf(msg, "%d CEK.!", OPEN_PERIOD);
  lcd.print(msg);

  delay(OPEN_PERIOD * 1000);
  reset();
}



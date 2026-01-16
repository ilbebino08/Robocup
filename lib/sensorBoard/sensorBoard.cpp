#include "sensorBoard.h"

/*°°°°° funzioni °°°°°*/

bool BottomSensor::start(){
    bool ris = true;

    Serial1.begin(SERIAL_SPEED_BS);
    delay(4000);

    status = 0b00000100;
    setCheckColor(getLineMode());

    return ris;
}

//lettura valori linea
int16_t BottomSensor::line() {
  int16_t val;
  uint8_t buffer;

  Serial1.flush();
  Serial1.write(3);

  wait_serial();
  buffer = Serial1.read();

  status = ( (buffer & 0xF0) | (status & 0x0F) );
  val = ((uint16_t)(buffer & 0x0F) << 8);

  wait_serial();
  buffer = Serial1.read();
  val = val | (unsigned int)buffer;

  return (val - 1750);
}

//check color
void BottomSensor::setCheckColor(uint8_t mode){

  Serial1.flush();

  if (mode == 1)        {
      //Serial.println("1111111");
      for (int i = 0; i < 10; i++) {
          Serial1.write((char) 6);         //silver//
          delay(20);
           }
  }
  else if (mode == 2) {
      //Serial.println("2222222");
      for (int i = 0; i < 10; i++) {
          Serial1.write((char)7);         //black
          delay(20);
      }

  }
  else if (mode == 3) {
      //Serial.println("666666666666");
      for (int i = 0; i < 10; i++){
          Serial1.write((char) 8);  //red
          delay(20);
      }
  }


  status = ( (status & 0xF3) | (mode << 2) );
}

//Calibrazione Sensori
bool BottomSensor::Utils::calibration() {
  uint8_t buffer;

  Serial1.flush();
  Serial1.write(1);
  delay(100);
  Serial1.write(0xAA);

  wait_serial();
  buffer = Serial1.read();
  
  return buffer;
}

//lettura valori calibrazione
uint16_t* BottomSensor::Utils::calibration_val() {

  static uint16_t* val = new uint16_t[N_SENSOR*2];
  uint8_t buffer;

  Serial1.flush();
  Serial1.write(2);

  for (uint8_t i = 0; i < N_SENSOR; i++) {
    
    wait_serial();
    buffer = Serial1.read();
    val[i] = (uint16_t)buffer * 256;

    wait_serial();
    buffer = Serial1.read();
    val[i] = val[i] | (uint16_t)buffer;
  }

  for (uint8_t i = 0; i < N_SENSOR; i++) {
    wait_serial();
    buffer = Serial1.read();
    val[i+N_SENSOR] = (uint16_t)buffer * 256;

    wait_serial();
    buffer = Serial1.read();
    val[i+N_SENSOR] = val[i+N_SENSOR] | (uint16_t)buffer;
  }

  return val;
}

//lettura valore sensore
uint16_t BottomSensor::Utils::val_sensor(uint8_t n){
    uint16_t val;
    uint8_t buffer;

    if (n > N_SENSOR) return -1;
    
    buffer = 4 | (n << 4);
    Serial1.flush();
    Serial1.write(buffer);

    wait_serial(); 
    buffer = Serial1.read();
    val = (uint16_t)buffer * 256;

    wait_serial(); 
    buffer = Serial1.read();
    val = val | (uint16_t)buffer;
    
    return val;
}

//lettura valori sensori
uint16_t* BottomSensor::Utils::val_sensor(){
    static uint16_t* val = new uint16_t[N_SENSOR];

    for (uint8_t i = 0; i < N_SENSOR; i++) val[i] = val_sensor(i);

    return val;
}

//lettura sensore calibrato
uint16_t BottomSensor::Utils::val_sensorCal(uint8_t ch){

  uint16_t val;
  uint8_t buffer;

  if (ch > N_SENSOR) return -1;

  buffer = 5 | (ch << 4);
  Serial1.flush();
  Serial1.write(buffer);

  wait_serial(); //lettura primo valore
  buffer = Serial1.read();
  val = (uint16_t)buffer * 256;

  wait_serial(); //lettura secondo valore
  buffer = Serial1.read();
  val = val | (uint16_t)buffer;

  return val;

}

//lettura sensori calibrati
uint16_t* BottomSensor::Utils::val_sensorCal() {
    static uint16_t* val = new uint16_t[N_SENSOR];

    for (uint8_t i = 0; i < N_SENSOR; i++) val[i] = val_sensorCal(i);

    return val;
}

void BottomSensor::Utils::greenSxCalibration() {
    Serial1.flush();
    Serial1.write(9);

    /* todo Da attivare quando verra restituito un valore dalla scheda.
    wait_serial();
    buffer = Serial1.read();
    return buffer;
    */
}

void BottomSensor::Utils::greenDxCalibration() {
  Serial1.flush();
  Serial1.write(10);

  /* todo Da attivare quando verra restituito un valore dalla scheda.
  wait_serial();
  buffer = Serial1.read();
  return buffer;
  */
}
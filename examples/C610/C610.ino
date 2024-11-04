#include <esp32twai.h>
#include <c610.h>

ESP32TWAI twai(GPIO_NUM_4, GPIO_NUM_5);
C610 c610;

void setup() {
  Serial.begin(115200);
  twai.setRxQueueSize(64);  // C610の周期に間に合うくらいのキューサイズに
  if (!twai.begin(TWAI_TIMING_CONFIG_1MBITS())) {
    Serial.println("Failed to start ESP32 TWAI(CAN)");
  }
  c610.begin(&twai);
}

void loop() {
  c610.update();

  float Kp = 100;
  float vTarget = 100.0f;
  // 現在の速度をrpsで取得
  float vActual = c610.getRps(C610::ID::ID2);
  float error = vTarget - vActual;

  // 電流値をmAで指定
  c610.setCurrent(C610::ID::ID2, Kp * error);

  // M2006の回転速度と絶対位置を出力
  Serial.printf("%f %f\n", c610.getRps(C610::ID::ID2), c610.getPosition(C610::ID::ID2));

  delay(10);
}

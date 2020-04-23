#include "PinMapping.h"
#include <SPI.h>
#include <epd1in54_V2.h>
#include <epdpaint.h>
#include "WifiAP.h"
#include "Log.h"
#include "InputManager.h"
#include "PowerManager.h"

WifiAP wifiAP;

unsigned char image[200*200];
Paint paint(image, 200, 200);
Epd epd;

unsigned long time_start_ms;

void setup()
{
    Serial2.begin(9600);
    
    Serial.begin(115200);
    Log().Info("MAIN") << "setup" << std::endl;

    //wifiAP.Start("SpotiPi_Settings", "12345678");

    Log().Info("MAIN") << "e-Paper init" << std::endl;
    if (epd.Init() != 0)
      Log().Error("MAIN") << "e-Paper init failed" << std::endl;

    paint.Clear(1);
    epd.Display(paint.GetImage());
    time_start_ms = millis();

    PowerManager::Init();
    InputManager::Init();

    Log().Info("MAIN") << "ready" << std::endl;
}


void loop()
{
  //Log().Info("MAIN") << "update" << std::endl;

  //wifiAP.Update();

  auto is = InputManager::GetInputState();
  auto ps = PowerManager::GetPowerState();

  String s = "";
  while (Serial2.available()) 
  {
    s += char(Serial2.read());
  }
  if (s != "")
    Serial.print(s);

  Serial2.println(s + " World " + String(millis()));
  
  float time_now_s = (float)(millis() - time_start_ms) / 1000;

  paint.Clear(1);   
  paint.DrawStringAt(0, 0, (String(time_now_s)).c_str(), &Font24, 0);
  
  paint.DrawStringAt(0, 20, (String("BTN: ") + String(is.buttons[0]) + String(is.buttons[1]) + String(is.buttons[2]) + String(is.buttons[3])).c_str(), &Font24, 0);
  paint.DrawStringAt(0, 40, (String("BAT: ") + String(ps.batteryVoltage)).c_str(), &Font24, 0);
  paint.DrawStringAt(0, 60, (String("FULL: ") + String(ps.batteryIsFull)).c_str(), &Font24, 0);
  paint.DrawStringAt(0, 80, (String("USB: ") + String(ps.isOnUsb)).c_str(), &Font24, 0);
  paint.DrawStringAt(0, 100, (String("RFID: ") + is.rfId).c_str(), &Font20, 0);
  paint.DrawStringAt(0, 120, (String("POTI: ") + is.poti).c_str(), &Font24, 0);

  if (is.buttons[0] + is.buttons[1] + is.buttons[2] + is.buttons[3] > 0)
    epd.WaitUntilIdle();

  if (!epd.IsBusy())
    epd.DisplayPart(paint.GetImage(), false);

  if (is.buttons[0] > 0)
  {
    digitalWrite(POWER_OFF_PIN, HIGH);
  }

  if (is.buttons[1] > 0)
  {
    digitalWrite(PI_POWER_PIN, HIGH);
  }

  if (is.buttons[2] > 0)
  {
    digitalWrite(PI_POWER_PIN, LOW);
  }

  if (is.buttons[3] > 0)
  {

  }
}

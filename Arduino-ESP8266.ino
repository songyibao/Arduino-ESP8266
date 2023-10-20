/***********************************************************
 * author: kerwincui [物美智能 wumei-smart]
 * create: 2022-02-20
 * email：164770707@qq.com
 * source:https://github.com/kerwincui/wumei-smart
 * board:esp8266 core for arduino v3.0.2
 ***********************************************************/

#include "Helper.h"
#include "DHT.h"
long lastMqttConn;          // 上次mqtt连接时间
long lastPublishMonitor;    // 上次发布监测数据时间
long lastTimerMonitor;      // 上次定时发布监测数据
DHT dht;
// 获取温湿度值
String getTemhumData(){
  StaticJsonDocument<1024> doc;
  JsonObject objTmeperature = doc.createNestedObject();
  objTmeperature["id"] = "temperature";
  objTmeperature["value"] = dht.getTemperature();
  objTmeperature["remark"] = (String)millis();

  JsonObject objHumidity   = doc.createNestedObject();
  objHumidity["id"] = "humidity";
  objHumidity["value"] = dht.getHumidity();
  objHumidity["remark"] = (String)millis();

  printMsg("获取温湿度数据:");
  serializeJson(doc, Serial);
  String output;
  serializeJson(doc, output);
  return output;
}
/**
 * 启动
 */
void setup()
{
  //打开串行端口：
  Serial.begin(115200);
  printMsg("wumei smart device starting...");
  connectWifi();
  connectMqtt();
  dht.setup(14);
}

/**
 * 循环执行
 */
void loop()
{
  // Wifi掉线重连
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWifi();
  }

  // 非阻塞Mqtt重连，间隔30秒
  if (WiFi.status() == WL_CONNECTED)
  {
    long now = millis();
    if (!mqttClient.connected())
    {
      if (now - lastMqttConn > 30000)
      {
        lastMqttConn = now;
        connectMqtt();
      }
    }
    else
    {
      mqttClient.loop();
    }
  }

  // 非阻塞发布实时监测数据,间隔默认1秒
  if(WiFi.status() == WL_CONNECTED && monitorCount>0){
    long now = millis();
    if (now - lastPublishMonitor > monitorInterval)
      {
        lastPublishMonitor = now;
        monitorCount--;
        publishMonitor();
      }
  }

  // 非阻塞定时上报，测试用，60秒发布一次
  if(WiFi.status() == WL_CONNECTED){
    long now = millis();
    if (now - lastTimerMonitor > 60000)
      {
        lastTimerMonitor = now;
        printMsg("执行定时上报");
        // 发布事件
        // publishEvent();
        // 发布时钟同步
        publishNtp();
        String output=getTemhumData();
        // 发布属性(监测值)
        publishProperty(output);
      }
  }

}

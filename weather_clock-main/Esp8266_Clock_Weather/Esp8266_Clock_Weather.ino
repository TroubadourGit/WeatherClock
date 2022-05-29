#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <DNSServer.h>//密码直连将其三个库注释
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h>

#include <time.h>                       
#include <sys/time.h>                  
#include <coredecls.h>      


//#include "SH1106Wire.h"   //1.3寸用这个
#include "SSD1306Wire.h"    //0.96寸用这个
#include "OLEDDisplayUi.h"
#include "HeFeng.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"

/***************************
   Begin Settings
 **************************/
/*  使用Web配网请注释本宏即可  */
// #define  WifiPassConnect

const char* WIFI_SSID = "Mr.Qin";  //填写你的WIFI名称及密码
const char* WIFI_PWD = "qin666666";

const char* BILIBILIID = "104521763";  //填写你的B站账号

//由于太多人使用我的秘钥，导致获取次数超额，所以不提供秘钥了，大家可以到https:// dev.heweather.com/获取免费的
const char* HEFENG_KEY = "243a741836da45739cd3130422ca9b55"; // 填写你的和风天气秘钥
const char* HEFENG_LOCATION = "CN101280608";  // 填写你的城市ID,可到https://where.heweather.com/index.html查询
// const char* HEFENG_LOCATION = "auto_ip";         // 自动IP定位

#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0

const int UPDATE_INTERVAL_SECS = 5 * 60; // 5分钟更新一次天气
const int UPDATE_CURR_INTERVAL_SECS = 2 * 59; // 2分钟更新一次粉丝数

const int I2C_DISPLAY_ADDRESS = 0x3c;  //I2c地址默认
#if defined(ESP8266)
const int SDA_PIN = 0;  //引脚连接
const int SDC_PIN = 2;  //
#endif


const String WDAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};  //星期
const String MONTH_NAMES[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};  //月份

// web配网页面自定义我的图标请随便使用一个图片转base64工具转换https://oktools.net/image2base64, 64*64--JPGE格式
   const char Icon[] PROGMEM = "data:image/jpeg;base64,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";

/***************************
   End Settings
 **************************/
 
//SH1106Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 1.3寸用这个
SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 0.96寸用这个
OLEDDisplayUi   ui( &display );

HeFengCurrentData currentWeather; //实例化对象
HeFengForeData foreWeather[3];
HeFeng HeFengClient;

#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

time_t now; //实例化时间

bool readyForWeatherUpdate = false; // 天气更新标志
bool first = true;  //首次更新标志
long timeSinceLastWUpdate = 0;    //上次更新后的时间
long timeSinceLastCurrUpdate = 0;   //上次天气更新后的时间

String fans = "-1"; //粉丝数

void drawProgress(OLEDDisplay *display, int percentage, String label);   //提前声明函数
void updateData(OLEDDisplay *display);
void updateDatas(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();

//添加框架
//此数组保留指向所有帧的函数指针
//框架是从右向左滑动的单个视图
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawForecast };
//页面数量
int numberOfFrames = 3;

OverlayCallback overlays[] = { drawHeaderOverlay }; //覆盖回调函数
int numberOfOverlays = 1;  //覆盖数

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // 屏幕初始化
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically(); //屏幕翻转
  display.setContrast(255); //屏幕亮度

#ifdef WifiPassConnect
  // 用固定密码连接，Web配网请注释
  wificonnect();
#else
  //Web配网，密码直连请注释
  webconnect();
#endif


  ui.setTargetFPS(30);  //刷新频率

  ui.setActiveSymbol(activeSymbole); //设置活动符号
  ui.setInactiveSymbol(inactiveSymbole); //设置非活动符号

  // 符号位置
  // 你可以把这个改成TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // 定义第一帧在栏中的位置
  ui.setIndicatorDirection(LEFT_RIGHT);

  // 屏幕切换方向
  // 您可以更改使用的屏幕切换方向 SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames); // 设置框架
  ui.setTimePerFrame(5000); //设置切换时间
  
  ui.setOverlays(overlays, numberOfOverlays); //设置覆盖

  // UI负责初始化显示
  ui.init();
  display.flipScreenVertically(); //屏幕反转

  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
  delay(200);

}

void loop() {
  if (first) {  //首次加载
    updateDatas(&display);
    first = false;
  }
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) { //屏幕刷新
    setReadyForWeatherUpdate();
    timeSinceLastWUpdate = millis();
  }
  if (millis() - timeSinceLastCurrUpdate > (1000L * UPDATE_CURR_INTERVAL_SECS)) { //粉丝数更新
    HeFengClient.fans(&currentWeather, BILIBILIID);
    fans = String(currentWeather.follower);
    timeSinceLastCurrUpdate = millis();
  }

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) { //天气更新
    updateData(&display);
  }

  int remainingTimeBudget = ui.update(); //剩余时间预算

  if (remainingTimeBudget > 0) {
    //你可以在这里工作如果你低于你的时间预算。
    delay(remainingTimeBudget);
  }
  
}

#ifdef WifiPassConnect
void wificonnect() {  //WIFI密码连接，Web配网请注释
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_5);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_6);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_7);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_8);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_1);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_2);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_3);
    display.display();
    delay(80);
    display.clear();
    display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_4);
    display.display();
  }
  Serial.println("");
  delay(500);
}
#else
void webconnect() {  ////Web配网，密码直连将其注释
  display.clear();
  display.drawXbm(0, 0, 128, 64, bilibili); //显示哔哩哔哩
  display.display();

  WiFiManager wifiManager;  //实例化WiFiManager
  wifiManager.setDebugOutput(false); //关闭Debug
  //wifiManager.setConnectTimeout(10); //设置超时
  wifiManager.setHeadImgBase64(FPSTR(Icon)); //设置图标
  wifiManager.setPageTitle("欢迎来到游吟诗人的WiFi配置页");  //设置页标题

  // if (!wifiManager.autoConnect("XiaoKai-IOT-Display")) {  //AP模式
  if (!wifiManager.autoConnect("Troubadour_Config")) {  //AP模式
    Serial.println("连接失败并超时");
    //重新设置并再试一次，或者让它进入深度睡眠状态
    ESP.restart();
    delay(1000);
  }
  Serial.println("connected...^_^");
  yield();
}
#endif

void drawProgress(OLEDDisplay *display, int percentage, String label) {    //绘制进度
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void updateData(OLEDDisplay *display) {  //天气更新
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  readyForWeatherUpdate = false;
}

void updateDatas(OLEDDisplay *display) {  //首次天气更新
  drawProgress(display, 0, "Updating fansnumb...");
  HeFengClient.fans(&currentWeather, BILIBILIID);
  fans = String(currentWeather.follower);
  
  drawProgress(display, 33, "Updating weather...");
  HeFengClient.doUpdateCurr(&currentWeather, HEFENG_KEY, HEFENG_LOCATION);
  
  drawProgress(display, 66, "Updating forecasts...");
  HeFengClient.doUpdateFore(foreWeather, HEFENG_KEY, HEFENG_LOCATION);
  
  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(200);
  
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示时间
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%04d-%02d-%02d  %s"), timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(64 + x, 5 + y, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64 + x, 22 + y, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //显示天气
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 38 + y, currentWeather.cond_txt + "    |   Wind: " + currentWeather.wind_sc + "  ");

  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  String temp = currentWeather.tmp + "°C" ;
  display->drawString(60 + x, 3 + y, temp);
  display->setFont(ArialMT_Plain_10);
  display->drawString(62 + x, 26 + y, currentWeather.fl + "°C | " + currentWeather.hum + "%");
  display->setFont(Meteocons_Plain_36);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(32 + x, 0 + y, currentWeather.iconMeteoCon);
}

void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {  //天气预报
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 1);
  drawForecastDetails(display, x + 88, y, 2);
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {  //天气预报

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y, foreWeather[dayIndex].datestr);
  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, foreWeather[dayIndex].iconMeteoCon);

  String temp = foreWeather[dayIndex].tmp_max + " | " + foreWeather[dayIndex].tmp_min;
  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, temp);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {   //绘图页眉覆盖
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[14];
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);

  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(6, 54, String(buff));
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  String temp = fans;
  display->drawString(122, 54, temp);
  display->drawHorizontalLine(0, 52, 128);
}

void setReadyForWeatherUpdate() {  //为天气更新做好准备
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}

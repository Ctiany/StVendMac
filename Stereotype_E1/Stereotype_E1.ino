#include <bits/stdc++.h>
#include <Keypad.h>
#include <Adafruit_ST7789.h>
#include <ESP32Servo.h>
#include <mbedtls/md.h>
#define TFT_CS        33
#define TFT_RST       22
#define TFT_DC        25
#define TFT_MOSI      23
#define TFT_SCLK      18
#define TFT_BLK       13
#define BLK_PIN       13
#define LED_ST        12
#define LED_C1         14
#define LED_C2         27
#define LED_C3         26
#define BUZZERPIN      21
//The following section sets up the variables.

int commodityNumber;
std::vector<int> availabilities;  //storing each commodities' availability.
std::vector<int> commodityPrices; //Prices.

int genuineIDSize;
std::vector<std::string> genuineID;
std::map<std::string, int> balance; //balances
std::vector<int> comPins;
char currentLoggedUserID[11];  //The user ID used for the currently logged in user.

char keypadInputBuffer[11]; int currentKeypadInputBufferPos = 0; //message from the keypad
std::string StringMessageBuffer;   //message in the output space.
std::string currentOLEDMessage;    //message being displayed in the OLED screen
std::string temp;

int mode = 0;
int sugar = 0;
//Mode:
//0 for authentification;
//1 for choosing.

//sugar: The value used to generate dynamic texts sent.
//Obviously, there must be some other ways of dynamic encryption that are safer than this one, but I did not find them on the Internet.

static const int servoPin = 32;



//The following section conigures the keypad.

#define ROW_NUM     4 
#define COLUMN_NUM  4 
char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pin_rows[4]      = {15, 2, 0, 4};
byte pin_column[4] = {16, 17, 5, 19};
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

//The following section configures the OLED screen.
Adafruit_ST7789 tft = Adafruit_ST7789(33, 25, 22);
void setupDisplay(){
  pinMode(BLK_PIN, OUTPUT);
  analogWrite(BLK_PIN, 128);
  tft.init(240, 240);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
}

int cursorLoc = 0;

//The following section sets up the Servo Motor.
Servo servo1;



//The following section is a function used to make the buzzer buzz.

void speakerBuzz(int total_time, int delayMS){
  for(int i=1; i<=total_time; i++){
    analogWrite(BUZZERPIN, 30);
    delay(delayMS); //Delaying
    analogWrite(BUZZERPIN ,0);
    delay(delayMS); //Delaying
  }
}

//Emergency function

void emergency_HALT(){
  while(1){
    speakerBuzz(1, 1000);
  }
}

//The following section defines some functions related to displaying message on the OLED Screen.

void screenText(char *text, uint16_t color){ 
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}
void OLEDwhiteTextInc(char *text){
  currentOLEDMessage += text;
  screenText(currentOLEDMessage.data(), ST77XX_WHITE);
}
void OLEDwhiteTextCov(char *text){
  currentOLEDMessage = text;
  tft.fillScreen(ST77XX_BLACK);
  screenText(currentOLEDMessage.data(), ST77XX_WHITE);
}

//The following section gets the available stocks.

void getCommodityProperties(){
  //In the first stereotype, we will just prescribe it.
  //Obviously, this is not true for real vending machines.
  //In the "Future Work" section, it is mentioned that in the following versions, the flash memory would be used.
  commodityNumber = 2;  //Including 0. That is actually 4 commodities, but 0 counted.
  availabilities.push_back(2);
  availabilities.push_back(3);
  availabilities.push_back(4);
  commodityPrices.push_back(10);
  commodityPrices.push_back(7);
  commodityPrices.push_back(12);

}

//This function verifies whether verification could be made for buffer length.
//Actually, this value is gigantically smaller than the maximum length of an std::string type variable.
//But it doesn't bother much.
void cond_MERGE(std::string a, std::string b){
  if(a.size() >= 1000000 || b.size() >= 1000000 || a.size() + b.size() >= 1000000)emergency_HALT();
}

//This function gets the corresponding pins.
void getCorPins(){
  comPins.push_back(14);
  comPins.push_back(27);
  comPins.push_back(26);
}


//The following section checks ones genuity
std::string sha256(const std::string &str)
{
  //Due to the security problem of MD5, I would consider using SHA256 instead.
  char *key = "thisIsAMachine112@@!!ThisIsAMachine>>!";
  char *payload = const_cast<char*>(str.data());
  std::string rest;
  byte hmacResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  const size_t payloadLength = strlen(payload);
  const size_t keyLength = strlen(key);  
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, keyLength);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *) payload, payloadLength);
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);
  for(int i= 0; i< sizeof(hmacResult); i++){
    char str[3];
    sprintf(str, "%02x", (int)hmacResult[i]);
    rest += str;
  }      
  Serial.println(rest.data());
  return rest;
}

bool isGenuine(std::string unique_ID){
  std::string shaed_ID = sha256(unique_ID);
  for(auto &u: genuineID){
    if(shaed_ID == u)return true;
  }
  return false;
}

//The following section controls a simulation of dispensing of dispensing of goods.

void turnServoMotor(int deg){
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    delay(20);
  }
  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    delay(20);
  }
  for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    delay(20);
  }
  //In this version, there is only one servo motor. However, for more complicated borads, there could be more servo motors. In such case, the only modification needed is to initialize more motors and determine which servo motor to turn after verifying the commodity number.
  return;

}

int dispense(int commodityID, std::string UID){
  //CONTACT THE SERVER.
  sugar ++;
  std::string sendee = "UPDATE" + sha256(temp + std::to_string(sugar)) + UID;
  Serial.println(F("TEMP / SUGAR :"));
  Serial.println(temp.data());
  Serial.println((std::to_string(sugar)).data());
  std::string num_update = std::to_string(commodityPrices[commodityID]);
  while(num_update.size() < 10)num_update = "0" + num_update;
  sendee += num_update;
  std::string updateStatus = getWebContent(sendee);
  if(updateStatus == "AUTHFAIL")return 0;
  else if(updateStatus == "NOTFOUND")return 1;
  else if(updateStatus == "NOTDIGIT")return 2;
  else if(updateStatus != "OPERDONE")return 3;
  availabilities[commodityID] --;
  balance[sha256(currentLoggedUserID)] -= commodityPrices[commodityID];
  digitalWrite(comPins[commodityID], HIGH);
  speakerBuzz(3, 500);
  sleep(2);
  turnServoMotor(180);
  digitalWrite(comPins[commodityID], LOW);
  speakerBuzz(10, 50);
  return 4;
}

//The following section charges the display of certain messages.
void commoditiesMessage(){
  tft.fillScreen(ST77XX_BLACK);
  StringMessageBuffer = "";
  int u_temp = -1;
  for(auto &u: availabilities){
    u_temp ++;
    cond_MERGE(StringMessageBuffer, "Commidity ID " + std::to_string(u_temp) + " $ " + std::to_string(commodityPrices[u_temp]) + ": " + std::to_string(u) + " Available \n");
    StringMessageBuffer += "Commidity ID " + std::to_string(u_temp) + " $ " + std::to_string(commodityPrices[u_temp]) + ": " + std::to_string(u) + " Available \n";
  }
  cond_MERGE(StringMessageBuffer, "Please verify your identity using the keypad.\nID: ");
  StringMessageBuffer += "Please verify your identity using the keypad.\nID: ";
  OLEDwhiteTextCov(StringMessageBuffer.data());
}

//The following section is for message displayed at choosing items.

void choosingMessage(){
  StringMessageBuffer = "";
  tft.fillScreen(ST77XX_BLACK);
  int u_temp = -1;
  for(auto &u: availabilities){
    u_temp ++;
    cond_MERGE(StringMessageBuffer, "Commidity ID " + std::to_string(u_temp) + " $ " + std::to_string(commodityPrices[u_temp]) + ": " + std::to_string(u) + " Available \n");
    StringMessageBuffer += "Commidity ID " + std::to_string(u_temp) + " $ " + std::to_string(commodityPrices[u_temp]) + ": " + std::to_string(u) + " Available \n";
  }
  cond_MERGE(StringMessageBuffer, "\nYour balance: " + std::to_string(balance[sha256(currentLoggedUserID)]));
  StringMessageBuffer += "\nYour balance: " + std::to_string(balance[sha256(currentLoggedUserID)]);
  cond_MERGE(StringMessageBuffer, "\nUse D key to exit.\nProduct: ");
  StringMessageBuffer += "\nUse D key to exit.\nProduct: ";
  OLEDwhiteTextCov(StringMessageBuffer.data());
}

//This function cleans the input and output buffer.
void cleanBuffer(){
  memset(keypadInputBuffer, 0, sizeof(keypadInputBuffer));
  StringMessageBuffer = "";
  currentKeypadInputBufferPos = 0;
}

//This function charges jumping between modes, or for a single mode.
void jumpMode(int toMode){
  cleanBuffer();
  mode = toMode;
  switch(mode){
    case 0:
      digitalWrite(LED_ST, LOW);
      commoditiesMessage();
      break;
    case 1:
      digitalWrite(LED_ST, HIGH);
      choosingMessage();
      break;
  }
}
//This function converts a verified char* type to int.
int charastraytoInt(char* text){
  std::string strText = text;
  int tt = 0;
  for(int i=strText.size()-1, j = 0; i>=0; i--, j++)tt += (strText[i] - '0') * pow(10, j);
  return tt;
}
//This function verifies the product.
//0 -> Not a commodity
//1 -> Okay
//2 -> Not enough balance
//3 -> Not enough in stock
//4 -> Error in authentification.
int isOkayToPurchase(char *comNumber, std::string strUID){
  strUID = sha256(strUID);
  std::string strComNumber = comNumber;
  for(auto &u: strComNumber){
    if(! (u >= '0' && u <= '9'))return 0;
  }
  //Convert it to numbers
  //Overflow error has been prevented in the keypad.
  int intNum = charastraytoInt(strComNumber.data());
  if(intNum < 0 || intNum > commodityNumber)return 0;
  //Mapping addresses will have been verified in the previous verification step.
  //However, here it will be verified again.
  if(balance.find(strUID) == balance.end())return 4;
  //If the address exists, then check the detail.
  if(commodityPrices[intNum] > balance[strUID])return 2;
  if(availabilities[intNum] <= 0)return 3;
  return 1;
}

//This function verifies data gathered from the USER.

void verifyDetail(){
  bool status = isGenuine(keypadInputBuffer);
  if(status == false){
    OLEDwhiteTextCov("Incorrect detail.");
    sleep(2);
    tft.fillScreen(ST77XX_BLACK);
    jumpMode(0);
    //A return here would be preferred. However without return the code would still run, because there are no following lines to be executed.
  }
  else {
    OLEDwhiteTextCov("Verification Successful. Use the keypad to input your choice. D key for exiting.");
    //Clean the data, yet for the following choice.
    sleep(2);
    tft.fillScreen(ST77XX_BLACK);
    memset(currentLoggedUserID, 0, sizeof(currentLoggedUserID));
    for(int i=0; i<currentKeypadInputBufferPos; i++)currentLoggedUserID[i] = keypadInputBuffer[i];
    jumpMode(1);
    return;
  }
}



//This function verifies the purchase.

void verifyPurchase(char *comNumber, char *rawUID){
  int pStates = isOkayToPurchase(comNumber, rawUID);
  switch(pStates){
    case 0:
      OLEDwhiteTextCov("This is not a correct commodity number.");
      break;
    case 2:
      OLEDwhiteTextCov("You do not have enough balance to buy it.");
      break;
    case 3:
      OLEDwhiteTextCov("No enough stock.");
      break;
    case 4:
      OLEDwhiteTextCov("Error in authentication process.");
      break;
    case 1:
      OLEDwhiteTextCov("Dispensing product in a second. Please wait. If no products are dispensed, then the verification server may be down.");
      sleep(2);
      tft.fillScreen(ST77XX_BLACK);
      int temp_COMNUM = charastraytoInt(comNumber);
      //jumpMode has to be put below it, because otherwise it would modify
      //the content the pointer *comNumber is pointing to, -> ...
      //causing the commodityID becoming 0.
      int dis_s = dispense(temp_COMNUM, sha256(currentLoggedUserID));
      if(dis_s != 4){
        OLEDwhiteTextCov("Communication with server has failed.");
        sleep(2);
      }
      break;
  }
  if(pStates == 1)return;
  sleep(2);
  tft.fillScreen(ST77XX_BLACK);
  jumpMode(1);
}

//The following function connects the board to Wifi.

#include <WiFi.h>

void connectToWifi(){
  //Can it even be safer, technically?
  //If you can directly send encrypted password to the router,
  //then anyone can connect to your Wi-Fi!
  WiFi.mode(WIFI_STA);
  WiFi.begin("LOREM_IPSUM", "LOREM_IPSUM");


  /*
  
  
      Note: Plase fill in your Wi-Fi details in the code before burning the code.
      Fill it in like this:

      WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWORD)

  
  */
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
  }
  Serial.println("Connected.");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

//The function for web content retrieveal
std::string getWebContent(std::string messageSent){
  WiFiClient localClient;
  while(! localClient.connected()){
    localClient.connect(IPAddress(-1,-1,-1,-1), -1);

    /*
    
      Note: Again, please fill it the blanks with the IP address and port number of the server like this:
      localClient.connect(IPAddress(YOUR_IP_ADDRESS), SERVER_PORT);

      Note that YOUR_IP_ADDRESS should include 4 separated integer variables, for example:
      IPAddress(192, 168, 71, 3)

      and the port number is indicated in the server code.
    */

  }
  localClient.println(messageSent.data());
  while(! localClient.available());
  std::string temp_data = localClient.readStringUntil('\n').c_str();
  return temp_data;
}

std::string stringPiece(std::string a, int begPos, int endPos){
  std::string b;
  for(int i = begPos; i <= endPos; i++)b.push_back(a[i]);
  return b;
}

void verify_on_boot(){
  //Send a request to the server.
  //The server will verify the IP and send back the hashed data.
  //Obviously the message could be intercepted, but since it is encrypted,
  //it should be fine.
  cond_MERGE(temp, std::to_string(commodityNumber));
  temp += std::to_string(commodityNumber);
  for(auto &u :commodityPrices){ 
    cond_MERGE(temp, std::to_string(u));
    temp += std::to_string(u);
  }
  for(auto &u :comPins){
    cond_MERGE(temp, std::to_string(u));
    temp += std::to_string(u);
  }
  std::string mergedContent = getWebContent("VERIFY");
  sugar ++;
  //It will be 32 bits. So no need of worrying.
  if(!(stringPiece(mergedContent, 0, 63) == sha256(temp + std::to_string(sugar)))){
    Serial.println(F("NOT EQUAL. HALTING."));
    emergency_HALT();
  }
  if(mergedContent.size() < 69)emergency_HALT();
  std::string genuine_ID_str_num = stringPiece(mergedContent, 64, 68); //
  int genuine_ID_num = charastraytoInt(genuine_ID_str_num.data());
  int readingPos = 69;  
  for(int i = 0; i <= genuine_ID_num; i++){
    if(readingPos + 74 > mergedContent.size())emergency_HALT();
    std::string curUniqID = stringPiece(mergedContent, readingPos, readingPos + 63);
    std::string str_BAL = stringPiece(mergedContent, readingPos + 64, readingPos + 73);
    
    int curBal = charastraytoInt(str_BAL.data());
    genuineID.push_back(curUniqID);
    balance[curUniqID] = curBal;
    readingPos += 74;
  }
}


void setup() {
  // GET ALL THE PINS DONE
  Serial.begin(9600);
  pinMode(LED_ST, OUTPUT);
  pinMode(LED_C1, OUTPUT);
  pinMode(LED_C2, OUTPUT);
  pinMode(LED_C3, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);
  servo1.attach(servoPin);
  getCommodityProperties();
  getCorPins();
  connectToWifi();
  speakerBuzz(10, 50);
  verify_on_boot();
  setupDisplay();
  commoditiesMessage();
}

void loop() {
  char key = keypad.getKey();
  if(key){
    if(mode == 0){
      commoditiesMessage(); //Involves cleaning the screen
      StringMessageBuffer = "";
      delay(20);
      if(key != '*' && key != '#' && currentKeypadInputBufferPos < 9){
        speakerBuzz(1, 50);
        tft.fillScreen(ST77XX_BLACK);
        keypadInputBuffer[currentKeypadInputBufferPos] = key;
        for(int i = 1; i <= currentKeypadInputBufferPos + 1; i ++)StringMessageBuffer += "*";
        OLEDwhiteTextInc(StringMessageBuffer.data());
        //Since commoditiesMessage() involves cleaning the screen,
        //the whole buffer should be output.
        currentKeypadInputBufferPos ++;
        return;
      }
      else if(key == '*' || currentKeypadInputBufferPos == 9){
        tft.fillScreen(ST77XX_BLACK);
        jumpMode(0);
        return;
      }
      else if(key == '#'){
        verifyDetail();
        return;
      }
    }

    //This part deals with choosing

    if(mode == 1){
      delay(20);
      choosingMessage();
      StringMessageBuffer = "";
      if(key != '*' && key != '#' && key != 'D' && currentKeypadInputBufferPos < 9){
        speakerBuzz(1, 50);
        tft.fillScreen(ST77XX_BLACK);
        keypadInputBuffer[currentKeypadInputBufferPos] = key;
        StringMessageBuffer += keypadInputBuffer;
        OLEDwhiteTextInc(StringMessageBuffer.data());
        //Since commoditiesMessage() involves cleaning the screen,
        //the whole buffer should be output.
        currentKeypadInputBufferPos ++;
        return;
      }
      if(key == '*' || currentKeypadInputBufferPos == 9){
        //It would be converted to int type so 9 digits are already enough.     
        tft.fillScreen(ST77XX_BLACK);
        jumpMode(1);
        return;
      }
      if(key == 'D'){
        OLEDwhiteTextCov("Logging out.");
        sleep(2);
        tft.fillScreen(ST77XX_BLACK);
        jumpMode(0);
        return;
      }
      //PREVENT directly pressing a #
      if(key == '#' && currentKeypadInputBufferPos > 0){
        //If equal to 0, we do nothing.
        verifyPurchase(keypadInputBuffer, currentLoggedUserID);
        jumpMode(1);
        //Because the above choosingMessage() will only be run
        //when the next key is pressed.
        return;
      }
    }
  }
}
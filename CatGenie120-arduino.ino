/*
  Прошивка для Arduino UNO для CatGenie 120.
  (C) Ged http://ged.korshunov.ru
*/

#include <EEPROM.h>


#define PIN_ON 0
// #define MAX_STEP 5
int MAX_STEP;
// MAX_UPTIME - Максимальное значение, которое может принять millis
#define MAX_UPTIME 4294967290
// TEST_WORK_TIME - За это время прекращаем любые работы (2 часа)
#define TEST_WORK_TIME 7200000
// Адрес памяти для записи статуса программы
#define MEM_PROG_STATUS 0
#define MEM_PROG_CUR_STEP 1

#define PIN_CORRECT 2
#define START_TIMEOUT 120
// DELAY_FOR_REPEAT_PROGRAM - Через какое время повторится программа. В часах
#define DELAY_FOR_REPEAT_PROGRAM 11


void pinSetState(String pinList);
void pinSetState(String pinList, int State);

// Lopata down pin 1+2 - 14 sec
// Lopata up pin 1
// Lotok rotate ccw - pin 3
// Lotok rotate cw - pin 3+4
// Kanalizaciya - pin 5
// Water - pin 6
// Fan - pin 7


int curToiletStep = 0;
int programRun = 0;
String inputString = ""; // a string to hold incoming data
boolean stringComplete = false; // whether the string is complete
unsigned long NextRun = 0;
unsigned long ProgramRetry = 10000000; // В секундах
class ToiletStep {
  public:
    String Name; // Название шага
    String pin; // Маска нужных пинов
    unsigned long TimeStep; // Время действия шага в десятых секунды
    void Start();
    void Stop();
    int checkRun();
    int checkTime();
  private:
    int _run;
    unsigned long _startStep, _stopStep;
};



void ToiletStep::Start() {
  _run = 1;
  _startStep = millis();
  _stopStep = _startStep + TimeStep * 100;
  Serial.println(String( millis() / 100) + " Start step:" + Name + " pin:" + pin + " Stop in: " + _stopStep / 100);
  pinSetState(pin);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
}

void StatusWrite(int StatusProgram, int CurStep)
{
  EEPROM.update(MEM_PROG_STATUS, StatusProgram);
  EEPROM.update(MEM_PROG_CUR_STEP, CurStep);
}

int StatusRead()
{
  int progStatusRead = 0;
  progStatusRead = EEPROM.read(MEM_PROG_STATUS);
  curToiletStep = EEPROM.read(MEM_PROG_CUR_STEP);
  return progStatusRead;
}


void ToiletStep::Stop() {
  _run = 0;
  Serial.println(String( millis() / 100) + " Stop step:" + Name + " pin:" + pin);
  pinSetState(pin, 0);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED on (HIGH is the voltage level)
}



int ToiletStep::checkRun() {
  //Serial.println("Status step:" + Name + ":" + _run);
  return _run;
}



int ToiletStep::checkTime() {
  int result = 0;
  if (_startStep > millis()) result = -1;
  else if (_stopStep < millis()) result = 0;
  else result = 1;



  // Serial.println("Time step:" + Name + ":" + result);
  return result;
}

void pinSetState(String pinList)
{


  int i, a;
  int length = pinList.length();
  int newState;
  String StatePin;
  for (i = 0; i < length; i++)
  {
    StatePin = pinList.substring(i, i + 1);
    if (StatePin != "1" && StatePin != "0")
    {
      // Serial.println("pin " + String(i) + " ignored " + StatePin);



      // Serial.println("String: "+pinList+" CurChar:" + pinList.substring(i, i+1));
      continue;
    }



    a = i + PIN_CORRECT;
    pinMode(a, OUTPUT);
    if (StatePin == "1")
    {
      if (PIN_ON) newState = 1;
      else newState = 0;
    }
    else
    { if (!PIN_ON) newState = 1;
      else newState = 0;
    }



    digitalWrite(a, newState);
    Serial.println("pin " + String(i) + ":" + String(a) + " set to " + String(newState));
  }
}



void pinSetState(String pinList, int State)
{
  int i, ii;
  int length = pinList.length();
  int newState;
  String StatePin, StatePinNew;



  Serial.println("Set to state: " + pinList + " set to " + String(State));



  for (i = 0; i < length; i++)
  {
    StatePin = pinList.substring(i, i + 1);
    // Serial.println("Set to state2: " + StatePin + " set to " + String(i));
    if (StatePin == "1")
    {
      // Serial.println("Set to state3: " + StatePin + " set to " + String(i));
      StatePinNew = "";
      for (ii = 0; ii < i; ii++)
      {
        // Serial.println("Set to state4: " + StatePinNew + " set to " + String(ii));



        StatePinNew += "-";
      }
      StatePinNew += String(State);
      //Serial.println("Run set pin: " + StatePinNew + " set to " + String(ii));



      pinSetState(StatePinNew);
    }
  }
}



ToiletStep ToiletStep[15];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  // pinSetState("01-0_1");



  // pinSetState("11111111");
  // delay(5000);
  Serial.println("Start: " );



  inputString.reserve(200);



  pinSetState("00000000");
  /*
    pinSetState("1");



    runTest("1");
    runTest("01");
    runTest("001");
    runTest("0001");
    runTest("00001");
    runTest("000001");
    runTest("0000001");



  */
  // Lopata down pin 1+2
  // Lopata up pin 1
  // Lotok rotate ccw - pin 3
  // Lotok rotate cw - pin 3+4
  // Kanalizaciya - pin 5
  // Water - pin 6



  curToiletStep = 0;
  ToiletStep[curToiletStep].Name = "Wait timeout";
  ToiletStep[curToiletStep].pin = "";
  ToiletStep[curToiletStep].TimeStep = 300;



  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate down";
  ToiletStep[curToiletStep].pin = "111";
  ToiletStep[curToiletStep].TimeStep = 170;



  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Rotate lotok";
  ToiletStep[curToiletStep].pin = "//1";
  ToiletStep[curToiletStep].TimeStep = 5 * 60 * 10;



  // Трясем лопаткой
  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate up";
  ToiletStep[curToiletStep].pin = "1";
  ToiletStep[curToiletStep].TimeStep = 120;



  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate down";
  ToiletStep[curToiletStep].pin = "11";
  ToiletStep[curToiletStep].TimeStep = 80;
  /*
    curToiletStep++;
    ToiletStep[curToiletStep].Name = "Lopate up";
    ToiletStep[curToiletStep].pin = "1";
    ToiletStep[curToiletStep].TimeStep = 80;



    curToiletStep++;
    ToiletStep[curToiletStep].Name = "Lopate down";
    ToiletStep[curToiletStep].pin = "11";
    ToiletStep[curToiletStep].TimeStep = 80;



  */
  // Сбрасываем какашки
  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate up";
  ToiletStep[curToiletStep].pin = "1";
  ToiletStep[curToiletStep].TimeStep = 125;



  // Опускаем лопатку, чтобы помыть
  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate down";
  ToiletStep[curToiletStep].pin = "11";
  ToiletStep[curToiletStep].TimeStep = 140;



  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Moem lotok";
  ToiletStep[curToiletStep].pin = "//1//1";
  // ToiletStep[curToiletStep].pin = "/////1";
  ToiletStep[curToiletStep].TimeStep = 900;

  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Kanalizaciya";
  ToiletStep[curToiletStep].pin = "////1";
  ToiletStep[curToiletStep].TimeStep = 2300;



  // Сушим
  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate down";
  ToiletStep[curToiletStep].pin = "111";
  ToiletStep[curToiletStep].TimeStep = 15;

  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Fan + Rotate lotok";
  ToiletStep[curToiletStep].pin = "//11//1";
  ToiletStep[curToiletStep].TimeStep = 20000;

  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate up";
  ToiletStep[curToiletStep].pin = "1/11";
  ToiletStep[curToiletStep].TimeStep = 7;

  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Rotate lotok";
  ToiletStep[curToiletStep].pin = "//11//1";
  ToiletStep[curToiletStep].TimeStep = 1800;

  curToiletStep++;
  ToiletStep[curToiletStep].Name = "Lopate up";
  ToiletStep[curToiletStep].pin = "1";
  ToiletStep[curToiletStep].TimeStep = 163;



  MAX_STEP = curToiletStep + 1;
  curToiletStep = 0;
  ProgramRetry = 60 * 60;
  ProgramRetry = ProgramRetry * DELAY_FOR_REPEAT_PROGRAM;

  NextRun = START_TIMEOUT;

  // ToiletStep[curToiletStep].Start();
  help();
  /*
  	if (StatusRead()==1)
  	{
  		programRun = 1;
  		NextRun = millis() / 1000 + ProgramRetry;
  		ToiletStep[curToiletStep].Start();
  	}
  */
}



void loop() {
  // put your main code here, to run repeatedly:
  delay(100);

  // Ручная калибровка лопатки
  // if (false) ToiletStep[MAX_STEP-1].Start();

  // На случай переполнения millis()
  // Если слишком большое, то настраиваем запуск программы на переполнение и ждем, когда же таки переполнится.
  if (millis() > (MAX_UPTIME - TEST_WORK_TIME) && programRun == 0)
  {
    if (NextRun > START_TIMEOUT)
    {
      NextRun = START_TIMEOUT;
      help();
    }
    return;
  }


  if (millis() > NextRun * 1000 && programRun == 0)
    programStart();



  if (!ToiletStep[curToiletStep].checkRun())
  {
    programRun = 0;
    // Serial.println("Cur step not run: " + ToiletStep[curToiletStep].Name);
    // return;
  }
  /*
    if ( ToiletStep[curToiletStep].checkTime() == 0)
    {
    Serial.println("\n\rStop step: " + String(curToiletStep)+" - "+ToiletStep[curToiletStep].Name);
    ToiletStep[curToiletStep].Stop();
    }
  */
  if (programRun == 1)
  {
    /*
      if ( ToiletStep[curToiletStep].checkTime() == 0)
      {
      Serial.println("\n\rStop step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
      ToiletStep[curToiletStep].Stop();
      }
    */
    if ( ToiletStep[curToiletStep].checkTime() == 0)
    {
      programNextStep();
    }
  }
  if (stringComplete) {
    Serial.println("Get string: " + inputString);
    if (inputString.indexOf("s") == 0)
    {
      String curToiletStep_str = inputString.substring(1);
      //Serial.println("\n\rGet Number:"+curToiletStep_str);
      curToiletStep = curToiletStep_str.toInt();



      if (curToiletStep < MAX_STEP)
      {
        Serial.println("\n\r" + String( millis() / 100) + ": Start step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
        ToiletStep[curToiletStep].Start();
      }
    }
    else if (inputString.indexOf("run") == 0)
    {
      //qqqqqqq
      programStart();
      /*
        programRun = 1;
        curToiletStep = 0;
        Serial.println("\n\r" + String( millis() / 100) + ": Start step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
        ToiletStep[curToiletStep].Start();
      */
    }
    else if (inputString.indexOf("help") == 0 || inputString.indexOf("h") == 0)
    {
      help();
    }
    else if (inputString.indexOf("next") == 0 || inputString.indexOf("n") == 0)
    {
      programNextStep();
    }



    else
    {
      programStop();



    }



    // clear the string:
    inputString = "";
    stringComplete = false;
  }



}



void runTest(String TestPin)
{
  Serial.println("\n\rTest");



  Serial.println("Test : " + TestPin);



  pinSetState(TestPin, PIN_ON);
  delay(5000);
  pinSetState(TestPin, !PIN_ON);
  delay(5000);



}



void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}



void help()
{
  //unsigned long ProgramTime=0;
  for (int i = 0; MAX_STEP > i; i++)
  {
    Serial.println("Step: " + String(i) + ". " + ToiletStep[i].Name + " pin:" + ToiletStep[i].pin + " time:" + String(ToiletStep[i].TimeStep));
    //ProgramTime=ProgramTime+ToiletStep[i].TimeStep;
  }
  Serial.println("\n\rTotal load steps: " + String(curToiletStep));
  Serial.println("Command: run - run program");
  Serial.println("s# - start # step");
  Serial.println("h - help");
  Serial.println("n|next - next step of program");



  Serial.println("Cur Time: " + String(millis() / 1000) + " Next run: " + String(NextRun) + " Retry every second:" + String(ProgramRetry));
  Serial.println("any key - stop all\n\r");



}

int totalProgramTime()
{
  unsigned long ProgramTime = 0;
  for (int i = 0; MAX_STEP > i; i++)
  {
    ProgramTime = ProgramTime + ToiletStep[i].TimeStep;
  }
  return int(ProgramTime);
}

void programStart()
{
  NextRun = millis() / 1000 + ProgramRetry;
  programRun = 1;
  curToiletStep = 0;
  Serial.println("\n\r" + String( millis() / 100) + ": Start step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
  Serial.println("Cur Time: " + String(millis() / 1000) + " Next run: " + String(NextRun) + " Retry every second:" + String(ProgramRetry));
  ToiletStep[curToiletStep].Start();
  StatusWrite(programRun, curToiletStep);

}



void programStop()
{
  ToiletStep[curToiletStep].Stop();
  programRun = 0;
  Serial.println("\n\r" + String( millis() / 100) + ": Stop step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
  pinSetState("00000000");
  StatusWrite(programRun, curToiletStep);

}



void programNextStep()
{
  Serial.println("\n\rStop step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
  ToiletStep[curToiletStep].Stop();
  curToiletStep++;
  delay(1000);
  if (curToiletStep < MAX_STEP)
  {
    Serial.println("\n\rStart step: " + String(curToiletStep) + " - " + ToiletStep[curToiletStep].Name);
    ToiletStep[curToiletStep].Start();
    StatusWrite(programRun, curToiletStep);
  }
}




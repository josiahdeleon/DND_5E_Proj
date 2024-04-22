#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// declare the lcd object for auto i2c address location
hd44780_I2Cexp lcd;

// README
// Controls: 
// Scroll Up = Green, Scroll Down = Black
// Select = White, Back = Grey

// Init LCD with correct rows and columns
const int row = 4; 
const int column = 20; 
bool pageUpdate = true; 
bool curUp = false;
bool curDown = false;
bool curLeft = false;
bool curRight = false; 
bool selectPressed = false; 
bool cursorMoved = false;
int cursorColumn = 0;
int cursorRow = 0;
int cursorColOffset = 0;
int cursorRowOffset = 0; 
int buttonInit[6] = {25,26,32,33,34,35};

String charClasses[8] = {"Barbarian", "Bard", "Cleric", "Druid", "Fighter", "Sorcerer", "Wizard", "Warlock"};

class playerCharacter{
  public: 
  struct attributes
  {
    int str,dex,con,inte,wis,cha; 
  };
  String name; 
  String charClass;
  int level;
  String race; 
  String background;
  String alignment;
  String playerName;
  int experiencePoints;
  int proficiencyBonus; 
  int armorClass;
  int initiative;
  int speed; 
  int maxHp, currHp, tempHp; 
  String spellCastingClass; 
  String spellCastingAbility; 
  int spellSaveDc;
  int spellAttackBonus; 
  // function: Calculate saving throws
};

playerCharacter pc1; 
// maxRow so cursor doesn't go below the last row item
// if there are < 4 rows
int maxRow = 3;
// column offset for selector, differs depending on what page you're on
int columnOffset = 0;
// lcd read info to get what page or data you're selecting
char readData; 
// data to see what page depth were on  
int pageDepth = 0; 
// Creating interrupt routines to check which button is pressed
// to move lcd cursor appropriately
void IRAM_ATTR buttonUp()
{
  // if(cursorRow > 0)
  // {
  //   cursorRow--;
  // }
  curUp = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonDown()
{
  // if(cursorRow < 19)
  // {
  //   cursorRow++;
  // }
  curDown = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonLeft()
{
  // if(cursorColumn > 0)
  // {
  //   cursorColumn--;
  // }
  curLeft = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonRight()
{
  // if(cursorColumn < 19)
  // {
  //   cursorColumn++;
  // }
  curRight = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonSelect()
{
  selectPressed = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonBack()
{
  //selectPressed = false; 
  pageUpdate = true; 
  if(pageDepth > 0)
  {
    pageDepth--;
  }
}

// setup function to set GPIO, communication, and to set up devices
void setup() {
  Serial.begin(115200);
  for(int i = 0; i < 5; i++)
  {
    pinMode(buttonInit[i], INPUT); 
  }
  attachInterrupt(25, buttonBack, RISING);
  attachInterrupt(26, buttonSelect, RISING);
  attachInterrupt(34, buttonUp, RISING);
  attachInterrupt(33, buttonDown, RISING);
  attachInterrupt(35, buttonLeft, RISING);
  attachInterrupt(32, buttonRight, RISING);
  int status;
  // initialize LCD with number of columns and rows: 
	// hd44780 returns a status from begin() that can be used
	// to determine if initalization failed.
	// the actual status codes are defined in <hd44780.h>
	// See the values RV_XXXX
	//
	// looking at the return status from begin() is optional
	// it is being done here to provide feedback should there be an issue
	status = lcd.begin(column, row);
  if(status) // non zero status means it was unsuccesful
	{
		// begin() failed so blink error code using the onboard LED if possible
    Serial.println(String("LCD setup failed with status: ") + status);
		hd44780::fatalError(status); // does not return
	}
  pc1.name = "Bongisimo";
  pc1.charClass = charClasses[0];
  pc1.level = 20; 
  lcd.print("Setup Successful");
}

void moveCursor(int columnOffset, int cursRow)
{
  lcd.setCursor(columnOffset,cursRow);
  lcd.print("-");
}

void page1()
{
  static int page1RowCursor = 0;
  // set where the select cursor is going to be
  // cursorRow = 0; 
  // offset the selector cursor; 
  cursorColOffset = 18; 
  // have to refresh the page when entering this function to ensure
  // everything is drawn in the correct location 
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("1:Character Info");
  lcd.setCursor(0,1);
  lcd.print("2:Attributes");
  lcd.setCursor(0,2);
  lcd.print("3:Combat Info");
  // Draw cursor boy 
  // probably need to add a select pressed counter to know what nested page we're on so 
  // we can go back and forth on page depth
  if(curUp && page1RowCursor > 0)
  {
    page1RowCursor--;
    curUp = false; 
  }

  if(curDown && page1RowCursor < 3)
  {
    page1RowCursor++;
    curDown = false; 
  }

  moveCursor(cursorColOffset,page1RowCursor);
  if(selectPressed)
  {
    lcd.setCursor(0,page1RowCursor);
    readData = lcd.read(); 
    pageDepth++; 
    selectPressed = false;
  }
}

void pageCharacterInfo()
{
  // Cursor selector offset; 
  cursorColOffset = 18; 
  // cursorRow = 2;
  static int charPageRowCursor = 1; 
  static int pgSel = 0;
  static int cursorRowOffset = 0;
  static int buttonPressCount = 0;
  String classLvlBuff = pc1.charClass + ": " + pc1.level; 
  String pageCharacterInfoBuffer[6] = {"Character Name: ", "Class/Level: ", "Race: ", 
  "Background: ", "Alignment: ", "Player Name: "};
  String pageCharacterDataBuffer[6] = {pc1.name, classLvlBuff, pc1.race, pc1.background, 
  pc1.alignment, pc1.playerName};
  
  lcd.clear(); 
  lcd.setCursor(0,0);
  
  //0,2,4,6
  //1,3,5,7
  if(curUp && charPageRowCursor > 1)
  {
    charPageRowCursor-=2;
    curUp = false; 

  }

  if(curDown && charPageRowCursor < 11)
  {
    charPageRowCursor+=2;
    curDown = false; 
  }
  switch (charPageRowCursor) // TODO: need to redo this eventually
  {
    case 1:
      pgSel = 0;
      break;
    case 3:
      pgSel = 0; 
      break; 
    case 5:
      pgSel = 2;
      break;
    case 7:
      pgSel = 2;
      break;
    case 9:
      pgSel = 4;
      break;
    case 11:
      pgSel = 4;
     break;
    default:
      break; 
  }
  lcd.print(pageCharacterInfoBuffer[pgSel]);
  lcd.setCursor(0,1);
  lcd.print(pageCharacterDataBuffer[pgSel]); 
  lcd.setCursor(0,2);
  lcd.print(pageCharacterInfoBuffer[pgSel + 1]);
  lcd.setCursor(0,3);
  lcd.print(pageCharacterDataBuffer[pgSel + 1]);

  cursorRowOffset = charPageRowCursor - (pgSel*2);
  // Want to transition pages when cursor draws on 1,5,9,13 so when charPageRowCursor = numbers, pgSel+=2;
  // needs to be something like some count starting at 0 * 4 for cursor position; 
  
  moveCursor(cursorColOffset, cursorRowOffset); 
}

void pageAttributesInfo()
{
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Attributes"); 
}

void pageCombatInfo()
{
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Combat"); 
}


void displayPage()
{
  if(pageDepth == 0)
  {
    page1(); 
  }
  if(pageDepth == 1)
  {
    switch(readData)
    {
      case '1':
        pageCharacterInfo(); 
        break;
      case '2':
        pageAttributesInfo();
        break;
      case '3':
        pageCombatInfo(); 
        break;
      default:
        break; 
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(pageUpdate)
  {
    pageUpdate = false;
    // insert switch case to determine page we're on
    displayPage();
  }
  
  
}


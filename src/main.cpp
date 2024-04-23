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
bool backPressed = false;
bool cursorMoved = false;
int cursorColumn = 0;
int cursorRow = 0;
int cursorColOffset = 0;
int cursorRowOffset = 0; 
int buttonInit[6] = {25,26,32,33,34,35};

char alphNumBuff [53] = {'A','B','C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                            'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',' '};

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
// lcd read info to get what page or data you're selecting
static char readData; 
// data to see what page depth were on  
static int pageDepth = 0; 
// Creating interrupt routines to check which button is pressed
// to move lcd cursor appropriately
void IRAM_ATTR buttonUp()
{
  curUp = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonDown()
{
  curDown = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonLeft()
{
  curLeft = true;
  pageUpdate = true; 
}

void IRAM_ATTR buttonRight()
{
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
  backPressed = true;
  if(pageDepth > 0)
  {
    pageDepth--;
  }
  pageUpdate = true;
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
  pc1.name = "Bongisimo          ";
  pc1.charClass = charClasses[0];
  pc1.level = 20; 
  lcd.print("Setup Successful");
}

void moveCursor(int columnOffset, int cursRow)
{
  lcd.setCursor(columnOffset,cursRow);
  lcd.print("-");
}

// pass in the selected data feild to edit character by character
// currently have to start all the way at A and scroll to select the character you want
// TODO: Get the current character index so it doesn't start all the way at A
// probably need to use a map
void editSelectedSingleChar(String &dataToEdit)
  {
  // left/right character to edit 
  int editPos = 0;
  // up down character from a-z, A-Z
  int editVal = 0;
  pageDepth++;
  // have to refresh the page when entering this function to ensure
  // everything is drawn in the correct location 
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print(dataToEdit); 
  moveCursor(0,1);
  while(!backPressed)
   {
    if(curUp)
      {
        if(editVal > 0)
        {
          editVal--;
        }
        dataToEdit[editPos] = alphNumBuff[editVal];
        // can probably make this chunk its own function eventually for all statements
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print(dataToEdit); 
        moveCursor(editPos,1);
        pageUpdate = false; 
        curUp = false;
      }

    if(curDown)
      {
        if(editVal < 52)
        {
          editVal++;
        }
        dataToEdit[editPos] = alphNumBuff[editVal];
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print(dataToEdit); 
        moveCursor(editPos,1);
        pageUpdate = false; 
        curDown = false;
      }

    if(curLeft)
      {
        editVal = 0;
        if(editPos > 0)
        {
          editPos--;
        }
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print(dataToEdit); 
        moveCursor(editPos,1);
        pageUpdate = false; 
        curLeft = false;
      }

    if(curRight)
      {
        editVal = 0;
        if(editPos < 19)
        {
          editPos++;
        }
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print(dataToEdit); 
        moveCursor(editPos,1);
        pageUpdate = false; 
        curRight = false;
      }
   }
    backPressed = false; 
  }

void pageMainPage()
{
  static int pageMainPageRowCursor = 0;
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
  // Draw cursor boy for main page
  if(curUp && pageMainPageRowCursor > 0)
  {
    pageMainPageRowCursor--;
    curUp = false; 
  }

  if(curDown && pageMainPageRowCursor < 3)
  {
    pageMainPageRowCursor++;
    curDown = false; 
  }

  moveCursor(cursorColOffset,pageMainPageRowCursor);
  if(selectPressed)
  {
    lcd.setCursor(0,pageMainPageRowCursor);
    readData = lcd.read(); 
    pageDepth++; 
    selectPressed = false;
  }
  if(backPressed)
  {
    backPressed = false;
  }
}
// Page for specific character info
// Displays Character name, Class/Level, Race, Background, Alignment
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
  if(backPressed)
  {
    backPressed = false;
  }
  // This checks where the cursor position is and will scroll pages 
  // TODO: need to redo this eventually
  switch (charPageRowCursor) 
  {
    // Cursor over character name
    case 1:
      pgSel = 0;
      if(selectPressed)
      {
       editSelectedSingleChar(pc1.name);
       selectPressed = false;
      }
      break;
    // Cursor over Class/Level
    case 3:
      pgSel = 0; 
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
      // editSelectedSingleChar(classLvlBuff);
      selectPressed = false;
      }
      break; 
    // Cursor over race
    case 5:
      pgSel = 2;
       if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
       editSelectedSingleChar(pc1.race);
       selectPressed = false;
      }
      break;
    case 7:
      pgSel = 2;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
       editSelectedSingleChar(pc1.background);
       selectPressed = false;
      }
      break;
    case 9:
      pgSel = 4;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
       editSelectedSingleChar(pc1.alignment);
       selectPressed = false;
      }
      break;
    case 11:
      pgSel = 4;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
       editSelectedSingleChar(pc1.playerName);
       selectPressed = false;
      }
     break;
    default:
      break; 
  }

  // Prints the data for this page
  lcd.clear(); 
  lcd.print(pageCharacterInfoBuffer[pgSel]);
  lcd.setCursor(0,1);
  lcd.print(pageCharacterDataBuffer[pgSel]); 
  lcd.setCursor(0,2);
  lcd.print(pageCharacterInfoBuffer[pgSel + 1]);
  lcd.setCursor(0,3);
  lcd.print(pageCharacterDataBuffer[pgSel + 1]);

  // only draw cursor every other line
  cursorRowOffset = charPageRowCursor - (pgSel*2);
  moveCursor(cursorColOffset, cursorRowOffset); 
}
// Will dispaly character attributes eventually
void pageAttributesInfo()
{
  if(backPressed)
  {
    backPressed = false;
  }
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Attributes"); 
}
// Will display combat info eventually
void pageCombatInfo()
{
  if(backPressed)
  {
    backPressed = false;
  }
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Combat"); 
}

// Function to check what page we're on and display that page
void displayPage()
{
  if(pageDepth == 0)
  {
    pageMainPage(); 
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

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <vector>

using namespace std;
// declare the lcd object for auto i2c address location
hd44780_I2Cexp lcd;

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

vector<String> charClassesVec({"Barbarian", "Bard", "Cleric", "Druid", "Fighter", "Sorcerer", "Wizard", "Warlock"});

vector<String> charRacesVec({"Amphibian", "Mamal", "Dog", "Cat", "Bird"});

vector<String> editPageValues({"Name", "Attributes", "Classes/Level", "Combat Stats", "Race", "Background","Alignment",
                                "Player Name"});
vector<String> attributeValues({"Strength", "Dexterity", "Constitution", "intelligence", "Wisdom", "Charisma"});

vector<String> editPageCombatStats({"Experience Points", "Proficiency Bonus", "Armor Class", "Initative", "Speed",
                                      "Max Hp", "Current Hp", "Temp Hp", "SplCst Class", "SplCst Ability", "SplSv DC", "SplSv Att Bnus"});
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
// Colors will change, colors reference online hardware sim
// Green button; scrolls up
void IRAM_ATTR buttonUp()
{
  curUp = true;
  pageUpdate = true; 
}
// Black button; scrolls down
void IRAM_ATTR buttonDown()
{
  curDown = true;
  pageUpdate = true; 
}
// Blue button; scrolls left
void IRAM_ATTR buttonLeft()
{
  curLeft = true;
  pageUpdate = true; 
}
//  Yellow button; scrolls right
void IRAM_ATTR buttonRight()
{
  curRight = true;
  pageUpdate = true; 
}
// White button, selects the item the scroll bar is on
void IRAM_ATTR buttonSelect()
{
  selectPressed = true;
  pageUpdate = true; 
}
// Grey button, button to go back a page
void IRAM_ATTR buttonBack()
{
  if(pageDepth > 0)
  {
    pageDepth--;
  }
  //selectPressed = false;
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
  pc1.charClass = charClassesVec[0];
  pc1.level = 20; 
  lcd.print("Setup Successful");
}
// void displayMultiLine(int startBuffIndex, int endBuffIndex, vector<String>toBeDisplayed1, vector<String>toBeDisplayed2)
void moveCursor(int columnOffset, int cursRow)
{
  lcd.setCursor(columnOffset,cursRow);
  lcd.print("-");
}

// pass in the selected data feild to edit character by character
// currently have to start all the way at A and scroll to select the character you want
// TODO: Get the current character index so it doesn't start all the way at A
// probably need to use a map
// int editVal = map[dataToEdit[editPos]] might get the index of the current character and then display that char?
void editSelectedSingleChar(String &dataToEdit)
  {
  // left/right character to edit 
  int editPos = 0;
  // up down character from a-z, A-Z
  // int editVal = map[dataToEdit[editPos]] might get the index of the current character and then display that char?
  int editVal = 0;
  pageDepth++;
  // have to refresh the page when entering this function to ensure
  // everything is drawn in the correct location 
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print(dataToEdit); 
  moveCursor(0,1);
 
  while(pageDepth == 2)
   {
    if(curUp)
      {
        editVal--;
        if(editVal == -1)
        {
          editVal = 52; 
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
        editVal++;
        if(editVal == 53)
        {
          editVal = 0;
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
        editVal = -1;
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
        editVal = -1;
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
  }
// function to edit a predetermined field
void editClassLvl(String &playerData, vector<String> fieldBuff, int &playerLvl) 
{
  int editPos = 0;
  static int editVal = 0; 
  int movCursorOffset = 10;
  pageDepth++; 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(playerData); 
  lcd.setCursor(15,0);
  lcd.print(playerLvl);
  moveCursor(movCursorOffset,0);
  while(pageDepth == 2)
  {
    if(curLeft && (editPos > 0))
    {
      editPos--; 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(playerData); 
      lcd.setCursor(15,0);
      lcd.print(playerLvl);
      moveCursor(10,0); 
      pageUpdate = false;
      curLeft = false; 
    }
    if(curRight && (editPos < 1))
    {
      editPos++;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(playerData); 
      lcd.setCursor(15,0);
      lcd.print(playerLvl);
      moveCursor(19,0); 
      pageUpdate = false;
      curRight = false;
    }
    if(curUp) // need to change edit value
    {
      lcd.clear();
      lcd.setCursor(0,0);
      if(editPos == 0)
      {
        editVal--; 
        if(editVal == -1)
        {
          editVal = 7;
        }
        movCursorOffset = 10;
      }
      if(editPos == 1)
      {
        playerLvl++; 
        movCursorOffset = 19;
      }
      playerData = fieldBuff[editVal]; 
      lcd.print(playerData); 
      lcd.setCursor(15,0);
      lcd.print(playerLvl);
      moveCursor(movCursorOffset,0);
      pageUpdate = false;
      curUp = false;
    }
    if(curDown)
    {   
      lcd.clear();
      lcd.setCursor(0,0);
      if(editPos == 0)
      {
        editVal++;
        if(editVal == 8)
        {
          editVal = 0;
        }
        movCursorOffset = 10;
      }
      if(editPos == 1)
      {
        playerLvl--; 
        movCursorOffset = 19;
      }
      playerData = fieldBuff[editVal]; 
      lcd.print(playerData); 
      lcd.setCursor(15,0);
      lcd.print(playerLvl);
      moveCursor(movCursorOffset,0);
      pageUpdate = false;
      curDown = false;  
    }
  }
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
  lcd.setCursor(0,3);
  lcd.print("4.Edit Character");
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
  // doesn't do anything atm
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
      // if(selectPressed)
      // {
      //  editSelectedSingleChar(pc1.name);
      //  selectPressed = false;
      // }
      break;
    // Cursor over Class/Level
    case 3:
      pgSel = 0; 
      // if(selectPressed)
      // {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
      // editClassLvl(pc1.charClass, charClassesVec,pc1.level);
      selectPressed = false;
      // }
      break; 
    // Cursor over race
    case 5:
      pgSel = 2;
       if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
       //editClassLvl(pc1.race,charRacesVec);
       selectPressed = false;
      }
      break;
    case 7:
      pgSel = 2;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
      //  editSelectedSingleChar(pc1.background);
       selectPressed = false;
      }
      break;
    case 9:
      pgSel = 4;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
      //  editSelectedSingleChar(pc1.alignment);
       selectPressed = false;
      }
      break;
    case 11:
      pgSel = 4;
      if(selectPressed)
      {
      //Instead of editing the whole thing, have a buffer of classes to select/scroll through
      //  editSelectedSingleChar(pc1.playerName);
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
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Attributes"); 
}
// Will display combat info eventually
void pageCombatInfo()
{
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Combat"); 
}
void pageEditCharInfo()
{
  // sets up first page of the character edit screen
  static int editPageNum = 0; 
  static int cursorOffset = 0;
  // need to find a better way to do page depth.
  lcd.clear(); 
  for(int i = 0; i < 4; i++)
  {
    lcd.setCursor(0,i);
    lcd.print(editPageValues[i]);
  }
  moveCursor(19,0);
  // logic to display and edit your character data
  // can do some sort of page offset to use a single loop to print the page
  // can probably do if(pageDepth == whatever page depth) as well, should make it consistent
  while(pageDepth == 1)
  {
    if(curUp)
    {
      editPageNum--;
      if(editPageNum == -1)
      {
        editPageNum = 7; 
      }
      if((editPageNum >= 0) && (editPageNum < 4))
      {
        
        cursorOffset = 0;
      }
      if((editPageNum >= 4) && (editPageNum < 8))
      {
        cursorOffset = 4;
      }
      lcd.clear();
      // can maybe turn this into its own function
      for(int i = 0; i < 4; i++)
      {
        lcd.setCursor(0,(i));
        lcd.print(editPageValues[i + cursorOffset]);
      }
      moveCursor(19,(editPageNum - cursorOffset));
      curUp = false;
      pageUpdate = false;
    }
    if(curDown)
    {
      editPageNum++;
      if(editPageNum == 8)
      {
        editPageNum = 0;
      }
      if((editPageNum >= 0) && (editPageNum < 4))
      {
        
        cursorOffset = 0;
      }
      if((editPageNum >= 4) && (editPageNum < 8))
      {
        cursorOffset = 4;
      }
      lcd.clear();
      // can maybe turn this into its own function
      for(int i = 0; i < 4; i++)
      {
        lcd.setCursor(0,(i));
        lcd.print(editPageValues[i + cursorOffset]);
      }
      moveCursor(19,(editPageNum - cursorOffset));
      curDown = false; 
      pageUpdate = false;
    }
    if(selectPressed)
    {
      switch(editPageNum)
      {
      // edit player name
      case 0:
        editSelectedSingleChar(pc1.name); 
        selectPressed = false; 
        break;
      case 1:
          // TODO: Impliment
          //editAttributes()
          selectPressed = false;
          break;
      case 2:
        editClassLvl(pc1.charClass, charClassesVec ,pc1.level); 
      default:
        selectPressed = false;
        break;
      }
      // update LCD before going back to previous screen
      lcd.clear();
      // can maybe turn this into its own function
      for(int i = 0; i < 4; i++)
      {
        lcd.setCursor(0,(i));
        lcd.print(editPageValues[i + cursorOffset]);
      }
      moveCursor(19,(editPageNum - cursorOffset));
    }
  }
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
      case '4':
        pageEditCharInfo();
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

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <vector>
#include <map>


using namespace std;
// declare the lcd object for auto i2c address location
hd44780_I2Cexp lcd;

// Init LCD with correct rows and columns
const int row = 4; 
const int column = 20; 
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

//TODO: can probably remove all of these generic buffers and throw this into one big string buffer
vector<String> charClassesVec({"Barbarian", "Bard", "Cleric", "Druid", "Fighter", "Sorcerer", "Wizard", "Warlock"});

vector<String> charRacesVec({"Amphibian", "Mamal", "Dog", "Cat", "Bird"});

vector<String> editPageValues({"Name", "Attributes", "Classes/Level", "Combat Stats", "Race", "Background","Alignment",
                                "Player Name"});
vector<String> attributeValues({"Strength", "Dexterity", "Constitution", "Intelligence", "Wisdom", "Charisma"});

vector<String> editPageCombatStats({"Experience Points", "Proficiency Bonus", "Armor Class", "Initative", "Speed",
                                      "Max Hp", "Current Hp", "Temp Hp", "SplCst Class", "SplCst Ability", "SplSv DC", "SplSv Att Bnus"});

vector<String> mainPageBuff = {"1:Character Info", "2:Attributes", "3:Combat Info", "4.Edit Character"};
// Buffers for displaying information 
vector<String> pageAllCharInfoBuff;
vector<String> pageAttributesInfoBuff;

enum Pages{
  MAIN_PAGE = 0,
  CHARACTER_INFO = 1, 
  ATTRIBUTES_INFO = 2, 
  COMBAT_INFO = 3, 
  EDIT_INFO = 4,
  EDIT_SELECT_SINGLE_CHAR = 5,
  EDIT_CLASS = 7
};

enum PlayerAttributes
{
  STR = 0,
  DEX = 1,
  CON = 2,
  INT = 3,
  WIS = 4,
  CHA = 5
};

class playerCharacter{
  public: 
    // using PlayerAttributes enums for the indexes of this array
    int pcAttributes[6] = {0,0,0,0,0,0};
    int pcAttributesModifier[6] = {0,0,0,0,0,0}; 
    String name; 
    String charClass;
    int level;
    String race; 
    String background;
    String alignment;
    String playerName;
    int experiencePoints;
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
static char readData = '0';
//TODO: Find a better way to assign a changing page number to the page enum
static int pageNumber = MAIN_PAGE; 
// Creating interrupt routines to check which button is pressed
// to move lcd cursor appropriately
// Colors will change, colors reference online hardware sim
// Green button; scrolls up
void IRAM_ATTR buttonUp()
{
  curUp = true;
}
// Black button; scrolls down
void IRAM_ATTR buttonDown()
{
  curDown = true; 
}
// Blue button; scrolls left
void IRAM_ATTR buttonLeft()
{
  curLeft = true;
}
//  Yellow button; scrolls right
void IRAM_ATTR buttonRight()
{
  curRight = true;
}
// White button, selects the item the scroll bar is on
void IRAM_ATTR buttonSelect()
{
  selectPressed = true;
}
// Grey button, button to go back a page
void IRAM_ATTR buttonBack()
{
  backPressed = true;
}

void clearInterruptFlags()
{
  curUp = false;
  curDown = false;
  curLeft = false;
  curRight = false;
  selectPressed = false;
  backPressed = false; 
}
// Function to calculate attribute modifier
void calculateAttributeMod(int attribute, int &attributeMod, int proficiencyBonus)
{
  if(attribute == 12 || attribute == 13)
    {
      attributeMod = 1; 
    }
  if(attribute == 14 || attribute == 15)
    {
      attributeMod = 2; 
    }
  if(attribute == 16 || attribute == 17)
    {
      attributeMod = 3; 
    }
  if(attribute == 18 || attribute == 19)
    {
      attributeMod = 4; 
    }
  if(attribute == 20)
    {
      attributeMod = 5; 
    }
  // Add proficiency bonus if there is one.
  attributeMod = attributeMod + proficiencyBonus;
}
//Super spaghetti function to update display buffers with new
void upateDisplayBuffers()
{
  pageAllCharInfoBuff = {"Character Name: ", pc1.name, "Class/Level: ", pc1.charClass + "/" + pc1.level, 
  "Race: ", pc1.race, "Background: ", pc1.background, "Alignment: ", pc1.alignment, "Player Name: ", pc1.playerName};
  //TODO: Can probably turn this into a for loop if we want to shrink the LOC, but loop not needed
  pageAttributesInfoBuff = {"Stat: Score/Mod:", 
  attributeValues[STR] + "      " + pc1.pcAttributes[STR] + "/" + pc1.pcAttributesModifier[STR],
  attributeValues[DEX] + "     " + pc1.pcAttributes[DEX] + "/" + pc1.pcAttributesModifier[DEX],
  attributeValues[CON] + "  " + pc1.pcAttributes[CON] + "/" + pc1.pcAttributesModifier[CON],
  attributeValues[INT] + "  " + pc1.pcAttributes[INT] + "/" + pc1.pcAttributesModifier[INT],
  attributeValues[WIS] + "        " + pc1.pcAttributes[WIS] + "/" + pc1.pcAttributesModifier[WIS],
  attributeValues[CHA] + "      " + pc1.pcAttributes[CHA] + "/" + pc1.pcAttributesModifier[CHA],
  };
}
// TODO: Can probably make a generic move cursor function for all pages
void moveCursor(int columnOffset, int cursRow)
{
  lcd.setCursor(columnOffset,cursRow);
  lcd.print("-");
}
//TODO: Change this funciton later so that two references don't have to be passed
// into this function and the next function
void printEditSelectSingleChar(String &dataToEdit, int editPos)
{
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print(dataToEdit); 
  moveCursor(editPos,1);
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
// have to refresh the page when entering this function to ensure
// everything is drawn in the correct location 
printEditSelectSingleChar(dataToEdit,editPos);
while(pageNumber == EDIT_SELECT_SINGLE_CHAR)
  {
  if(curUp)
    {
      editVal--;
      if(editVal <= -1)
      {
        editVal = 52; 
      }
      dataToEdit[editPos] = alphNumBuff[editVal];
      printEditSelectSingleChar(dataToEdit,editPos);
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
      printEditSelectSingleChar(dataToEdit,editPos);
      curDown = false;
    }
  if(curLeft)
    {
      editVal = -1;
      if(editPos > 0)
      {
        editPos--;
      }
      printEditSelectSingleChar(dataToEdit,editPos);      
      curLeft = false;
    }
  if(curRight)
    {
      editVal = -1;
      if(editPos < 19)
      {
        editPos++;
      }
      printEditSelectSingleChar(dataToEdit,editPos);      
      curRight = false;
    }
  if(backPressed)
    {
      pageNumber = EDIT_INFO; 
      backPressed = false; 
    }
  }
}
void printEditClassLvl(String &playerData, int &playerLvl, int cursorOffset)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(playerData); 
  lcd.setCursor(15,0);
  lcd.print(playerLvl);
  moveCursor(cursorOffset,0);
}
// function to edit a predetermined field
void editClassLvl(String &playerData, vector<String> fieldBuff, int &playerLvl) 
{
  int editPos = 0;
  static int editVal = 0; 
  int movCursorOffset = 10;
  printEditClassLvl(playerData, playerLvl, movCursorOffset);
  while(pageNumber == EDIT_CLASS)
  {
    if(curLeft)
    {
      if(editPos > 0)
        {
          editPos--; 
          printEditClassLvl(playerData, playerLvl, 10);
        }      
      curLeft = false; 
    }
    if(curRight)
    {
      if(editPos < 1)
      {
        editPos++;
        printEditClassLvl(playerData, playerLvl, 19);
      }  
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
      printEditClassLvl(playerData, playerLvl, movCursorOffset);
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
      printEditClassLvl(playerData, playerLvl, movCursorOffset);
      curDown = false;  
    }
    if(backPressed)
    {
      pageNumber = EDIT_INFO;
      backPressed = false;
    }
  }
}

// Function to take an input string buffer and display it on the LCD
// String buffers should be defined in each page function
// Each String index should be one line that you want to print on the LCD
void printPageDisplayInfo(vector<String> infoBuffer, int buffOffset)
{
  // have to refresh the page when entering this function to ensure
  // everything is drawn in the correct location 
  lcd.clear();
  for(int i = 0+buffOffset, j = 0; i <= 3+buffOffset; i++)
  {
   lcd.setCursor(0,j);
   lcd.print(infoBuffer[i]);
   j++; 
  }
}
// Main Select Page
//TODO: maybe add just one top level summary page
void pageMainPage()
{
  static int pageMainPageRowCursor = 0;
  int cursorColOffset = 18;
  
  printPageDisplayInfo(mainPageBuff, 0);
  // Draws cursor selection on main page
  moveCursor(cursorColOffset,pageMainPageRowCursor);
  while(pageNumber == MAIN_PAGE)
  {
      if(curUp)
      {
        if(pageMainPageRowCursor > 0)
        {
          pageMainPageRowCursor--;
          printPageDisplayInfo(mainPageBuff, 0);
          moveCursor(cursorColOffset,pageMainPageRowCursor);
        }
        curUp = false; 
      }
      if(curDown)
      {
        if (pageMainPageRowCursor < 3)
        {
          pageMainPageRowCursor++;
          printPageDisplayInfo(mainPageBuff, 0);
          moveCursor(cursorColOffset,pageMainPageRowCursor);
        }
        curDown = false; 
      }
      if(selectPressed)
      {
        pageNumber = pageMainPageRowCursor + 1;
        selectPressed = false;
      }
  }
}
// General function to print out pages that only display data
void printPageNoEdit(vector<String> printBuff)
{
  int buffOffset = 0; 
  int maxPrintBuffSize = printBuff.size();
  Serial.println(maxPrintBuffSize);
  lcd.clear(); 
  printPageDisplayInfo(printBuff, buffOffset);
  while(pageNumber != MAIN_PAGE)
  {
    if(curUp)
    {
      if (buffOffset > 1)
      {
        buffOffset-=4;
        printPageDisplayInfo(printBuff, buffOffset);
      }
      curUp = false; 
    }
    if(curDown)
    {
      if(buffOffset < printBuff.size())
      {
        buffOffset+=4;
        printPageDisplayInfo(printBuff, buffOffset);
      }
      curDown = false; 
    }
    if(backPressed)
    {
      backPressed = false;
      pageNumber = MAIN_PAGE;
    }
  }
}

// Will display combat info eventually
void pageCombatInfo()
{
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("WIP Combat"); 
  while(pageNumber == COMBAT_INFO)
  {
    if(backPressed)
    {
      pageNumber = MAIN_PAGE;
      backPressed = false; 
    }
    // need to add a delay for now because without it the interrupt doesn't trigger for some reason
    delay(1);
  }
}
//TODO: Remove when all edit pages are made
void pageEditInfoWIP()
{
  lcd.clear(); 
  lcd.setCursor(0,0);
  lcd.print("WIP Edit");
  while(pageNumber != EDIT_INFO)
  {
    if(backPressed)
    {
      pageNumber = EDIT_INFO;
      backPressed = false;
    }
    delay(1);
  }
}
void printPageEditCharInfo(int editPageNum, int offset)
{
  lcd.clear();
  for(int i = 0; i < 4; i++)
  {
    lcd.setCursor(0,i);
    lcd.print(editPageValues[i + offset]);
  }
    moveCursor(19,(editPageNum - offset));
}
void pageEditCharInfo()
{
  // sets up first page of the character edit screen
  static int editPageNum = 0; 
  static int cursorOffset = 0;
  printPageEditCharInfo(editPageNum, cursorOffset);
  // logic to display and edit your character data
  while(pageNumber == EDIT_INFO)
  {
    if(curUp)
    {
      editPageNum--;
      if(editPageNum <= -1)
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

      printPageEditCharInfo(editPageNum, cursorOffset);
      curUp = false;
    }
    if(curDown)
    {
      editPageNum++;
      if(editPageNum >= 8)
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
      printPageEditCharInfo(editPageNum, cursorOffset);
      curDown = false; 
      
    }
    if(selectPressed)
    {
      selectPressed = false;
      //We are on EDIT_INFO which is page 4 and then need to add + 1 because
      //editPageNum is zero indexed and we are using editPageNum here to do 
      //TODO: can probably make some sort of array or map to hold the below value and
      //pass that into the edit function. 
      pageNumber = editPageNum + EDIT_INFO + 1;
    }
    if(backPressed)
    {
      //pageNumber is changed back to MAIN_PAGE here since it's a static global.
      backPressed = false; 
      pageNumber = MAIN_PAGE;
    }
  }
}
// Function to check what page we're on and display that page
void displayPage()
{
//TODO: Maybe find a better place to put the buffers that need to update when edited
upateDisplayBuffers();
  switch(pageNumber)
  {
    case MAIN_PAGE:
      pageMainPage();
      break;
    case CHARACTER_INFO:
      printPageNoEdit(pageAllCharInfoBuff); 
      break;
    case ATTRIBUTES_INFO:
      printPageNoEdit(pageAttributesInfoBuff);
      break;
    case COMBAT_INFO:
      pageCombatInfo(); 
      break;
    case EDIT_INFO:
      pageEditCharInfo();
      break;
    case EDIT_SELECT_SINGLE_CHAR:
      editSelectedSingleChar(pc1.name); 
      break;
    case 6: 
      pageEditInfoWIP();
      break;
    case EDIT_CLASS:
      editClassLvl(pc1.charClass,charClassesVec,pc1.level);
    default:
      break; 
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
  pc1.name = "Bongisimo          ";
  pc1.charClass = charClassesVec[0];
  pc1.level = 20; 
  lcd.print("Setup Successful");
  pc1.pcAttributes[STR] = 13;
}

void loop() {
  // insert switch case to determine page we're on
    displayPage();
  }

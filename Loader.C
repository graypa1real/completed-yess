/**
 * Names:
 * Team:
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>

#include "Loader.h"
#include "Memory.h"

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
 
   loaded = false;
   if(argc < 2)
       return;
   else
   {
       //std::cout << "here";
       //opens the file and sets opened to true if it worked
       bool openable = Loader::canLoad(argv[1]);
    
   
       // returns if the file failed to open.
       if (!openable)
       { 
           return;
       }
       //std::cout << "hi" << std::endl;     
       //The file handle is declared in Loader.h.  You should use that and
       //not declare another one in this file.
       
       std::string line;
       bool failed = loadFile();
       //std::cout << "here" << std::endl;
       if(failed)
       {
           loaded = false;
           return;
       }
       //Next write a simple loop that reads the file line by line and prints it out
       
   //Next, add a method that will write the data in the line to memory 
   //(call that from within your loop)

   //Finally, add code to check for errors in the input line.
   //When your code finds an error, you need to print an error message and return.
   //Since your output has to be identical to your instructor's, use this cout to print the
   //error message.  Change the variable names if you use different ones.
   //  std::cout << "Error on line " << std::dec << lineNumber
   //       << ": " << line << std::endl;


   //If control reaches here then no error was found and the program
   //was loaded into memory.
   loaded = true;  
   //std::cout << "loaded is true" << std::endl;
   return;
   }
}
bool Loader::loadFile()
{
    std::string line;
    int32_t lineNumber = 0;
    bool failed = false;
    int32_t pastAddr = 0;
    int32_t currAddr = 0;
    while(std::getline(inf,line))
    {
        lineNumber++; 
        //std::cout << "what" << std::endl;
        
        if(Loader::hasErrors(line))
        {
            std::cout << "Error on line " << std::dec << lineNumber 
                     << ": " << line << std::endl;
            failed = true;
            return failed;
        }
        if(convert(line,ADDRBEGIN, ADDREND) >= 0)
        {
            currAddr = convert(line, ADDRBEGIN, ADDREND);
        }
        if(pastAddr >= currAddr && currAddr > 1)
        {
            failed = true;
            std::cout << "Error on line " << std::dec << lineNumber << ": "
                     << line << std::endl;
            return failed;
        }
        //std::cout << "no error on this line" << std::endl;
        loadLine(line, currAddr);
        if(line[7] != ' ')
        {
            pastAddr = currAddr + (dataCount(line) / 2)-1;
        }
        //std::cout << line; 
               
    }
    return failed; 
}
/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
   return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.

bool Loader::canLoad(char fileName[])
{
   
    std::string file = fileName;
    int len = file.length();
    //this if statement has been tested.  it will return false if the file does not end
    //with a .yo 
    if(fileName[len - 3] != '.' || fileName[len - 2] != 'y' || fileName[len-1] != 'o')
    {        
        return false;
    }
    inf.open(fileName, std::ifstream::in);
    if(!inf.is_open())
    {
        return false;
    }
    //std::cout << "opened" << std::endl;
    return true;
}
//calls the helper methods to check the address, colon, space after colon, data is in hex, there is a space after data, and a pipe at 28
//returns true if there are any errors.
bool Loader::hasErrors(std::string &line)
{
    bool acwc = line[0] != ' '; 

    if(addrCheckWithColon(line)|| dataCheck(line, acwc) || pipeCheck(line))
     {
         
         return true;
     }
    
    return false;
}
//this method will return true if there is an error in the address, or if there is not a colon in the correct place.
bool Loader:: addrCheckWithColon(std::string &line)
{
    bool blankLine = false;
    if(line[0] == ' ')
        blankLine = true;
    if((!(line[0] == '0') ||!(line[1] == 'x')) && !blankLine)
    {    
        //std::cout << "here";
        return true;
    }
    for(int i = 2; i <= ADDREND; i++)
    {
      
        if(!((line[i] >= '0'&& line[i] <= '9') || (line[i] >='a' && line[i] <= 'f') || line[i] == ' '))
        {   
             return true;
        }
    }
    if(!(blankLine|| line[ADDREND + 1] == ':'))
    {
    
        return true;
    }
    if(line[ADDREND + 2] != ' ')
    {
        
        return true; 
    }
     
    return false;
}
//this method will return true if there is an error in the data.  an error is slots 6 through 27 having something that is not a space or hex.
bool Loader:: dataCheck(std::string &line, bool addExist)
{
    
    int currAddr = convert(line, ADDRBEGIN, ADDREND);
    
    int dataPoint = 0;
    if(line[DATABEGIN] != ' ' && !addExist)
        return true;
    dataPoint = dataCount(line);
    //std::cout << dataPoint << std::endl;
    if(dataPoint == -1)
    {
        return true;
    }
    if(dataPoint % 2 != 0)
    {
        return true;
    }
    if((currAddr + dataPoint -1) > (MEMSIZE))
    {
        
        return true;
    }
    return false;
}
int Loader::dataCount(std::string &line)
{
    int i = DATABEGIN;
    //std::cout << line[i] << std::endl;
    bool lineHasEnded = false;
    int count = 0;
    while (i < COMMENT)
    {
        if(i == (COMMENT -1) && line[i] != ' ')
        {
            return -1;
        }
        if(line[i] == ' ')
        {
            lineHasEnded = true;
        }
        if(lineHasEnded && (line[i] != ' '))
        {    
           // std::cout << "here"  << std::endl;
            return -1;
        }
        if(!((line[i] >= '0' && line[i] <= '9') || (line[i] >= 'a' && line[i] <= 'f') || lineHasEnded))
        {    
            //std::cout <<  i  << std::endl;
            return -1;
        }
        if((line[i] >= '0' && line[i] <= '9') || (line[i] >= 'a' && line[i] <= 'f'))
        {    
            count++;
        }
        i++;
    }
    return count;
}
//this method will return true if there is a character that is not a | in slot 28.
bool Loader:: pipeCheck(std::string &line)
{
    if(line[COMMENT] != '|')
    {    
      
        return true;
    }
    
    return false;
}

void Loader::loadLine(std::string &line, int32_t linNum)
{
    int firByt = DATABEGIN;
    
    if(line[firByt] == ' ')
    {
        return;
    }
    int32_t firBytD = convert(line, firByt, firByt+1);
 
    
    
    bool imemErr = false;
    while(isxdigit(line[firByt]))
    {
          
        Memory::getInstance()->putByte(firBytD, linNum, imemErr);
        
        linNum += 1;
        firByt += 2;
        
        
        firBytD = convert(line, firByt, firByt+1);
        
        
        
    }
    
    return;
}
int32_t Loader::convert(std::string &line, int begin, int end)
{
    int addr = 0;
    while(begin <= end)
    {
        //std::cout << begin; 
        //std::cout << line[begin];
        if(line[begin] >= '0' && line[begin] <= '9')
        {
            addr *= 16;
            addr += line[begin] - '0';
        }
        else if(line[begin] >= 'a' && line[begin] <= 'f')
        {
            addr *= 16;
            addr += line[begin] - 'a' + 10;
        }
        else if(line[begin] == ' ')
        {
            return addr;
        }
        else
      {
            return -1;
        }
        begin++;
    }
    return addr;
}

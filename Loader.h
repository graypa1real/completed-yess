
class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      bool canLoad(char line[]);       //checks if the file can be opened
      bool hasErrors(std::string &line);
      bool addrCheckWithColon(std::string &line);
      int dataCount(std::string &line);
      bool dataCheck(std::string &line, bool addExist);
      bool loadFile();
      bool pipeCheck(std::string &line);
      void loadLine(std::string &line, int32_t linNum);
      int32_t convert(std::string &line,int begin, int end); 
      std::ifstream inf;  //input file handle
   public:
      Loader(int argc, char * argv[]);
      bool isLoaded();
};

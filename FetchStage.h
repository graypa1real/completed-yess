//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      int selectPC(PipeReg ** pregs);
      bool needRegIds(uint64_t ficode);
      bool needValC(uint64_t ficode);
      int predictPC(uint64_t ficode, uint64_t fvalC, uint64_t fvalP);
      uint64_t buildStat(bool imemError, bool instr_valid, uint64_t icode);
      uint64_t memCheckicode(bool error, uint64_t ifun);
      uint64_t memCheckifun(bool error, uint64_t ifun);
      bool validInstruction(uint64_t icode);
      void stallF(PipeReg ** pregs, E * ereg, Stage ** stages);
      void stallD(E * ereg, Stage ** stages);
      void bubbleD(PipeReg ** pregs, E * ereg, Stage ** stages);
      void getRegIds(uint64_t f_pc, uint64_t & rA, uint64_t & rB);
      uint64_t buildValC(uint64_t f_pc, uint64_t icode);
      void updateBubble(D * dreg);
      uint64_t PCincrement(uint64_t f_pc, bool needRegIds, bool);
      void calculateControlSignals(PipeReg ** pregs, E * ereg, Stage ** stages);
      bool F_stall, D_stall, D_bubble;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};

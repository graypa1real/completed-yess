class WritebackStage: public Stage
{
           private:
               void writeReg(uint64_t dstM, uint64_t valM);
           public:
               bool doClockLow(PipeReg ** pregs, Stage ** stages); 
               void doClockHigh(PipeReg ** pregs);

};



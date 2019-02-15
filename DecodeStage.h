class DecodeStage: public Stage
{
       private:
            uint64_t buildsrcA(D * dreg, uint64_t icode);
            uint64_t buildsrcB(D * dreg, uint64_t icode);
            uint64_t builddstE(D * dreg, uint64_t icode);
            uint64_t builddstM(D * dreg, uint64_t icode);
            uint64_t buildvalA(uint64_t srcA, PipeReg ** preg, Stage ** stages);
            uint64_t buildvalB(uint64_t srcB, PipeReg ** preg, Stage ** stages);
            bool calculateControlSignals(E * ereg, Stage ** stages);
            uint64_t d_valA, d_valB;
            bool E_bubble;
       public:
           bool doClockLow(PipeReg ** pregs, Stage ** stages);
           void doClockHigh(PipeReg ** pregs);
           void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
           uint64_t getsrcA();
           uint64_t getsrcB();

};


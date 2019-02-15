class MemoryStage: public Stage
{
    private:
        bool memRead(uint64_t icode);
        bool memWrite(uint64_t icode);
        bool accessMemory(uint64_t memAddr, bool read, bool write, uint64_t valA);
        uint64_t makeAddr(M * mreg);
        uint64_t valM;
		uint64_t stat;
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
        void setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM);
        uint64_t getValM();
	uint64_t getm_stat();
};



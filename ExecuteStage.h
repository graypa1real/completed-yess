class ExecuteStage: public Stage
{
    private:
        uint64_t alu(E * ereg, ConditionCodes * cc, PipeReg ** pregs, Stage ** stages);
        uint64_t aluAfun(E * ereg);
        uint64_t aluBfun(E * ereg);
        uint64_t alufun(E * ereg);
        bool set_cc(E * ereg, PipeReg ** pregs, Stage ** stages);
        uint64_t makee_dstE(E * ereg, bool cnd);
        bool makeCnd(uint64_t icode, uint64_t ifun);
        uint64_t e_valE, e_dstE, e_Cnd;
		void calculateControlSignals(PipeReg ** pregs, Stage ** stages);
		bool M_bubble; 
    public:
        bool doClockLow(PipeReg ** pregs, Stage ** stages);
        void doClockHigh(PipeReg ** pregs);
        void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
        uint64_t getDstE();
        uint64_t getValE();
	    bool getCnd();	
};



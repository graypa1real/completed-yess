#include <string>
#include <cstdint>
#include <iostream>
#include "Tools.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ConditionCodes.h"
#include "Instructions.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"

uint64_t e_dstE;
uint64_t e_valE;
uint64_t e_Cnd;
bool M_bubble; 

bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    uint64_t stat = ereg->getstat()->getOutput();
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t ifun = ereg->getifun()->getOutput();
    //uint64_t cnd = 0;
    //uint64_t valE = ereg->getvalC()->getOutput();
    e_Cnd = makeCnd(icode, ifun);
    uint64_t valA = ereg->getvalA()->getOutput();
    e_dstE = makee_dstE(ereg, e_Cnd);
    uint64_t dstM = ereg->getdstM()->getOutput();
    ConditionCodes * cc = ConditionCodes::getInstance();
    e_valE = alu(ereg, cc, pregs, stages); //here
    calculateControlSignals(pregs, stages);
    setMInput(mreg, stat, icode, e_Cnd, e_valE, valA, e_dstE, dstM);
    return false;
}
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
	if(!M_bubble) //normalM
	{
		M * mreg = (M *) pregs[MREG];
		mreg->getstat()->normal();
		mreg->geticode()->normal();
		mreg->getCnd()->normal();
		mreg->getvalE()->normal();  
		mreg->getvalA()->normal();  
		mreg->getdstE()->normal();
		mreg->getdstM()->normal();
	}
	else //bubbleM
	{ 
		M * mreg = (M *) pregs[MREG];
		mreg->getstat()->bubble(SAOK);
		mreg->geticode()->bubble(INOP);
		mreg->getCnd()->bubble(0);
		mreg->getvalE()->bubble(0);
		mreg->getvalA()->bubble(0);
		mreg->getdstE()->bubble(RNONE);
		mreg->getdstM()->bubble(RNONE);
	}
	
    
}
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                             uint64_t cnd, uint64_t valE, uint64_t valA,
                             uint64_t dstE, uint64_t dstM)
{
    mreg->getstat()->setInput(stat);
    mreg->geticode()->setInput(icode);
    mreg->getCnd()->setInput(cnd);
    mreg->getvalE()->setInput(valE);
    mreg->getvalA()->setInput(valA);
    mreg->getdstE()->setInput(dstE);
    mreg->getdstM()->setInput(dstM);
}
//getValE returns e_valE so that the decode stage can have forwarding.
uint64_t ExecuteStage::getValE()
{
    return e_valE;
}
//getDstE returns e_dstE so that DecodeStage can implement forwarding.
uint64_t ExecuteStage::getDstE()
{
    return e_dstE;
}
bool ExecuteStage::getCnd()
{
    return e_Cnd;
}

uint64_t ExecuteStage::alu(E * ereg, ConditionCodes * cc, PipeReg ** pregs, Stage ** stages)
{
    bool of = false;
    uint64_t alu_A = aluAfun(ereg);
    uint64_t alu_B = aluBfun(ereg);
    int fun = alufun(ereg);
    bool error = false;
    bool needCC = set_cc(ereg, pregs, stages);
    //uint64_t dstE = e_dstE(ereg, needCC);
    uint64_t answer = 0;
    if(fun == ADDQ)
    {
        answer = alu_A + alu_B;
        of = Tools::addOverflow(alu_A, alu_B);
    }
    else if(fun == XORQ)
    {
        answer = (alu_B ^ alu_A);
    }
    else if(fun == SUBQ)
    {
        of = Tools::subOverflow(alu_A, alu_B);
        answer = alu_B - alu_A;
    }
    else
    {
        answer = alu_A & alu_B;
    }
    if(needCC)
    {
        
        cc->setConditionCode(of, OF, error);
        cc->setConditionCode(!(answer), ZF, error);
        cc->setConditionCode((Tools::sign(answer)), SF, error);
    }
    return answer;
}

uint64_t ExecuteStage::aluAfun(E * ereg)
{
    uint64_t icode = ereg->geticode()->getOutput();
    if(icode == IRRMOVQ || icode == IOPQ)
    {
        return ereg->getvalA()->getOutput();
    }
    else if(icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ)
    {
        return ereg->getvalC()->getOutput();
    }
    else if(icode == ICALL || icode == IPUSHQ)
    {
        return -8;
    }
    else if(icode == IRET || icode == IPOPQ)
    {
        return 8;
    }
    else
    {
        return 0;
    }
}
uint64_t ExecuteStage::aluBfun(E * ereg)
{
    uint64_t icode = ereg->geticode()->getOutput();
    if(icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || icode == ICALL ||
        icode == IPUSHQ || icode == IRET || icode == IPOPQ)
    {
        return ereg->getvalB()->getOutput();
    }
    else if(icode == IRRMOVQ || icode == IIRMOVQ)
    {
        return 0;
    }
    else 
    {
        return 0;
    }
}
uint64_t ExecuteStage::alufun(E * ereg)
{
    uint64_t icode = ereg->geticode()->getOutput();
    if(icode == IOPQ)
    {
        return ereg->getifun()->getOutput();
    }
    else
    {
        return ADDQ;
    }
}
bool ExecuteStage::set_cc(E * ereg, PipeReg ** pregs, Stage ** stages)
{
	W * wreg = (W *) pregs[WREG];
	uint64_t icode = ereg->geticode()->getOutput();
	MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
	uint64_t m_stat = mstage->getm_stat();
	uint64_t W_stat = wreg->getstat()->getOutput();
	
	
    return (icode == IOPQ) && !((m_stat == SADR) || (m_stat == SINS)
			|| (m_stat == SHLT)) && !((W_stat == SADR) || (W_stat == SINS)
			|| (W_stat == SHLT));
}
uint64_t ExecuteStage::makee_dstE(E * ereg, bool cnd)
{
    if(ereg->geticode()->getOutput() == IRRMOVQ && !(cnd))
    {
        return RNONE;
    }
    else
    {
        return ereg->getdstE()->getOutput();
    }
}
bool ExecuteStage::makeCnd(uint64_t icode, uint64_t ifun)
{
    bool error = false;
    ConditionCodes * cc = ConditionCodes::getInstance();
    if(icode != IJXX && icode != ICMOVXX)
    {
        return 0;
    }
    else if(ifun == 0)
    {
        return 1;
    }
    else if(ifun == 1)
    {
        bool sf = cc->getConditionCode(SF, error);
        bool of = cc->getConditionCode(OF, error);
        bool zf = cc->getConditionCode(ZF, error);
        return (sf ^ of) | zf;
    }
    else if(ifun == 2)
    {
        bool sf = cc->getConditionCode(SF, error);
        bool of = cc->getConditionCode(OF, error);
        return (sf ^ of);
    }
    else if(ifun == 3)
    {
        return cc->getConditionCode(ZF, error);
    }
    else if(ifun == 4)
    {
        return !(cc->getConditionCode(ZF, error));
    }
    else if(ifun == 5)
    {
        bool sf = cc->getConditionCode(SF, error);
        bool of = cc->getConditionCode(OF, error);
        return !(sf ^ of);
    }
    else if(ifun == 6)
    {
         bool sf = cc->getConditionCode(SF, error);
         bool of = cc->getConditionCode(OF, error);
         bool zf = cc->getConditionCode(ZF, error);
         return (!(sf ^ of) & !zf);
    }
    return 0;
}



void ExecuteStage::calculateControlSignals(PipeReg ** pregs, Stage ** stages)
{
	W * wreg = (W *) pregs[WREG];
	MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
	uint64_t m_stat = mstage->getm_stat();
	uint64_t W_stat = wreg->getstat()->getOutput();
	
	M_bubble = ((m_stat == SADR) || (m_stat == SINS) || (m_stat == SHLT)
		|| (W_stat == SADR) || (W_stat == SINS) || (W_stat == SHLT));	
}

#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "ConditionCodes.h"
#include "Stage.h"
#include "Instructions.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
uint64_t d_srcA;
uint64_t d_srcB;
bool E_bubble;
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    uint64_t stat = dreg->getstat()->getOutput();
    uint64_t icode = dreg->geticode()->getOutput();
    uint64_t ifun = dreg->getifun()->getOutput();

    uint64_t valC = dreg->getvalC()->getOutput();
    uint64_t dstE = builddstE(dreg, icode);
    uint64_t dstM = builddstM(dreg, icode);
    d_srcA = buildsrcA(dreg, icode);
    d_srcB = buildsrcB(dreg, icode);
    uint64_t valA = buildvalA(d_srcA, pregs, stages);
    uint64_t valB = buildvalB(d_srcB, pregs, stages);
    E_bubble = calculateControlSignals(ereg, stages);
    setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, d_srcA, d_srcB);
    return false;
}
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
    E * ereg = (E *)pregs[EREG];
    if(!E_bubble)
    {
        ereg->getstat()->normal();
        ereg->geticode()->normal();
        ereg->getifun()->normal();
        ereg->getvalC()->normal();
        ereg->getvalA()->normal();
        ereg->getvalB()->normal();
        ereg->getdstE()->normal();
        ereg->getdstM()->normal();
        ereg->getsrcA()->normal();
        ereg->getsrcB()->normal();
    }
    else //E_bubble true
    {
        ereg->getstat()->bubble(SAOK);
        ereg->geticode()->bubble(INOP);
        ereg->getifun()->bubble(0);
        ereg->getvalC()->bubble(0);
        ereg->getvalA()->bubble(0);
        ereg->getvalB()->bubble(0);
        ereg->getdstE()->bubble(RNONE);
        ereg->getdstM()->bubble(RNONE);
        ereg->getsrcA()->bubble(RNONE);
        ereg->getsrcB()->bubble(RNONE);
    }
    }
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC,
                            uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
    ereg->getstat()->setInput(stat);
    ereg->geticode()->setInput(icode);
    ereg->getifun()->setInput(ifun);
    ereg->getvalC()->setInput(valC);
    ereg->getvalA()->setInput(valA);
    ereg->getvalB()->setInput(valB);
    ereg->getdstE()->setInput(dstE);
    ereg->getdstM()->setInput(dstM);
    ereg->getsrcA()->setInput(srcA);
    ereg->getsrcB()->setInput(srcB);
}
uint64_t DecodeStage::buildvalA(uint64_t srcA, PipeReg ** preg, Stage ** stages)
{
   
    bool iregError = false;
    RegisterFile * reg = RegisterFile::getInstance();
    ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
    uint64_t e_dstE = estage->getDstE();
    uint64_t e_valE = estage->getValE();

    MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
    uint64_t m_valM = mstage->getValM();

    M * mreg = (M *) preg[MREG];
    W * wreg = (W *) preg[WREG];
    uint64_t valA = 0;
    //changed ifs
    D * dreg = (D *) preg[DREG];
    uint64_t icode = dreg->geticode()->getOutput();
    if(icode == IJXX || icode == ICALL)
    {
        valA = dreg->getvalP()->getOutput();
    } 
    else if(srcA == RNONE)//step II
    {
        return 0;
    }
    else if(srcA == e_dstE)
    {
        valA = e_valE;
    }
    else if(srcA == mreg->getdstM()->getOutput())
    {
        valA = m_valM;
    }
    else if(srcA == mreg->getdstE()->getOutput())
    {
        valA = mreg->getvalE()->getOutput();
    }
    else if(srcA == wreg->getdstM()->getOutput())
    {
        valA = wreg->getvalM()->getOutput();
    }
    else if(srcA == wreg->getdstE()->getOutput())
    {
        valA = wreg->getvalE()->getOutput();
    }
    else
    {
        valA = reg->readRegister(srcA,iregError);
    }
    return valA;

}
uint64_t DecodeStage::buildvalB(uint64_t srcB, PipeReg ** preg, Stage ** stages)
{
    if(srcB == RNONE)//step II
    {
        return 0;
    }
    bool iregError = false;
    RegisterFile * reg = RegisterFile::getInstance();
    ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
    uint64_t e_dstE = estage->getDstE();
    uint64_t e_valE = estage->getValE();
    M * mreg = (M *) preg[MREG];
    W * wreg = (W *) preg[WREG];
    uint64_t valB = 0;
    //changed

    MemoryStage * mstage = (MemoryStage *) stages[MSTAGE];
    uint64_t m_valM = mstage->getValM();

    if(srcB == e_dstE)
    {
      valB = e_valE;
    }
    else if(srcB == mreg->getdstM()->getOutput())
    {
      valB = m_valM;
    }
    else if(srcB == mreg->getdstE()->getOutput())
    {
        valB = mreg->getvalE()->getOutput();
    }
    else if(srcB == wreg->getdstM()->getOutput())
    {
        valB = wreg->getvalM()->getOutput();
    }
    else if(srcB == wreg->getdstE()->getOutput())
    {
        valB = wreg->getvalE()->getOutput();
    }
    else
    {
        valB = reg->readRegister(srcB,iregError);
    }
    return valB;

}
uint64_t DecodeStage::builddstE(D * dreg, uint64_t icode)
{
    if(icode == IRRMOVQ || icode == IIRMOVQ || icode == IOPQ)
    {
        return dreg->getrB()->getOutput();
    }
    if(icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}
uint64_t DecodeStage::builddstM(D * dreg, uint64_t icode)
{
    if(icode == IMRMOVQ || icode == IPOPQ)
    {
        return dreg->getrA()->getOutput();
    }
    else
    {
        return RNONE;
    }
}
uint64_t DecodeStage::buildsrcA(D * dreg, uint64_t icode)
{
    if(icode == IRRMOVQ || icode ==IRMMOVQ || icode ==IOPQ || icode == IPUSHQ)
    {
        return dreg->getrA()->getOutput();
    }
    else if(icode == IPOPQ || icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}
uint64_t DecodeStage::buildsrcB(D * dreg, uint64_t icode)
{
    if(icode == IOPQ || icode == IRMMOVQ || icode == IMRMOVQ)
    {
        return dreg->getrB()->getOutput();
    }
    else if(icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}
uint64_t DecodeStage::getsrcA()
{
    return d_srcA;
}
uint64_t DecodeStage::getsrcB()
{
    return d_srcB;
}
bool DecodeStage::calculateControlSignals(E * ereg, Stage ** stages)
{
    ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
    uint64_t cnd = estage->getCnd();
    
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t dstM = ereg->getdstM()->getOutput();
    
    return ((icode == IJXX && !cnd) || ((icode == IMRMOVQ || icode == IPOPQ) && (dstM == d_srcA || dstM == d_srcB)));
}

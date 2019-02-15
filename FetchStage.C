#include <string>
#include <iostream>
#include <cstdint>
#include "Tools.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Memory.h"
#include "ConditionCodes.h"
#include "Instructions.h"
#include "E.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "DecodeStage.h"
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
bool F_stall;
bool D_stall;
bool D_bubble;
/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.

   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(f_pc + 1);
   f_pc = selectPC(pregs);
   bool imemError = false;
   Memory * mem = Memory::getInstance();
   uint64_t curr = mem->getByte(f_pc, imemError);
   icode = curr >> 4;
   icode = memCheckicode(imemError, icode);
   ifun = curr % 16;
   ifun = memCheckifun(imemError, ifun);
   bool instr_valid = validInstruction(icode);
   stat = buildStat(imemError, instr_valid, icode);
   //provide the input values for the D register
   bool needregId = needRegIds(icode);
   bool needvalC = needValC(icode);
   valP = PCincrement(f_pc, needregId, needvalC);
   //uint64_t newPC= predictPC(icode, valC, valP);
   if(needregId)
   {
       getRegIds(f_pc, rA, rB);
   }
   if(needvalC)
   {
       valC = buildValC(f_pc, icode);
       //std::cout << valC << std::endl;
       //std::cout << "pineapple" << std::endl;
   }
   uint64_t newPC= predictPC(icode, valC, valP);
   calculateControlSignals(pregs, ereg, stages);
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   freg->getpredPC()->setInput(newPC);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   if(!F_stall)
   {
       freg->getpredPC()->normal();
   }
   if(!D_stall && !D_bubble)
   {
       dreg->getstat()->normal();
       dreg->geticode()->normal();
       dreg->getifun()->normal();
       dreg->getrA()->normal();
       dreg->getrB()->normal();
       dreg->getvalC()->normal();
       dreg->getvalP()->normal();
   }
   if(D_bubble)
   {
       updateBubble(dreg);
   }
}

void FetchStage::updateBubble(D * dreg)
{
    dreg->getstat()->bubble(SAOK);
    dreg->geticode()->bubble(INOP);
    dreg->getifun()->bubble(0);
    dreg->getrA()->bubble(RNONE);
    dreg->getrB()->bubble(RNONE);
    dreg->getvalC()->bubble(0);
    dreg->getvalP()->bubble(0);
}
/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode,
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}
bool FetchStage::validInstruction(uint64_t icode)
{
    if(icode == INOP || icode == IHALT || icode == IRRMOVQ || icode == IIRMOVQ || icode == IRMMOVQ ||
        icode == IMRMOVQ || icode == IOPQ || icode == IJXX || icode == ICALL || icode == IRET || icode == IPUSHQ ||
        icode == IPOPQ)
    {
        return true;
    }
    return false;
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needRegIds, bool needvalC)
{
    int add = 1;
    if(needRegIds == true)
    {
       add += 1;
    }
    if(needvalC == true)
    {
        add += 8;
    }
    return f_pc +add;
}
int FetchStage::selectPC(PipeReg ** pregs)
{

    F * freg = (F *) pregs[FREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    uint64_t predPc = freg->getpredPC()->getOutput();
    if((mreg->geticode()->getOutput() == IJXX)&& !(mreg->getCnd()->getOutput()))
    {
        predPc = mreg->getvalA()->getOutput();
    }
    else if(wreg->geticode()->getOutput() == IRET)
    {
        predPc = wreg->getvalM()->getOutput();
    }
    return predPc;

}
uint64_t FetchStage::memCheckicode(bool error, uint64_t icode)
{
    if(error)
    {
        return INOP;
    }
    else
    {
        return icode;
    }
}
uint64_t FetchStage::memCheckifun(bool error, uint64_t ifun)
{
    if(error)
    {
        return FNONE;
    }
    else
    {
        return ifun;
    }
}

bool FetchStage::needRegIds(uint64_t ficode)
{
    bool need_regids = false;
    if(ficode == IRRMOVQ || ficode == IOPQ || ficode == IPUSHQ)
    {
        need_regids = true;
    }
    else if(ficode == IPOPQ || ficode == IIRMOVQ || ficode == IRMMOVQ || ficode == IMRMOVQ)
    {
        need_regids = true;
    }
    return need_regids;
}
uint64_t FetchStage::buildStat(bool imemError, bool instr_valid, uint64_t icode)
{
    if(imemError)
    {
        return SADR;
    }
    if(!instr_valid)
    {
        return SINS;
    }
    if(icode == IHALT)
    {
        return SHLT;
    }
    else
    {
        return SAOK;
    }
}

bool FetchStage::needValC(uint64_t ficode)
{
    bool need_valC = false;
    if(ficode == IJXX || ficode == ICALL)
    {
        need_valC = true;
    }
    else if(ficode == IIRMOVQ || ficode == IRMMOVQ || ficode == IMRMOVQ)
    {
       need_valC = true;
    }
    return need_valC;
}
int FetchStage::predictPC(uint64_t ficode, uint64_t fvalC, uint64_t fvalP)
{

    uint64_t predPC = fvalP;

    if(ficode == IJXX || ficode == ICALL)
    {
        predPC = fvalC;
    }
    return predPC;
}
void FetchStage::getRegIds(uint64_t f_pc, uint64_t & rA, uint64_t & rB)
{
    bool imemError = false;
    Memory * mem = Memory::getInstance();
    uint64_t instruct = mem->getByte(f_pc+1, imemError);
    rB = instruct % 16;
    rA = instruct / 16;

}
uint64_t FetchStage::buildValC(uint64_t f_pc, uint64_t icode)
{
    bool imemError = false;
    Memory * mem = Memory::getInstance();
    uint8_t valC[8];
    int offset = 2;
    if(icode == IJXX || icode == ICALL)
    {
        offset = 1;
    }
    for(int i = offset; i < 10; i++)
    {
        valC[i-offset] = mem->getByte(f_pc + i, imemError);
    }
    return Tools::buildLong(valC);

}

void FetchStage::stallF(PipeReg ** pregs, E * ereg, Stage ** stages)
{
    //ExecuteStage * estage =(ExecuteStage *) stages[ESTAGE];
    DecodeStage * dstage =(DecodeStage *) stages[DSTAGE];

    D * dreg = (D *) pregs[DREG];
    M * mreg = (M *) pregs[MREG];

    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t icode = ereg->geticode()->getOutput(); //E_icode
    uint64_t M_icode = mreg->geticode()->getOutput();

    uint64_t dstM = ereg->getdstM()->getOutput();
    uint64_t d_srcA = dstage->getsrcA();
    uint64_t d_srcB = dstage->getsrcB();
    F_stall = ((icode == IMRMOVQ || icode == IPOPQ) && (dstM == d_srcA || dstM == d_srcB)) || (D_icode == IRET || icode == IRET || M_icode == IRET);
}
void FetchStage::stallD(E * ereg, Stage ** stages)
{
    DecodeStage * dstage = (DecodeStage *) stages[DSTAGE];
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t dstM = ereg->getdstM()->getOutput();
    uint64_t d_srcA = dstage->getsrcA();
    uint64_t d_srcB = dstage->getsrcB();
    D_stall = ((icode == IMRMOVQ || icode == IPOPQ) && (dstM == d_srcA || dstM == d_srcB));
}

void FetchStage::bubbleD(PipeReg ** pregs, E * ereg, Stage ** stages)
{
    D * dreg = (D *) pregs[DREG];
    M * mreg = (M *) pregs[MREG];
    DecodeStage * dstage = (DecodeStage *) stages[DSTAGE];
    ExecuteStage * estage = (ExecuteStage *) stages[ESTAGE];
    uint64_t D_icode = dreg->geticode()->getOutput();
    uint64_t icode = ereg->geticode()->getOutput(); //E_icode
    uint64_t M_icode = mreg->geticode()->getOutput();
    uint64_t dstM = ereg->getdstM()->getOutput();

    uint64_t d_srcA = dstage->getsrcA();
    uint64_t d_srcB = dstage->getsrcB();

    bool e_cnd = estage->getCnd();
    D_bubble = ((icode == IJXX && !e_cnd) ||( !((icode == IMRMOVQ || icode == IPOPQ) && (dstM == d_srcA || dstM == d_srcB)) && (D_icode == IRET || icode == IRET || M_icode == IRET)));
}
void FetchStage::calculateControlSignals(PipeReg ** pregs, E * ereg, Stage ** stages)
{
    stallF(pregs, ereg, stages);
    stallD(ereg, stages);
    bubbleD(pregs, ereg, stages);
}

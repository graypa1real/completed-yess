#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "Instructions.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"

bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    W * wreg = (W *) pregs[WREG];
    //uint64_t icode = wreg->geticode()->getOutput(); 
    if(wreg->getstat()->getOutput() != SAOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];
    uint64_t dstE = wreg->getdstE()->getOutput();
    RegisterFile * reg = RegisterFile::getInstance();
    bool iregError = false;
    reg->writeRegister(wreg->getvalE()->getOutput(), dstE, iregError);   
    uint64_t dstM = wreg->getdstM()->getOutput();
    uint64_t valM = wreg->getvalM()->getOutput();
    writeReg(dstM, valM);
}
void WritebackStage::writeReg(uint64_t dstM, uint64_t valM)
{
    bool error = false;
    RegisterFile * reg = RegisterFile::getInstance();
    reg->writeRegister(valM, dstM, error);
}

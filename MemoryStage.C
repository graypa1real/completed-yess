#include <string>
#include <cstdint>
#include "Instructions.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Memory.h"
#include "Debug.h"
uint64_t valM;
uint64_t stat;

bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG]; 
	
	stat = mreg->getstat()->getOutput(); //sets to M_stat
	
    uint64_t icode = mreg->geticode()->getOutput();
    uint64_t valE = mreg->getvalE()->getOutput();
    uint64_t valA = mreg->getvalA()->getOutput();
    valM = 0;  
    uint64_t dstE = mreg->getdstE()->getOutput();
    uint64_t dstM = mreg->getdstM()->getOutput();
    uint64_t memAddr = makeAddr(mreg);
    bool mem_read = memRead(icode);
    bool mem_write = memWrite(icode);
    bool mem_error = accessMemory(memAddr, mem_read, mem_write, valA);
	
	if(mem_error) //mem_error : SADR
	{
		stat = SADR; 
	}
	
    setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
    return false;
}
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
    
    W * wreg = (W *) pregs[WREG];
    wreg->getstat()->normal();
    wreg->geticode()->normal(); 
    wreg->getvalE()->normal();
    wreg->getvalM()->normal();
    wreg->getdstE()->normal();
    wreg->getdstM()->normal();        
}
void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE, 
                             uint64_t valM, uint64_t dstE, uint64_t dstM)
{
    wreg->getstat()->setInput(stat);
    wreg->geticode()->setInput(icode);
    wreg->getvalE()->setInput(valE);
    wreg->getvalM()->setInput(valM);
    wreg->getdstE()->setInput(dstE);
    wreg->getdstM()->setInput(dstM);
}
bool MemoryStage::accessMemory(uint64_t memAddr, bool read, bool write, uint64_t valA)
{
    Memory * mem = Memory::getInstance();
    bool error = false;
    if(read)
    {
        valM = mem->getLong(memAddr, error);
    }
    else if(write)
    {
        mem->putLong(valA, memAddr, error);
    }
	
	return error;
}
bool MemoryStage::memRead(uint64_t icode)
{
    return (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
}
bool MemoryStage::memWrite(uint64_t icode)
{
    return (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
}
uint64_t MemoryStage::makeAddr(M * mreg)
{
    uint64_t memAddr = 0;
    uint64_t icode = mreg->geticode()->getOutput();
    if(icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL || icode == IMRMOVQ)
    {
        memAddr = mreg->getvalE()->getOutput();
    }
    else if(icode == IPOPQ || icode == IRET)
    {
        memAddr = mreg->getvalA()->getOutput();
    }
    else
    {
        memAddr = 0;
    }
    return memAddr;
}

uint64_t MemoryStage::getm_stat()
{
	return stat;
}

uint64_t MemoryStage::getValM()
{
        return valM;
}

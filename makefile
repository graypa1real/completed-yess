CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ = yess.o Memory.o Tools.o RegisterFile.o Simulate.o \
ConditionCodes.o D.o E.o F.o M.o W.o Loader.o  PipeReg.o PipeRegField.o \
FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o
NEEDH = RegisterFile.h PipeRegField.h PipeReg.h Status.h Tools.h\
RegisterFile.C PipeRegField.C PipeReg.C Tools.C
EXE = ExecuteStage.C ExecuteStage.h
MEM = MemoryStage.C MemoryStage.h
FET = FetchStage.C FetchStage.h
WBS = WritebackStage.C WritebackStage.h
EXTH = F.h D.h M.h W.h Stage.h FetchStage.h F.C D.C M.C W.C FetchStage.C


.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)
	$(CC) $(OBJ) -o yess
yess.o: yess.C Memory.h ConditionCodes.h Loader.h $(NEEDH) $(NEEDC)
ConditionCodes.o: Tools.h Tools.C ConditionCodes.C ConditionCodes.h
Memory.o: Tools.h Tools.C Memory.h
RegisterFile.o: Tools.h Tools.C
Loader.o: Loader.C Loader.h Memory.h
D.o: $(NEEDH) D.C D.h Instructions.h  
E.o: $(NEEDH) E.C E.h Instructions.h
F.o: $(NEEDH) F.C F.h Instructions.h
M.o: $(NEEDH) M.C M.h Instructions.h
W.o: $(NEEDH) W.C W.h Instructions.h
ExecuteStage.o: $(NEEDH) $(EXTH) ExecuteStage.C ExecuteStage.h  ConditionCodes.C ConditionCodes.h
DecodeStage.o: $(NEEDH) $(EXTH) E.h E.C DecodeStage.C DecodeStage.h Memory.C Memory.h ExecuteStage.h ExecuteStage.C
FetchStage.o: $(NEEDH) $(EXTH) FetchStage.C FetchStage.h

MemoryStage.o: $(NEEDH) $(EXTH) MemoryStage.C MemoryStage.h ExecuteStage.C ExecuteStage.h
WritebackStage.o: $(NEEDH) $(EXTH) WritebackStage.C WritebackStage.h
Simulate.o: PipeRegField.h PipeRegField.C PipeReg.h PipeReg.C $(EXTH) E.h E.C $(EXE) $( MEM) $(WBS) Memory.h Memory.C ConditionCodes.C ConditionCodes.h RegisterFile.C RegisterFile.h Simulate.h



clean:
	rm $(OBJ)

run:
	make clean
	make yess
	./run.sh


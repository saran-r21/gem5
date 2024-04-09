#include "cpu/pred/gselect.hh"
#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/GSDebug.hh"

using namespace std;
/*
Here we assign all the values we use in the code. The values are obtained from the class present in the BranchPredictor.py code.
*/
GSelectBP::GSelectBP(const GSelectBPParams &params)
    : BPredUnit(params),
      globalHistoryBits(params.globalHistoryBits),
      globalHistoryReg(params.globalHistoryBits, 0),
      PredictorSize(params.PredictorSize),
      globalHistoryMask(params.PredictorSize-1),
      PHTCtrBits(params.PHTCtrBits),
      BranchMask(params.PredictorSize-1),
      lclCounters(params.PredictorSize, SatCounter8(params.PHTCtrBits))
{
   DPRINTF(GSDebug, "Shift amt: %i\n", instShiftAmt);
   DPRINTF(GSDebug, "Global bits: %u\n", globalHistoryBits);   
   DPRINTF(GSDebug, "Size: %u\n", PredictorSize ); 
   DPRINTF(GSDebug, "PHTCtr bits: %u\n", PHTCtrBits); 
   DPRINTF(GSDebug, "Global mask: %u\n", globalHistoryMask);
   DPRINTF(GSDebug, "Branch mask: %u\n", BranchMask);      
}


void
GSelectBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
    DPRINTF(GSDebug, "In uncondBranch. Global history register is: %d. Branch address: %#x\n", globalHistoryReg[tid],pc);
}

void
GSelectBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;
    delete history;
    DPRINTF(GSDebug, "In squash. Global history register is: %d\n", globalHistoryReg[tid]);
}
/*
In lookup function we first get the values of tid and branch address.
We then call the NOROperation function which performs the operation on the history register and the branch address (which is shifted by 2 bits) and is assigned to PHTIndex. We have a variable countervalue which will be given the value of the counter at the index PHTIndex. Based on the countervalue we call the getPrediction function and get the Prediction to be 0 or 1. Based on the prediction we update the register and print the value of the Register and the Branch Address.
*/
bool
GSelectBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    DPRINTF(GSDebug, "In lookup. Global History Register was: %d Branch address: %#x\n", globalHistoryReg[tid], branchAddr);
    bool Prediction;
    unsigned PHTIndex = NOROperation(branchAddr >> instShiftAmt, globalHistoryReg[tid]);                       
    uint8_t countervalue = lclCounters[PHTIndex];
    Prediction= getPrediction(countervalue);
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    bpHistory = static_cast<void*>(history);
    DPRINTF(GSDebug,"In lookup. PHTIdx:%d\n",PHTIndex);
    updateGlobalHistReg(tid, Prediction);
    DPRINTF(GSDebug, "In lookup. Global History Register is: %d Branch address: %#x\n", globalHistoryReg[tid], branchAddr);    
    DPRINTF(GSDebug, "In lookup. mm is: %d\n",(branchAddr >> instShiftAmt) & globalHistoryMask);
    return Prediction;
}

void
GSelectBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg[tid] &= (globalHistoryMask & ~ULL(1));
}
/*
In the update function we first get the values of tid and branch address.
We then call the NOROperation function which performs the operation on the history register and the branch address (which is shifted by 2 bits) and is assigned to PHTIndex. Then based on the boolean value of taken(0/1) the counter values are updated. We then print the value of the Global history register and the Branch Address.
*/
void
GSelectBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed, const StaticInstPtr & inst, Addr corrTarget)
{
   
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    assert(bpHistory);
    if (squashed) {
    	DPRINTF(GSDebug, "In update. Squashed. Global history register was: %d. Branch address: %#x \n", globalHistoryReg[tid], branchAddr);
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        globalHistoryReg[tid] &= globalHistoryMask;
        return;
    }

    unsigned PHTIndex = NOROperation(branchAddr >> instShiftAmt, history->globalHistoryReg);

        if (taken) {
            lclCounters[PHTIndex]++;
        } else {
            lclCounters[PHTIndex]--;
        }
    DPRINTF(GSDebug,"In update. PHTIdx:%d\n",PHTIndex);    
    DPRINTF(GSDebug, "In update. Global history register was: %d. Branch address: %#x \n", globalHistoryReg[tid], branchAddr);
    DPRINTF(GSDebug, "In update. mm is: %d\n",(branchAddr >> instShiftAmt) & globalHistoryMask);
    delete history;
}

void
GSelectBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    // This function decides the value of the register based on the value 
    // of the boolean variable taken.
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= globalHistoryMask;
}


inline
bool
GSelectBP::getPrediction(uint8_t &count)
{
    // Get the MSB of the count
    return (count >> (PHTCtrBits - 1));
}

/*NOR operation is performed in this function.
This function implements the NOR operation bit by bit. 
The MSB of both the Address and the History is extracted in the for loop.
The result of both these bits after the NOR operation is stored in the 
norResult variable.
*/
unsigned 
GSelectBP::NOROperation(unsigned Address, unsigned GB_History)
{
  unsigned result = 0;
  if(Address ==0 && GB_History==0)
  	return 1;
  if(Address < GB_History)
  	swap(Address,GB_History);

    for (int i = 7; i >= 0; i--) {
        unsigned bitAddress = (Address >> i) & 1;
        unsigned bitHistory = (GB_History >> i) & 1;
        unsigned norResult = !(bitAddress | bitHistory);
        result = (result << 1) | norResult;
    }

    return result & globalHistoryMask;
}



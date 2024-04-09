#ifndef __CPU_PRED_GSELECT_6_PRED_HH__
#define __CPU_PRED_GSELECT_6_PRED_HH__

#include "base/sat_counter.hh"
#include "cpu/pred/bpred_unit.hh"
#include "params/GSelectBP.hh"
#include <vector>

class GSelectBP : public BPredUnit
{
  public:
    GSelectBP(const GSelectBPParams &params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    void squash(ThreadID tid, void *bp_history);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,bool squashed, const StaticInstPtr & inst, Addr corrTarget);
    unsigned NOROperation(unsigned Address, unsigned GB_History);
  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);
    inline bool getPrediction(uint8_t &count);
    struct BPHistory {
        unsigned globalHistoryReg;
    };

    unsigned globalHistoryBits;
    std::vector<unsigned> globalHistoryReg;
    unsigned historyRegisterMask;
    unsigned PredictorSize;
    unsigned globalHistoryMask;
    unsigned PHTCtrBits;
    unsigned BranchMask;
    std::vector<SatCounter8> lclCounters;
    
};

#endif // __CPU_PRED_GSELECT_6_PRED_HH__

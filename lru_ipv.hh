#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__

#include "mem/cache/replacement_policies/base.hh"

// Forward declaration for LRUIPVRPParams
struct LRUIPVRPParams;
namespace ReplacementPolicy {

// Class declaration for LRUIPVRP, inheriting from Base class
class LRUIPVRP : public Base {
  protected:
    const int numWays;           // Number of cache ways
    int k;                        // Counter for instantiation
    typedef std::vector<int> LRUIPVvector;  // Type definition for vector
    LRUIPVvector *vectInstance;    // Instance of the vector

    // Struct for replacement data, inheriting from ReplacementData
    struct LRUReplData : ReplacementData {
        Tick lastTouchTick;         // Timestamp of the last touch
        mutable int index;          // Mutable index for replacement
        std::shared_ptr<LRUIPVvector> Vector;  // Shared pointer to the vector
        LRUReplData(int assoc, std::shared_ptr<LRUIPVvector> Vector);
    };

    // Initialization of the InsertionVector with predefined values
    const std::vector<int> InsertionVector{0, 0, 1, 0, 2, 0, 1, 2, 1, 0, 6, 1, 0, 0, 1, 11, 12};

  public:
    // Type definition for LRUIPVRPParams
    typedef LRUIPVRPParams Params;
    LRUIPVRP(const Params &p);  // Constructor for LRUIPVRP
    ~LRUIPVRP() {}              // Destructor

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Sets its last touch tick as the starting tick.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const override;

    /**
     * Touch an entry to update its replacement data.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Sets its last touch tick as the current tick.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const override;

    /**
     * Find replacement victim using LRU timestamps.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

}  // namespace ReplacementPolicy

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__


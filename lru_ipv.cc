/**
 * Copyright (c) 2018-2020 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/lru_ipv.hh"

#include <cassert>
#include <memory>

#include "params/LRUIPVRP.hh"
#include "sim/core.hh"

namespace ReplacementPolicy {

// Constructor for the LRUIPVRP class
LRUIPVRP::LRUIPVRP(const Params &p)
  : Base(p), k(0), vectInstance(nullptr), numWays(p.numWays)
{
}

// Constructor for the LRUReplData class
LRUIPVRP::LRUReplData::LRUReplData(int index, std::shared_ptr<LRUIPVvector> Vector)
  : index(index), Vector(Vector)
{
}

// Implementation of the invalidate method
void LRUIPVRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // No action needed for invalidation in this policy
    return;
}

// Implementation of the touch method
void LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Cast replacement_data to LRUReplData
    std::shared_ptr<LRUReplData> Data = std::static_pointer_cast<LRUReplData>(replacement_data);
    // Get the vector from replacement_data
    LRUIPVvector* vect = Data->Vector.get();

    // Get the values for updating the vector
    int New = InsertionVector[Data->index];
    int Old = vect->at(Data->index);
    int IPV_Index = 0;

    // Update the vector based on the touch operation
    while (IPV_Index < vect->size()) {
        if ((vect->at(IPV_Index) >= New) && (vect->at(IPV_Index) < Old)) {
            vect->at(IPV_Index) = vect->at(IPV_Index) + 1;
        }
        IPV_Index++;
    }
    vect->at(Data->index) = New;
}

// Implementation of the reset method
void LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Cast replacement_data to LRUReplData
    std::shared_ptr<LRUReplData> Data = std::static_pointer_cast<LRUReplData>(replacement_data);
    // Get the vector from replacement_data
    LRUIPVvector* vect = Data->Vector.get();

    int IPV_Index = 0;

    // Reset the vector when a new entry is inserted
    while (IPV_Index < vect->size()) {
        if ((vect->at(IPV_Index) >= InsertionVector[numWays]) && (vect->at(IPV_Index) < numWays)) {
            vect->at(IPV_Index) = vect->at(IPV_Index) + 1;
        }
        IPV_Index++;
    }

    vect->at(Data->index) = InsertionVector[numWays];
}

// Implementation of the getVictim method
ReplaceableEntry* LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const
{
    // Ensure that there is at least one candidate
    assert(candidates.size() > 0);
    // Cast replacement_data of the first candidate to LRUReplData
    std::shared_ptr<LRUReplData> Data = std::static_pointer_cast<LRUReplData>(candidates[0]->replacementData);
    // Get the vector from replacement_data
    LRUIPVvector* vect = Data->Vector.get();
    int IPV_Index = 0;
    int Maximum = 0;
    int Insert = 0;

    // Find the entry with the maximum value in the vector
    while (IPV_Index < vect->size()) {
        if (Maximum < vect->at(IPV_Index)) {
            Maximum = vect->at(IPV_Index);
            Insert = IPV_Index;
        }
        IPV_Index = IPV_Index + 1;
    }

    // Return the candidate with the maximum value in its replacement data
    return candidates[Insert];
}

// Implementation of the instantiateEntry method
std::shared_ptr<ReplacementData> LRUIPVRP::instantiateEntry()
{
    // Check if a new vector instance needs to be created
    if (k % numWays == 0) {
        vectInstance = new LRUIPVvector(numWays, numWays);
    }

    // Create a new LRUReplData instance and update the counter
    LRUReplData* Data = new LRUReplData(k % numWays, std::shared_ptr<LRUIPVvector>(vectInstance));
    k++;
    return std::shared_ptr<ReplacementData>(Data);
}

} // namespace ReplacementPolicy


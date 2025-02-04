//===- llvm/ADT/SmallVector.cpp - 'Normally small' vectors ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SmallVector class.
//
//===----------------------------------------------------------------------===//

#include "wpi/SmallVector.h"
#include "wpi/MemAlloc.h"

using namespace wpi;

/// grow_pod - This is an implementation of the grow() method which only works
/// on POD-like datatypes and is out of line to reduce code duplication.
void SmallVectorBase::grow_pod(void *FirstEl, size_t MinCapacity,
                               size_t TSize) {
    // Ensure we can fit the new capacity in 32 bits.
    if (MinCapacity > UINT32_MAX)
        report_bad_alloc_error("SmallVector capacity overflow during allocation");

    size_t NewCapacity = 2 * capacity() + 1; // Always grow.
    NewCapacity =
            std::min(std::max(NewCapacity, MinCapacity), size_t(UINT32_MAX));

    void *NewElts;
    if (BeginX == FirstEl) {
        NewElts = safe_malloc(NewCapacity * TSize);

        // Copy the elements over.  No need to run dtors on PODs.
        memcpy(NewElts, this->BeginX, size() * TSize);
    } else {
        // If this wasn't grown from the inline copy, grow the allocated space.
        NewElts = safe_realloc(this->BeginX, NewCapacity * TSize);
    }

    this->BeginX = NewElts;
    this->Capacity = NewCapacity;
}

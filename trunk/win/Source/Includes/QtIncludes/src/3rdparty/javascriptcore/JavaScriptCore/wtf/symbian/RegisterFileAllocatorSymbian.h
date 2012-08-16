/*
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RegisterFileAllocatorSymbian_h
#define RegisterFileAllocatorSymbian_h

#include "SymbianDefines.h"

namespace WTF {

/**
 *  Allocates contiguous regions of size poolSize.
 *  poolSize must be a multiple of system page size (typically 4K on Symbian/ARM)
 *
 *  @param reservationSize Virtual address range to be reserved upon creation of chunk (bytes).
 *  @param poolSize Size of a single allocation.
 */
class RegisterFileAllocator {

public:
    RegisterFileAllocator(
            TUint32 reservationSize, TUint32 poolSize = SYMBIAN_REGFILEALLOC_DEFAULTPOOLSIZE);
    ~RegisterFileAllocator();
    void* buffer() const;
    void grow(void* newEnd);
    void shrink(void* newEnd);

private:
    RChunk   m_chunk; // Symbian chunk that lets us reserve/commit/decommit

    // all following values are in numbers of bytes
    TInt     m_pageSize; // cached value of system page size, typically 4K on Symbian
    TUint32  m_reserved; // total number of reserved bytes in virtual memory
    TUint32  m_poolSize; // size of one memory pool, set by default to 64K in wtf/symbian/SymbianDefines.h

    void* m_buffer; // pointer to base of the chunk
    void* m_comEnd; // pointer to end of currently committed memory
    void* m_resEnd; // pointer to end of reserved memory

};

} // end of namespace

#endif // RegisterFileAllocatorSymbian_h

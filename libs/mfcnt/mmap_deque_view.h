/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_VIEW_H
#define _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_VIEW_H

#include "mfcnt/types.h"
#include "mfcnt/details/mmap_base_container.h"

namespace mfcnt {

template<typename TTp, size_t TCount = 4*1024*1024>
class mmap_deque_view : protected details::mmap_base_container<TTp, sizeof(TTp)*TCount>
{
    typedef details::mmap_base_container<TTp, sizeof(TTp)*TCount>   base;

public:
    typedef TTp                                     value_type;
    typedef typename base::const_pointer            pointer;
    typedef typename base::const_pointer            const_pointer;
    typedef typename base::const_reference          reference;
    typedef typename base::const_reference          const_reference;
//    typedef typename base::const_iterator           iterator;
//    typedef typename base::const_iterator           const_iterator;
//    typedef std::reverse_iterator<const_iterator>   reverse_iterator;
//    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;

    mmap_deque_view()
        : base()
    {}

    mmap_deque_view(const char* file_path, size_t size, off64_t offset, mode m = mode::R_ONLY)
        : base(file_path, size, offset, m)
    {}

    mmap_deque_view(const std::string& file_path, size_t size, off64_t offset, mode m = mode::R_ONLY)
        : base(file_path, size, offset, m)
    {}

    mmap_deque_view(const char* file_path, off64_t offset = 0, mode m = mode::R_ONLY)
        : base(std::string(file_path), offset, mode::R_ONLY)
    {}

    mmap_deque_view(const std::string& file_path, off64_t offset = 0, mode m = mode::R_ONLY)
        : base(file_path, offset, mode::R_ONLY)
    {}

//    mmap_deque_view(const mmap_deque_view& orig)
//        : base(orig)
//    {}

//    mmap_deque_view(mmap_deque_view&& orig)
//        : Base(std::move(orig))
//    {}

    virtual ~mmap_deque_view() {}

    bool empty() const { return (size() == 0); }

    size_type size() const { return base::m_size; }
};

} // namespace mfcnt

#endif /* _MMAP_CONTAINERS_MFCNT_MMAP_DEQUE_VIEW_H */


//
//  sparseMatrix.hpp
//  Graphs
//
//  Created by Oleg Bakharev on 05/02/2018.
//  Copyright © 2018 Oleg Bakharev. All rights reserved.
//

#ifndef sparseMatrix_hpp
#define sparseMatrix_hpp

#include <iostream>
#include "sparseArray.hpp"
#include "matrix.h"
#include <algorithm>

// Специализация for_iter_t для итерации по элементам разреженной матрицы
template <typename T, typename Context, typename Enable> class for_iter_t <
basic_slice_iter<T, SparseArray<T>, Context>, Context, Enable> {
    using slice_iter = basic_slice_iter<T, SparseArray<T>, Context>;
    using container = SparseArray<T>;
    using iterator = typename container::iterator;
    using value_type = typename container::value_type;
    using reference = value_type&;//typename container::reference;
    slice_iter& iter;
    iterator pos; // = ближайшее к началу [start(), start() + size() * stride()); O(log(N))
public:
    for_iter_t(const slice_iter& iter) : iter(const_cast<slice_iter&>(iter)),
    pos(std::lower_bound(iter.v.begin(), iter.v.end(), value_type(iter.lower_index(), T()))) {
        
    }
    reference operator*() { return *pos; }
    bool operator != (const for_iter_t& end) const {
        if (!(pos != iter.v.end())) return false; // Проверка переполнения разреженного массива на последнем элементе.
        size_t last = end.iter.upper_index();
        size_t first = pos->first;
        return first < last; }
    void operator++() { ++pos; }
};

template <typename T, typename Context = void>
using SparseMatrix = basic_matrix<T, SparseArray<T>, Context>;

void testSparseMatrix();

#endif /* sparseMatrix_hpp */

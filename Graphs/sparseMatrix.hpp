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

template <typename T, typename Context = void>
using SparseMatrix = basic_matrix<T, SparseArray<T>, Context>;

void testSparseMatrix();

#endif /* sparseMatrix_hpp */

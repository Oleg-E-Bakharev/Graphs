//
//  sparseMatrix.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 05/02/2018.
//  Copyright © 2018 Oleg Bakharev. All rights reserved.
//

#include "sparseMatrix.hpp"

void testSparseMatrix()
{
    SparseMatrix<int> m{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}};

    cout << m;
    
    m[1][1] = 10;
    
    cout << m;
//    auto m1 = m.tr();
//    cout << m1;
}

//
//  sparseMatrix.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 05/02/2018.
//  Copyright © 2018 Oleg Bakharev. All rights reserved.
//

#include "sparseMatrix.hpp"
#include <random>

using namespace std;

void testSparseMatrix()
{
    SparseMatrix<int> m{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}};

    cout << m;
    
    m[1][1] = 10;
    
    cout << m;
    
    SparseMatrix<int> sm(20, 20);
    default_random_engine gen(random_device{}());
    uniform_int_distribution<> dis(0, 20);
    for(int i = 0; i < 30; i++) {
        size_t j = dis(gen);
        size_t k = dis(gen);
        if (i != j) {
            sm[j][k] = 1;
        }
    }
    cout << sm;
}

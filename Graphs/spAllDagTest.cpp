//
//  spAllDagTest.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 31/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#include "graphGen.h"
#include "weightedGraph.h"
#include "spAllDag.h"

using namespace std;
using namespace Graph;

template <class G> void spAllBuildDag(G& g) {
    // Рис 21.15 Седжвик.
    insertEdges(g, {
        {0, 1, .41},
        {0, 7, .41},
        {0, 9, .41},
        {1, 2, .51},
        {6, 3, .21},
        {6, 8, .21},
        {7, 3, .32},
        {7, 8, .32},
        {8, 2, .32},
        {9, 4, .29},
        {9, 6, .29}
    });
}

template <class G> void testSpAllDag(const G& g)
{
    cout << spAllDagMax(g) << std::endl;
    cout << spAllDagMin(g);
}

static void testDenseDag() {
    cout << "Dense weighted dir graph:\n";
    DenseGraphWD g(10);
    spAllBuildDag(g);
    testSpAllDag(g);
}

static void testSparseDag() {
    cout << "\nSparse weighted dir graph:\n";
    SparseGraphWD g(10);
    spAllBuildDag(g);
    testSpAllDag(g);
}

void spAllDagTest()
{
    testDenseDag();
    testSparseDag();
}

//
//  testSPAll.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 23/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#include <iostream>

#include "graphGen.h"
#include "weightedGraph.h"
#include "spAll.h"

using namespace std;
using namespace Graph;

template <class G> void spAllBuildGraph(G& g) {
    // Рис 21.1 Седжвик.
    insertEdges(g, {
        {0, 1, .41},
        {1, 2, .51},
        {2, 3, .50},
        {4, 3, .36},
        {3, 5, .38},
        {3, 0, .45},
        {0, 5, .29},
        {5, 4, .21},
        {1, 4, .32},
        {4, 2, .32},
        {5, 1, .29}
    });
}

template <class G> void spAllBuildGraphNegative(G& g) {
    // Рис 21.26 Седжвик.
    insertEdges(g, {
        {0, 1, .41},
        {1, 2, .51},
        {2, 3, .50},
        {4, 3, .36},
        {3, 5, -.38},
        {3, 0, .45},
        {0, 5, .29},
        {5, 4, .21},
        {1, 4, .32},
        {4, 2, .32},
        {5, 1, -.29}
    });
}

template <class G> void testSpAll(G& g)
{
//    auto spad = spAllDijkstra(g);
//    cout << spad;
//    cout << "Path 0-2 distance: " << spad.distance(0, 2) << endl;
//    for (const auto& edge : spad.path(0, 2)) {
//        cout << edge << endl;
//    }
//    cout << diameter(g, spad);
	
    auto spaf = spAllFloyd(g);
    cout << spaf;
    
    cout << "Path 0-2 distance: " << spaf.distance(0, 2) << endl;
    for (const auto& edge : spaf.path(0, 2)) {
        cout << edge << endl;
    }
//    cout << diameter(g, spaf);
	
	auto spaj = spAllJohnson(g);
	cout << spaj;
	
	cout << "Path 0-2 distance: " << spaj.distance(0, 2) << endl;
	for (const auto& edge : spaj.path(0, 2)) {
		cout << edge << endl;
	}
//	cout << diameter(g, spaj);

}

static void testDenseSpAll() {
    cout << "Dense weighted dir graph:\n";
    DenseGraphWD g(6);
//    spAllBuildGraph(g);
	spAllBuildGraphNegative(g);
    testSpAll(g);
}

static void testSparseSpAll() {
    cout << "\nSparse weighted dir graph:\n";
    SparseGraphWD g(6);
	spAllBuildGraphNegative(g);
//    spAllBuildGraph(g);
    testSpAll(g);
}

void spAllTest()
{
    testDenseSpAll();
    testSparseSpAll();
}

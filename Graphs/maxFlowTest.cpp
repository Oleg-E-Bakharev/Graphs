//
//  maxFlow.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 10.04.17.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#include "maxFlow.h"
#include "weightedGraph.h"
#include "graphGen.h"
using namespace std;
using namespace Graph;

void maxFlowTest()
{
	SparseGraph_T<WeightedGraphTraits<int>> net(6);
	insertEdges(net, {
		{0, 1, 2},
		{0, 2, 3},
		{1, 3, 3},
		{1, 4, 1},
		{2, 3, 1},
		{2, 4, 1},
		{3, 5, 2},
		{4,	5, 3}
	});
	
	auto mfFF = maxFlowFF(net, 0, 5);
	cout << "Max Flow Ford-Fulkerson: " << mfFF() << "\nMinCut:\n";
	for (auto& edge : mfFF.minCut()) {
		cout << edge << endl;
	}
	cout << endl;
	
    auto mfPP = maxFlowPP(net, 0, 5);
	cout << "Max Flow Preflow-Push: " << mfPP() << "\nMinCut:\n";
	for (auto& edge : mfPP.minCut()) {
		cout << edge << endl;
	}
    
    cout << endl;
    
    auto mfD = maxFlowD(net, 0, 5);
    cout << "Max Flow Dinic: " << mfPP() << "\nMinCut:\n";
    for (auto& edge : mfD.minCut()) {
        cout << edge << endl;
    }

}

//
//  main.cpp
//  Graphs
//
//  Created by Oleg Bakharev on 06/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#include <iostream>
#include "sparseArray.hpp"
#include "denseGraph.h"
#include "sparseGraph.h"
#include "graphGen.h"
#include "searchGraph.h"
#include "directedGraph.h"
#include "dag.h"
#include "strongComponents.h"
#include "weightedGraph.h"
#include <fstream>
#include "mst.h"
#include "spt.h"
#include "spAll.h"
#include "spAllDag.h"
#include "maxFlow.h"

using namespace std;
using namespace Graph;

template <class G> void buildGraph(G& g) {
    insertEdges(g, {
        // Седжвик 18.5
        {0, 2}, {0, 5}, {0, 7},
        {1, 7},
        {2, 6},
        {3, 4}, {3, 5},
        {4, 5}, {4, 6}, {4, 7}
    });
}

template <class G> void buildBiGraph(G& g) {
    // Двудольный граф.  Седжвик 17.5
    insertEdges(g, {
        {0, 1}, {0, 3}, {0, 5},
        {1, 2},
        {2, 9},
        {3, 4},
        {4, 5}, {4, 11},
        {6, 7}, {6, 9},
        {7, 8},
        {8, 9},
        {9, 10}, {9, 12},
        {11, 12},
    });
}

template <class G> void buildEdgeSeparableGraph(G& g) {
    // Реберно-разделимый несвязный граф для мостов и шарниров. Седжвик Рис. 18.16.
    insertEdges(g, {
        {0, 1}, {0, 5}, {0, 6},
        {1, 2},
        {2, 6},
        {3, 4}, {3, 5},
        {4, 5}, {4, 9}, {4, 11},
        {6, 7},
        {7, 8}, {7, 10},
        {8, 10},
        {9, 11},
        {11, 12}
    });
}

template <class G> void buildDirGraph(G& g) {
	// Рис. 19.1 Седжвик.
    g.insert({0, 1});
    assert(g.edge(0, 1));
    g.remove({0, 1});
    assert(!g.edge(0, 1));
    insertEdges(g, {
		{4, 2}, {11, 12}, {4, 11}, {5, 4},
		{2, 3}, {12, 9}, {4, 3}, {0, 5},
		{3, 2}, {9, 10}, {3, 5}, {6, 4},
		{0, 6}, {9, 11}, {7, 8}, {6, 9},
		{0, 1}, {8, 9}, {8,7}, {7, 6},
		{2, 0}, {10, 12}
	});
}

template <class G> void buildDirGraphTC(G& g) {
    // Рис. 19.13 Седжвик.
    insertEdges(g, {
        {0, 2}, {0, 5}, {1, 0}, {2, 1}, {3, 2}, {3, 4}, {4, 5}, {5, 4}
    });
}

template <class G> void buildDAG(G& g) {
    // Рис. 19.21 Седжвик.
    insertEdges(g, vector<vector<size_t>>({
        {1, 2, 3, 5, 6 },
        {},
        {3},
        {4, 5},
        {9},
        {},
        {4, 9},
        {6},
        {7},
        {10, 11, 12},
        {},
        {12},
        {}
    }));
}

template <class G> void buildWeightedGraph(G& g) {
    // Рис 20.1 Седжвик.
    g.insert({0, 1, 1,});
    assert(g.edge(0, 1));
    g.remove({0, 1, 1});
    assert(!g.edge(0, 1));
    insertEdges(g, {
        {0, 6, .51},
        {0, 1, .32},
        {0, 2, .29},
        {4, 3, .34},
        {5, 3, .18},
        {7, 4, .46},
        {5, 4, .40},
        {0, 5, .60},
        {6, 4, .51},
        {7, 0, .31},
        {7, 6, .25},
        {7, 1, .21}
    });
}

template <class G> void buildWeightedDirGraph(G& g) {
    // Рис 21.1 Седжвик.
    g.insert({0, 1, 1});
    assert(g.edge(0, 1));
    g.remove({0, 1, 1});
    assert(!g.edge(0, 1));
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

template <class G> void buildWeightedDirGraphNegative(G& g) {
    // Рис 21.26 Седжвик.
    g.insert({0, 1, 1});
    assert(g.edge(0, 1));
    g.remove({0, 1, 1});
    assert(!g.edge(0, 1));
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

template <class G> void testGraph(G& g) {
	buildGraph(g);
//    buildWeightedGraph(g);
    cout << (g) << "BFS:\n";
    auto sb = searchTrace(g);
    auto bfs = BFS(g, sb);
    traverse(g, bfs);
    
	cout << "\nDFS:\n";
    auto sd = searchTrace(g);
    auto dfs = DFS(g, sd);
    traverse(g, dfs);
    
    auto cc = CC(g);
    cout << endl << cc.size() << " connected components\n";
	SCTrace(cout, cc);
	
	auto bi = BI(g);
	traverse(g, bi);
	cout << endl << bi.bipartite() << " bipartite status\n\n";
}

void testDenseGraph() {
	cout << "Dense graph:\n";
	DenseGraph dg(8);
	testGraph(dg);
}

void testSparseGraph() {
	cout << "Sparse graph:\n";
	SparseGraph sg(8);
	testGraph(sg);
}

void testGraphs() {
	testDenseGraph();
	testSparseGraph();
}

void testBrisgesJoints() {
    SparseGraph sg(13);
    
    buildEdgeSeparableGraph(sg);
    cout << sg;

    auto et = searchTrace(sg);
    auto bridges1 = Bridges(sg, et);
    et.reset();
    auto joints1 = ArtPoints(sg, et);
    return;
//    et.reset();
	
    DenseGraph dg(13);
    buildEdgeSeparableGraph(dg);
    
    auto bridges2 = Bridges(dg, et);
    et.reset();
    auto joints2 = ArtPoints(dg, et);
}

template <class G> void testDirGraph(G& g) {
//    buildDirGraph(g);
    buildDAG(g);
//    buildWeightedGraph(g);
	cout << g;
	auto s = searchTrace(g);
	auto dfs = DFS(g, s);
	traverse(g, dfs);
//    auto tc2 = TCW(g).getTC();
//    cout << "Transitive closure Warshall:\n" << tc2;
//    cout << "Transitive closure Warshall2:\n" << TCW(tc2).getTC();
    auto tc = TC(g).getTC();
    cout << "Transitive closure DFS:\n" << tc;
	auto tcsc = TCSC(g);
	tcsc.out(cout);
//    cout << "Transitive closure DFS2:\n" << TC(tc).getTC();
    auto ts = TS(g);
    cout << "Is Dag: " << ts.isDAG() << endl;
    if( ts.isDAG() ) {
        cout << "indices: [ 0  1  2  3  4  5  6  7  8  9 10 11 12]\n";
        cout << "topolog: " << ts.ts();
        cout << "relabel: " << ts.relabel();
    }
    
    auto tssq = TSSQ(g);
    cout << "TSSQ\n";
    cout << "indices: [ 0  1  2  3  4  5  6  7  8  9 10 11 12]\n";
    cout << "topolog: " << ts.ts();
    cout << "relabel: " << ts.relabel();
    
    auto sc = CC(g);
    SCTrace(cout, sc);
    auto scTar = SCTar(g);
    SCTrace(cout, scTar);
    auto scGab = SCGab(g);
    SCTrace(cout, scGab);
}

void testDenseDirGraph() {
	cout << "Dense directed graph:\n";
//	DenseGraphD g(13);
	DenseDAG g(13);
    testDirGraph(g);
}

void testSparseDirGraph() {
	cout << "Sparse directed graph:\n";
//	SparseGraphD g(13);
    SparseDAG g(13);
	testDirGraph(g);
}

void testDirGraphs() {
	testDenseDirGraph();
	cout << endl;
	testSparseDirGraph();
}

template <class G> void testWeightedGraph(G& g) {
    //    buildWeightedGraph(g);
    cout << (g) << "BFS:\n";
    auto sb = searchTrace(g);
    auto bfs = BFS(g, sb);
    traverse(g, bfs);
    
    cout << "\nDFS:\n";
    auto sd = searchTrace(g);
    //SearchTraceD sd(g.size());
    
    auto dfs = DFS(g, sd);
    traverse(g, dfs);
    
    auto prim = mstPrim(g);
    cout << prim;
    
    auto krus = mstKrus(g);
    cout << krus;

    auto dijkstra = sptDijkstra(g, 0);
    cout << dijkstra;
	
    auto bfNaive = sptBFNaive(g, 0);
    cout << bfNaive;
    
    auto bfAdvanced = sptBFAdvanced(g, 0);
    cout << bfAdvanced;
}

void testDenseWeightedGraph() {
    cout << "Dense weighted graph:\n";
    DenseGraphW g(8);
    buildWeightedGraph(g);
    testWeightedGraph(g);
}

void testSparseWeightedGraph() {
    cout << "\nSparse weighted graph:\n";
    SparseGraphW g(8);
    buildWeightedGraph(g);
    testWeightedGraph(g);
}

void  testWeightedGraphs()
{
    testDenseWeightedGraph();
    testSparseWeightedGraph();
}

void testDenseWeightedDirGraph() {
    cout << "Dense weighted dir graph:\n";
    DenseGraphWD g(6);
    buildWeightedDirGraph(g);
//    buildWeightedDirGraphNegative(g);
    testWeightedGraph(g);
}

void testSparseWeightedDirGraph() {
    cout << "\nSparse weighted dir graph:\n";
    SparseGraphWD g(6);
    buildWeightedDirGraph(g);
//    buildWeightedDirGraphNegative(g);
    testWeightedGraph(g);
}

void  testWeightedDirGraphs()
{
    testDenseWeightedDirGraph();
    testSparseWeightedDirGraph();
}

void test_k_neighbourGraph() {
    const size_t N = 50000;
    SparseGraph g(N);
    k_neighbor(g, N*4, N/200);
//    cout << g;
    size_t from = 0;
    size_t to = N/2;
    auto pathCount = countBFS(g);
    ofstream ofs("out.txt");
    ofs << g;
    cout << "count of shortest paths from " << from << " to " << to << " is " << pathCount.solve(from, to);
}

int main(int argc, const char * argv[]) {
//    testBrisgesJoints();
    testSparseArray();
    testGraphs();
//	testDirGraphs();
//    test_k_neighbourGraph();
//    testWeightedGraphs();
//    testWeightedDirGraphs();
//    spAllTest();
//    spAllDagTest();
//    maxFlowTest();
	
    return 0;
}

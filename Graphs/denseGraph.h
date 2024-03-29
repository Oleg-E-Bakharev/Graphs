//
//  denseGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 07/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef denseGraph_h
#define denseGraph_h

#include "graphBase.h"
#include "matrix.h"
#include <cassert>
#include <type_traits>

using namespace std;

// Специализация for_iter_t для итерации по смежным вершинам.
// Для контекстов, WeightType которых есть bool.
template <class T, class C> class for_iter_t <T, C,
typename enable_if<is_same<typename C::WeightType, bool>::value>::type> {
    T& t;
    size_t pos;
public:
    for_iter_t(T& t) : t(t), pos(0) { if(!t[pos]) ++*this; }
    size_t operator*() { return pos; }
    bool operator != (const for_iter_t& f) const { assert(&f.t == &t); return pos != t.size(); }
    void operator++() { while(++pos < t.size() && t[pos] == false); }
};

///////////////////////////////////////
namespace Graph {

	// Граф на матрице смежности.
	template<typename GT = GraphTraits>
	class DenseGraph_T {
        using AdjMatrix = matrix<typename GT::WeightType, GT>;
		AdjMatrix _adj;
		size_t _edges = 0;
        static constexpr typename GT::WeightType EPS = std::numeric_limits<typename GT::WeightType>::epsilon();
        
	public:
		using Traits = GT;
        using Edge = typename Traits::EdgeType;
        using reference = typename AdjMatrix::reference::reference;
        using WeightType = typename Traits::WeightType;
		using NodeType = typename GT::AdjListNodeType;
		
		DenseGraph_T(size_t v) : _adj(v, v) {}
        
        // Конструктор от другого графа.
        template<class G> DenseGraph_T ( const G& g,
            typename enable_if<is_base_of<GraphTraits, typename G::Traits>::value>::type* = 0 ) : _adj(g.size(), g.size())
        {
            for ( size_t v = 0; v < g.size(); v++ )
                for( size_t w = 0; w < g.size(); w++ )
                    if ( g.edge(v, w) ) {
                        _adj[v][w] = true;
                        _edges++;
                    }
        }
		
		size_t size() const { return _adj.h(); }
		size_t edgesCount() const { return _edges; }
		
		constexpr bool directed() const { return Traits::directed; }
		
		void insert(const Edge& e) {
			size_t v(e.v), w(e.w);
            if (!directed() && v == w) {
                return;
            }
            reference x = _adj[v][w];
            if (!x) {
                _edges++;
                x = e;
            }
			assert(edge(v,w));
            if( !directed() ) {
                _adj[w][v] = e.inverse();
            }
		}
		
		void remove(Edge e) {
			size_t v(e.v), w(e.w);
			reference x = _adj[v][w];
            if (x) {
                _edges--;
            }
			x = false;
			assert(!edge(v,w));
            if (!directed()) {
                _adj[w][v] = false;
            }
		}
		
		bool edge(size_t v, size_t w) const { return _adj[v][w]; }
        				
        using AdjIter = typename AdjMatrix::vec;
		
        AdjIter adjacent(size_t v) const { return _adj[v]; }
        // Итератор смежности по транспонированному графу.
        AdjIter adjacentTranspond(size_t v) const { return _adj.col(v); }
		
		using AdjMethod = decltype(&DenseGraph_T::adjacent);
		
		void reweight(size_t v, const NodeType& node, WeightType newWeight) {
            if (std::abs(newWeight) < EPS) newWeight = EPS;
			_adj[v][node.dest] = newWeight;
		}
	};
	
	using DenseGraph = DenseGraph_T<GraphTraits>;
	using DenseGraphD = DenseGraph_T<DirectedGraphTraits>;
    using DenseDAG = DenseGraph_T<DAGTraits>;
}

#endif /* denseGraph_h */

//
//  sparceGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 08/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef sparceGraph_h
#define sparceGraph_h

#include "graphBase.h"
#include <vector>
#include <algorithm>
#include <set>

namespace Graph {
    
    // Граф на списке смежности вершин.
	template<class GT = GraphTraits>
    class SparseGraph_T {
	public:
        using NodeType = typename GT::AdjListNodeType;
	private:
        using AdjList = vector<NodeType>;
		using AdjLists = vector<AdjList>;
        AdjLists _adj;
        size_t _edges = 0;
        bool _ready = false;
        
        void prepare_() const {
            const_cast<SparseGraph_T&>(*this).prepare_();
        }
        
        // Сортирует списки смежности и удаляет дубликаты. Вычисляет количество ребер.
        void prepare_() {
            if (_ready) return;
            _edges = 0;
            for( size_t v = 0; v < _adj.size(); v++ ) {
                AdjList& l = _adj[v];
                set<NodeType> s(l.begin(), l.end());
                l.clear();
                for( auto& w : s ) {
                    l.push_back(w);
                    if( v < w ) {
                        _edges++;
                    } else if (directed()) {
                        _edges++;
                    }
                }
            }
            _ready = true;
        }

    public:
		using Traits = GT;
        using Edge = typename Traits::EdgeType;
        using WeightType = typename Traits::WeightType;
		
        SparseGraph_T(size_t vertices) : _adj(vertices) {}
		
        // Кол-во вершин
        size_t size() const { return _adj.size(); }
		
        // Кол-во ребер
        size_t edgesCount() const { prepare_(); return _edges; }
		
		constexpr bool directed() const { return Traits::directed; }
		
        // Вставка.
        void insert(const Edge& e) {
            size_t v(e.v), w(e.w);
			if ( !directed() && v == w ) {
				return;
			}
            // Необходима приготовление.
            _ready = false;
			_edges++;
            _adj[v].push_back(e);
			if(!directed()) {
				_adj[w].push_back(e.inverse());
			}
        }
		
        // Удаление.
        void remove(const Edge& e) {
            prepare_();
            size_t v(e.v), w(e.w);
            AdjList& lv = _adj[v];
            // lower_bound это binary_search на векторах.
            auto pos = lower_bound(lv.begin(), lv.end(), w );
            if( pos != lv.end() ) {
                lv.erase(pos);
                _edges--;
                if( !directed() ) {
                    AdjList& lw = _adj[w];
                    lw.erase(lower_bound(lw.begin(), lw.end(), v));
                }
            }
        }
        
        // Еть ли ребро {v, w}?
        bool edge(size_t v, size_t w) const {
            prepare_();
			const AdjList& l = _adj[v];
			return binary_search(l.begin(), l.end(), w); // На векторах O(log(N))
        }
        
        // Итератор по смежным вершинам графа.
        using AdjIter = AdjList;
        const AdjIter& adjacent(size_t v) const {
            prepare_();
            return _adj[v];
        }
        AdjIter& adjacent(size_t v) {
            prepare_();
            return _adj[v];
        }
		
		void reweight(size_t v, const NodeType& node, WeightType weight) {
			const_cast<NodeType&>(node).weight = weight;
		}
        
//        const NodeType& node(size_t v, size_t w) const {
//            return _adj[v][w];
//        }
//        NodeType& node(size_t v, size_t w) {
//            return _adj[v][w];
//        }
    };
	
	using SparseGraph = SparseGraph_T<GraphTraits>;
	using SparseGraphD = SparseGraph_T<DirectedGraphTraits>;
    using SparseDAG = SparseGraph_T<DAGTraits>;
}

#endif /* sparceGraph_h */

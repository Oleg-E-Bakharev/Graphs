//
//  directedGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 15/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef directedGraph_h
#define directedGraph_h

#include "debug.h"
#include "graphBase.h"
#include "searchGraph.h"
#include <cassert>
#include <algorithm>
#include <ostream>

using namespace std;

namespace Graph {
    
	////////////////////////////////////////////////////////////////////////////
	// Специализация DFS для ориентированных графов. Метод traverse.
    template <class G, class Inspector> class DFS_T<G, Inspector,
    typename enable_if<is_base_of<DirectedGraphTraits, typename G::Traits>::value>::type> {
		const G& g;
		Inspector& i;
		vector<size_t> enter; // порядок входов в вершины.
		vector<size_t> leave; // порядок выходов из вершин.
		size_t cnt;
		size_t depth;
	public:
        DFS_T( const G& g, Inspector& i ) : g(g), i(i), enter(g.size(), -1), leave(g.size(), -1), cnt(0), depth(0) {trace("DFS_T directed");}
		
		bool operator() (size_t v, vector<bool>& c ) {
			c[v] = true;
			enter[v] = cnt++;
			depth++;
			for ( size_t w : g.adjacent(v) ) {
				EdgeRole et = Tree;
				if (enter[w] == -1) {
					assert(c[w] == false);
					i.visit( {v, w}, depth, et );
					(*this)(w, c);
				} else {
                    if ( leave[w] == -1 ) {
                        et = Back;
                    } else if ( enter[v] < enter[w] ) {
                        et = Forward;
                    } else et = Cross;
					i.visit( {v, w}, depth, et );
				}
			}
			leave[v] = cnt++;
			depth--;
			return true;
		}
	};
    
    /////////////////////////////////////////////////////////////////////////////////////
    // TransitiveClosure Warshall. Транзитивное замыкание Уоршелла. Седжвик 19.3
    // O(V^3)
    // Идея: В конце 0-й итерации по i. Результирующий граф содержит 1 на пересечении s и t в случае ориентированного пути s-t или s-0-t.
    // После 1-й итерации 1 добавляется в случае: s-1-t, s-1-0-t, s-0-1-t. И т.д.
    // После i-й итерации 1 добавляется в случае пути s-t который не содержит вершин > i.
    // Эффективен для плотных графов на матрице смежности.
    class TCW {
        // Исходим из предположения, что граф транзитивного замыкания будет очень плотным.
        // Плюс граф на списках смежности не приспособлен для использования с динамической вставкой ребер.
        DenseGraphD tc;
    public:
        TCW (const DenseGraphD& g) : tc(g) { trace("TC Warshall");
            for ( size_t i = 0; i < g.size(); i++ ) {
                tc.insert({i, i});
            }
            for ( size_t i = 0; i < g.size(); i++ ) {
                for ( size_t s = 0; s < g.size(); s++ ) {
                    if( tc.edge(s, i) ) {
                        for( size_t t = 0; t < g.size(); t++ ) {
                            if ( tc.edge(i, t) ) {
                                tc.insert({s, t});
                            }
                        }
                    }
                }
            }
        }
        
        bool reachable( size_t v, size_t w ) const { return tc.edge(v , w); }
        const DenseGraphD& getTC() const { return tc; }
    };
	
	//////////////////////////////////////////////////////////////////////////////////
	// TransitiveClosure - Транзитивное замыкание - построение множества достижимости на основе DFS. Седжвик 19.4
	// O(V(V + E)).
	template <class G, class Context = typename G::Traits> class TC_T {
		const G& g;
		// Исходим из предположения, что граф транзитивного замыкания будет очень плотным.
		// Плюс граф на списках смежности не приспособлен для использования с динамической вставкой ребер.
		DenseGraphD tc;
		
		void dfs_( size_t v, size_t w ) {
			tc.insert({v, w});
			for ( size_t t : g.adjacent(w)) {
				if (!tc.edge(v, t)) dfs_(v, t);
			}
		}
		
	public:
		TC_T( const G& g) : g(g), tc(g.size()) { trace("TC_T DFS");
			for( size_t v = 0; v < g.size(); v++ )
                dfs_(v, v);
		}
		
		bool reachable( size_t v, size_t w ) const { return tc.edge(v , w); }
		const DenseGraphD& getTC() const { return tc; }
	};
	
	// Ускоритель вызова.
	template<class G> TC_T<G> TC(const G& g) { return TC_T<G>(g); }    
}

#endif /* directedGraphs_h */

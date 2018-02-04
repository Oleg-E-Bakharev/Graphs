//
//  graph.h
//  Graphs
//
//  Created by Oleg Bakharev on 06/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef graph_h
#define graph_h

#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <type_traits>
#include "matrix.h"

using namespace std;

namespace Graph {

    // Ребро графа.
    struct GraphEdge {
        size_t v;
        size_t w;
        GraphEdge(size_t v, size_t w):v(v),w(w) {}
        
        operator bool() const { return true; }
        GraphEdge inverse() const { return {w, v}; }
    };

    // Структура описывающая узел 
    struct GraphAdjListNode {
        size_t v; // вершина куда направлено ребро.
        GraphAdjListNode(const size_t& v) : v(v) {}
        GraphAdjListNode(const GraphEdge& e) : v(e.w) {}
        operator size_t&() { return v; }
        operator const size_t&() const { return v; }
    };
    
    // Свойства различных графов.
	struct GraphTraits {
		static const bool directed = false;
        static const bool acyclic = false;
        using EdgeType = GraphEdge;
        using WeightType = bool;
        using AdjListNodeType = GraphAdjListNode;
	};
	
	struct DirectedGraphTraits : public GraphTraits {
        static const bool directed = true;
	};
    
    struct DAGTraits : public DirectedGraphTraits {
        static const bool acyclyc = true;
    };
		
    // АТД графа
    class Graph {
    public:
		
		using Traits = GraphTraits;
        using Edge = Traits::EdgeType;
		
        Graph(size_t vertices, bool isDirected);
        // Кол-во вершин
        size_t size() const;
		
        // Кол-во ребер
        size_t edgesCount() const;
		
		// Ориентирован ли граф
        bool directed() const { return Traits::directed; }
		
		// Вставить ребро
        void insert(const Edge&);
		
		// Удалить ребро
        void remove(const Edge&);
		
        // Еть ли ребро {v, w}
        bool edge(size_t v, size_t w) const;
        
        // Вес ребра).
        Traits::WeightType weight(size_t v, size_t w) const;
        
        // Итератор по смежным вершинам графа. Для for(:)
		class AdjIter;

        AdjIter adjacent(size_t v) const;
    };
        
    // Все ребра графа ввиде vector<Edge>
    template <class G>
    vector<typename G::Edge> edges(const G& g, bool skipUndirectedReverse = true) {
        vector<typename G::Edge> a;
        a.reserve(g.edgesCount());
        for (size_t v = 0; v < g.size(); v++) {
            for(auto w : g.adjacent(v)) {
                if ((g.directed() || !skipUndirectedReverse) || v < w) {
                    a.push_back({v, w});
                }
            }
        }
		
        return a;
    }

    // Вывод графа в виде списка смежности.
    template <class G>
    void show(const G& g, ostream& os = cout) {
        os << "v: " << g.size() << endl;
        os << "e: " << g.edgesCount() << endl;
        for( size_t v = 0; v < g.size(); v++ ) {
            os << setw(2) << v << ":";
			for( auto e : g.adjacent(v) ) {
				os << setw(2) << e << " ";
			}
            os << endl;
        }
        os << endl;
	}
    
    // Вывод для ввода задания студентам.
//    template <class G>
//    void show(const G& g, ostream& os = cout) {
//        os << g.size() << endl;
//        os << g.edgesCount() << endl;
//        for( size_t y = 0; y < g.size(); y++ ) {
//            assert(!g.edge(y, y));
//            for( size_t x : g.adjacent(y)) {
//                if (x > y) {
//                    assert(g.edge(x, y));
//                    os << y << " " << x << endl;
//                }
//            }
//        }
//        os << endl;
//    }
	
	// Оператор вывода графа в ostream.
	template<class G, class T = typename G::Traits>
	typename enable_if< is_base_of<GraphTraits, T>::value, ostream& >::type
	operator << (ostream& os, const G& g) {
		show(g, os);
		return os;
	}
    
    // Ввод графа в виде пар номеров смежных вершин.
    template <class G>
    void scan(G& g) {
        size_t v, w;
        while( cin >> v >> w ) {
            assert(v >= 0 && w>= 0 && v < g.size() && w < g.size());
            g.insert( v, w );
        }
    }
	
	// Вектор степеней вершин графа.
	template <class G>
	vector<size_t> degree( const G& g) {
		vector<size_t> deg(g.size());
		for( size_t v = 0; v < g.size(); v++ ) {
			for( auto w : g.adjacent(v) ) {
				deg[v]++; (void)w; // убираем warning.
			}
		}
		return deg;
	}    
}

#endif /* graph_h */

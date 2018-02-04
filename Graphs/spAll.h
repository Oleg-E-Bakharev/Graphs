//
//  spAll.h
//  Graphs
//
//  Created by Oleg Bakharev on 21/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//
// Нахождение кратчайших путей между всеми парами вершин.

#ifndef spAll_h
#define spAll_h

#include <iostream>
#include <vector>
#include <memory>
#include "spt.h"
#include "matrix.h"
#include "weightedGraph.h"

namespace Graph {
    
    // АТД кратчайших путей. Седжвик 21.2
    template <typename G> class SPAll {
    public:
        // Вес пути между вершинами.
        double distance(size_t, size_t) const;
        // кратчайший путь между вершинами.
        std::vector<typename G::Edge> path(size_t, size_t) const;
    };
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Диаметр взвешенного графа. Седжвик 21.3
    template <typename G, typename SPAll> class Diameter {
        const G& _g;
        const SPAll& _sp;
        size_t _vMax = -1;
        size_t _wMax = -1;
        double _dMax = 0;
        using Edge = typename G::Edge;
        
    public:
        Diameter( const G& g, const SPAll& sp ) : _g(g), _sp(sp) {
            for (size_t v = 0; v < g.size(); v++) {
                for (size_t w = 0; w < (g.directed() ? g.size() : v); w++) {
                    double d = _sp.distance(v, w);
                    if ( d > _dMax) {
                        _dMax = d;
                        _vMax = v;
                        _wMax = w;
                    }
                }
            }
        }
        
        operator double() const { return _dMax; }
        
        operator Edge() const { return {_vMax, _wMax, _dMax}; }
        
        friend std::ostream& operator<<(std::ostream& os, const Diameter& d) {
            using namespace std;
            os << "Weighted Graph Diameter: " << Edge(d) << endl;
            
            for (auto& edge : d._sp.path(d._vMax, d._wMax)) {
                os << edge << endl;
            }
            return os;
        }
    };
    
    template <typename G, typename SPAll> Diameter<G, SPAll> diameter(const G& g, const SPAll& sp) { return {g, sp}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Вычисление всех кратчайших путей алгоритмом Дейкстры. O(VE*lg(V))
    template <class G> class SPAllDijkstra_T {
        using Spt = SptDijkstra_T<G>;
        using Edge = typename G::Edge;
        
        const G& _g;
        std::vector<std::shared_ptr<Spt>> _spAll;
        
    public:
        SPAllDijkstra_T( const G& g ) : _g(g) {
            _spAll.reserve(g.size());
            for (size_t v = 0; v < g.size(); v++) {
                _spAll.push_back(std::make_shared<Spt>(g, v));
            }
        }
        
        // Вес пути между вершинами.
        double distance(size_t v, size_t w) const {
            return _spAll[v]->distance(w);
        }
        
        // Кратчайший путь между вершинами.
        std::vector<Edge> path(size_t v, size_t w) const {
            return _spAll[v]->spt(w);
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SPAllDijkstra_T& spAll) {
            using namespace std;
            os << "SPAllDijkstra" << endl << "Distances:\n";
            for (int i = 0; i < spAll._g.size(); i++) {
                os << i << ": ";
                for (int j = 0; j < spAll._g.size(); j++) {
                    double d = spAll._spAll[i]->distance(j);
                    os << setw(4);
                    if (d != 0.) {
                        os << d;
                    } else {
                        os << "";
                    }
                    os << " ";
                }
                os << endl;
            }
            os << "Sources\n";
            for (int i = 0; i < spAll._g.size(); i++) {
                os << i << ": ";
                for (int j = 0; j < spAll._g.size(); j++) {
                     os << setw(2) << (int)spAll._spAll[i]->source(j) << " ";
                }
                os << endl;
            }
            return os;
        }
    };
    
    template<class G> SPAllDijkstra_T<G> spAllDijkstra(const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Вычисление всех кратчайших путей алгоритмом Флойда. Седжвик 21.5 O(V^3).
    // По сути - взвешенное транзитивное замыкание Уоршелла.
    template <class G> class SPAllFloyd_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        using Node = typename Traits::AdjListNodeType;
        const Weight INF = std::numeric_limits<double>::max();
        
        const G& _g;
        matrix<double> _weight; // Веса кратчайшего пти от i к j
        matrix<size_t> _next; // Следующее ребро на кратчайшем пути от i к j.
        
        // s-t: ребро графа. i - вершина текущей внешней итерации алгоритма.
        void tryRelax_(size_t s, size_t t, size_t i) {
            if (_weight[s][t] > _weight[s][i] + _weight[i][t]) {
                _next[s][t] = _next[s][i];
                _weight[s][t] = _weight[s][i] + _weight[i][t];
            }
        }
        
    public:
        SPAllFloyd_T (const G& g) : _g(g), _weight(g.size(), g.size(), INF), _next(g.size(), g.size(), -1) {
            for ( size_t i = 0; i < g.size(); i++ ) {
                for ( const Node& node : g.adjacent(i) ) {
                    _weight[i][node.dest] = node.weight;
                    _next[i][node.dest] = node.dest;
                }
            }
            
            for ( size_t i = 0; i < g.size(); i++ ) {
                for ( size_t s = 0; s < g.size(); s++ ) {
                    if( _next[s][i] != -1 ) {
                        for( size_t t = 0; t < g.size(); t++ ) {
                            if (s != t) {
                                tryRelax_(s, t, i);
                            }
                        }
                    }
                }
            }
        }
        
        // Вес пути между вершинами.
        double distance(size_t v, size_t w) const {
            Weight wt = _weight[v][w];
            return wt != INF ? wt : 0;
        }
        
        // Кратчайший путь между вершинами.
        std::vector<Edge> path(size_t v, size_t w) const {
            using namespace std;
            vector<Edge> path;
            
            if (_next[v][w] == -1) {
                // Пути из v в w нет.
                return path;
            }
            
            Weight wt = 0.;
            while( w != v ) {
                size_t p = _next[v][w];
                path.push_back({v, p, Edge(v, p, wt += _weight[v][p])});
                v = p;
            }
            
            return path;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SPAllFloyd_T& spAll) {
            using namespace std;
            os << "SPAllFloyd" << endl << "Distances:\n";
            for (int i = 0; i < spAll._g.size(); i++) {
                os << i << ": ";
                for (int j = 0; j < spAll._g.size(); j++) {
                    double d = spAll._weight[i][j];
                    os << setw(4);
                    if (d != numeric_limits<double>::max()) {
                        os << d;
                    } else {
                        os << "";
                    }
                    os << " ";
                }
                os << endl;
            }
            os << "Paths\n";
            for (int i = 0; i < spAll._g.size(); i++) {
                os << i << ": ";
                for (int j = 0; j < spAll._g.size(); j++) {
                    os << setw(2) << (int)spAll._next[i][j] << " ";
                }
                os << endl;
            }
            return os;
        }
    };
    
    template<class G> SPAllFloyd_T<G> spAllFloyd(const G& g) { return {g}; }
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Алгоритм Джонсона с перевзвешиванием графа вычисления всех кратчайших путей за VElog(V).
	// Алгоритмом Беллмана-Форда вычисляем лес кратчайших Седжвик Лемма 21.25, Рис 21.31-32.
	template <class G> class SPAllJohnson_T {
		using Traits = typename G::Traits;
		using Edge = typename Traits::EdgeType;
		using Weight = typename Traits::WeightType;
		using Node = typename Traits::AdjListNodeType;
		const Weight INF = std::numeric_limits<double>::max();

		using SptBF = SptBFAdvanced_T<G>;
		
		G& _g;
		std::shared_ptr<SPAllDijkstra_T<G>> _spAll;
		
		void reweight_(SptBF& bf) {
			for (size_t v = 0; v < _g.size(); v++) {
				for( const Node& n : _g.adjacent(v)) {
					_g.reweight(v, n, n.weight + bf.distance(v) - bf.distance(n.dest) + std::numeric_limits<double>::epsilon());
				}
			}
		}
		
		bool _fail;
		
	public:
		SPAllJohnson_T (G& g) : _g(g), _spAll(nullptr) {
			SptBF bf(g, 4);
			_fail = bf.hasNegativeCycle();
			if (!_fail) {
				reweight_(bf);
				_spAll = std::make_shared<SPAllDijkstra_T<G>>(g);
			}
		}
		
		bool hasNegativeCycles() const { return _fail; }
				
		// Вес пути между вершинами.
		double distance(size_t v, size_t w) const {
			assert(!_fail);
			return _spAll->distance(v, w);
		}
		
		// Кратчайший путь между вершинами.
		std::vector<Edge> path(size_t v, size_t w) const {
			assert(!_fail);
			return _spAll->path(v, w);
		}
		
		friend std::ostream& operator<<(std::ostream& os, const SPAllJohnson_T& spAll) {
			using namespace std;
			os << "SPAllJohnson\n";
			if (spAll._fail) {
				os << "Negative cycles detected\n";
				return os;
			}
			cout << "Reweighted graph:\n" << spAll._g;
			cout << *spAll._spAll;
			return os;
		}
	};
	
	template<class G> SPAllJohnson_T<G> spAllJohnson(G& g) { return {g}; }
	
} // namespace Graph

void spAllTest();

#endif /* spAll_h */

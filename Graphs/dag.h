//
//  dag.h
//  Graphs
//
//  Created by Oleg Bakharev on 22/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//
// Алгоритмы для DAG - directed aciclic graphs.

#ifndef dag_h
#define dag_h

#include "directedGraph.h"
#include "strongComponents.h"
#include <memory>
#include <queue>

namespace Graph {
    
    // Специализация транзитивного замыкания для DAG. Седжвик 19.9
    template <class G> class TC_T<G, DAGTraits> {
        const G& g;
        vector<size_t> enter;
        size_t cnt;
        // Исходим из предположения, что граф транзитивного замыкания будет очень плотным.
        // Плюс граф на списках смежности не приспособлен для использования с динамической вставкой ребер.
        DenseGraphD tc;
        
        void dfs_( size_t v ) {
            enter[v] = cnt++;
			tc.insert({v, v});
            for ( size_t w : g.adjacent(v) ) {
				tc.insert({v, w});
				if (enter[w] == -1) {
					dfs_(w);
				} else if ( enter[w] > enter[v] ) {
					continue; // Для прямых ребер все уже посчитано.
				}
				for ( size_t i = 0; i < tc.size(); i++ ) {
                    if ( tc.edge(w, i) ) tc.insert({v, i});
				}
            }
        }
        
    public:
        TC_T( const G& g) : g(g), tc(g.size()), enter(g.size(), -1), cnt(0) { trace("TC_T DAG");
			for( size_t v = 0; v < g.size(); v++ ) {
				dfs_(v);
			}
        }
        
        bool reachable( size_t v, size_t w ) const { return tc.edge(v , w); }
        const DenseGraphD& getTC() const { return tc; }
    };
    
    ///////////////////////////////////////////////////////////////
    // DAG Topoligical Sort. Седжвик 19.6
    template <class G> class TS_T {
        const G& g;
        vector<size_t> enter; // Порядок входов в вершины.
        vector<size_t> leave; // Обратный вектор переименования.
        vector<size_t> top; // Обратный топологический порядок.
        size_t cnt;
        bool isDag;
        
        bool dfs_ (size_t v) {
            enter[v] = cnt++;
            for ( size_t w : g.adjacent(v) ) {
                if ( enter[w] == -1 ) {
                    if(!dfs_(w)) return false;
                } else {
                    if ( leave[w] == -1 ) {
                        // back edge detected. Not a DAG.
                        isDag = false;
                        return false;
                    }
                }
            }
            leave[v] = top.size();
            top.push_back(v);
            return true;
        }
        
    public:
        TS_T( const G& g) : g(g), enter(g.size(), -1), leave(g.size(), -1), cnt(0), isDag(true) { trace("TS_T");
            top.reserve(g.size());
            for( size_t v = 0; v < g.size(); v++ ) {
                if( enter[v] == -1 )
                    if ( !dfs_(v) ) break;
            }
            reverse(top.begin(), top.end());
        }
        
        bool isDAG() const { return isDag; }
        
        // Возвращает последовательность вершин в топологическом порядке.
        const vector<size_t>& ts(){ return top; }

        // Возвращает i-ю вершину в топологическом порядке.
        size_t operator[] (size_t i) const { return top[i]; }

        // Возвращает вектор переименования вершин так, чтобы ребра выходили из вершин с большим номером и входили в вершины с меньшим номером.
        const vector<size_t>& relabel() { return leave; }
    };
    
    // Ускоритель вызова.
    template<class G> TS_T<G> TS(const G& g) { return TS_T<G>(g); }
	
    ///////////////////////////////////////////////////////////////
    // DAG Topoligical Sort. На основе очереди истоков. Седжвик 19.8
    template <class G> class TSSQ_T {
        
        std::vector<size_t> _top;
        std::vector<size_t> _relabel;
    public:
        TSSQ_T (const G& g) : _top(g.size(), -1), _relabel(g.size(), -1) {
            std::vector<size_t> ins(g.size());
            // Заполняем вектор степеней захода для каждой вершины.
            for (size_t v = 0; v < g.size(); g++) {
                for (auto w : g.adjacent(v)) {
                    ins[w]++;
                }
            }
            
            std::queue<size_t> sq; // source queue.
            // Заполняем очередь истоков.
            for (size_t v = 0; v < g.size(); g++) {
                if (ins[v] == 0) {
                    sq.push(v);
                }
            }
            
            // Удаляем истоки, уменьшаем степени захода вершин куда шли ребра из истоков. Новые истоки заносим в очередь.
            for (size_t v = 0; !sq.empty(); v++) {
                size_t s = sq.back();
                sq.pop();
                _top[v] = s;
                _relabel[s] = v;
                for (auto w : g.adjacent[s]) {
                    if (--ins[w] == 0) {
                        sq.push(w);
                    }
                }
            }
        }
        
        // Возвращает последовательность вершин в топологическом порядке.
        const vector<size_t>& ts() { return _top; }

        // Возвращает i-ю вершину в топологическом порядке.
        size_t operator[] (size_t i) const { return _top[i]; }
        
        // Возвращает вектор переименования вершин так, чтобы ребра выходили из вершин с большим номером и входили в вершины с меньшим номером.
        const vector<size_t>& relabel() { return _relabel; }
    };
    
    // Ускоритель вызова.
    template<class G> TS_T<G> TSSQ(const G& g) { return TS_T<G>(g); }
    
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Транзитивное замыкание на основе сильных компонент. Седжвик 19.13.
	// Сначала вычисляем сильные компоненты по алгоритму Габова, потом строим базовый DAG. Потом строим ТС базового DAG.
	template<class G> class TCSC_T {
		const G& g;
		SCGab_T<G> sc;
		DenseDAG dag; // Удобно реализовано избавление от многократного дублирования ребер.
		shared_ptr<TC_T<DenseDAG>> dagTCPtr;
		
	public:
		TCSC_T( const G& g ) : g(g), sc(g), dag(sc.size()) { trace("TCSC_T");
			for ( size_t v = 0; v < g.size(); v++ )
				for ( size_t w : g.adjacent(v) )
					dag.insert({sc.id(v), sc.id(w)});
			dagTCPtr = make_shared<TC_T<DenseDAG>>(TC(dag));
		}
		
		bool reachable( size_t v, size_t w ) const { return dagTCPtr->reachable(sc.id(v) ,sc.id(w)); }
		
        void out(std::ostream& os) {
			os << dag;
			os << "DAG TC\n";
			os << dagTCPtr->getTC();
			os << "Result TC\n";
			for ( size_t v = 0; v < g.size(); v++ ) os << sc.id(v) << ", ";
			os << endl;
			DenseGraphD dg(g.size());
			for ( size_t v = 0; v < g.size(); v++ )
				for ( size_t w = 0; w < g.size(); w++ )
					if( reachable(v, w) )
						dg.insert({v, w});
			os << dg;
		}
	};
			
	// Ускоритель вызова.
	template<class G> TCSC_T<G> TCSC(const G& g) { return TCSC_T<G>(g); }
			
};

#endif /* dag_h */

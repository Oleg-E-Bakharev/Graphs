//
//  strongComponents.h
//  Graphs
//
//  Created by Oleg Bakharev on 23/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef strongComponents_h
#define strongComponents_h

#include <vector>
#include <stack>
#include "graphBase.h"
#include "debug.h"

using namespace std;

namespace Graph {
	
	template <class InG, class OutG> void reverseGraph( const InG& i, OutG& o ) {
		for ( size_t v = 0; v < i.size(); v++ ) {
			for ( size_t w : i.adjacent(v) ) {
				o.insert({w, v});
			}
		}
	}

    ////////////////////////////////////////////////////////////////////////////
    // Сильные компоненты. Специализация CC_T для ориентированных графов. Алгоритм Косарайю. Седжвик 19.10
    // Строгое доказательтсво корректности см. Кормен 22.5.
    template <class G, class C> class CC_T<G, C,
    typename enable_if<is_same<G, DenseGraph_T<C>>::value &&
    !is_base_of<DirectedGraphTraits, C>::value>::type> {
        size_t cnt;
        size_t scnt;
        vector<size_t> ids, leave;
        
        void dsf_( const G& g, size_t v) {
            ids[v] = scnt;
            for( size_t w : g.adjacent(v) )
                if(ids[w] == -1) dsf_(g, w);
            leave[cnt++] = v;
        }
        
        template<class T> friend void SCTrace(ostream&, const T&);
		
    public:
        CC_T( const G& g ) : cnt(0), scnt(0), ids(g.size(), -1), leave(g.size()) { trace("CC_T Kosaraju");
			// Делаем "топсорт" на обращении графа.
			G r(g.size());
			reverseGraph(g, r);
            for ( size_t v = 0; v < r.size(); v++ )
                if( ids[v] == -1) dsf_(r, v);
            
            ids.assign(g.size(), -1);
            vector<size_t> order = leave;
            cnt = scnt = 0;
            
            for ( size_t v = g.size() - 1; v < -1; v-- )
                if ( ids[order[v]] == -1) {
                    dsf_(g, order[v]);
                    scnt++;
                }
        }
        
        size_t size() const { return scnt; }
		size_t id(size_t v) const { return ids[v]; }
        bool connected( size_t v, size_t w ) const { return ids[v] == ids[w]; }
    };
    
    // Сильные компоненты. Специализация CC_T для графа на матрице смежности. Алгоритм Косарайю.
    // Вместо транспонирования графа применяем обращение к транспонированной матрице смежности.
    template <class G, class C> class CC_T<G, C,
        typename enable_if<is_same<G, DenseGraph_T<C>>::value &&
        is_base_of<DirectedGraphTraits, C>::value>::type> {
        
        using Vec = matrix<bool>::vec;
        size_t cnt;
        size_t scnt;
        vector<size_t> ids, leave;
		
		template<typename G::AdjMethod adjMethod>
        void dfs_( const G& g, size_t v) {
            ids[v] = scnt;
            for( size_t w : (g.*adjMethod)(v) )
                if(ids[w] == -1) dfs_<adjMethod>(g, w);
            leave[cnt++] = v;
        }
		
        friend void SCTrace<CC_T>(ostream&, const CC_T&);
        
    public:
        CC_T( const G& g) : cnt(0), scnt(0), ids(g.size(), -1), leave(g.size()) { trace("CC_T Kosaraju adjmatrix");
			// Делаем "топсорт" на обращении графа.
            for ( size_t v = 0; v < g.size(); v++ )
				if( ids[v] == -1) dfs_<&G::adjacentTranspond>(g, v);
			
            ids.assign(g.size(), -1);
            vector<size_t> order = leave;
            cnt = scnt = 0;
			
			// Проходим dfs-ом по вершинам в топологическом порядке.
			for ( size_t v = g.size() - 1; v < -1; v-- ) {
				size_t next = order[v];
                if ( ids[next] == -1) {
					dfs_<&G::adjacent>(g, next);
                    scnt++;
                }
			}
        }
        
        size_t size() const { return scnt; }
		size_t id(size_t v) const { return ids[v]; }
        bool connected( size_t v, size_t w ) const { return ids[v] == ids[w]; }
    };
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Сильные компоненты, алгоритм Тарьяна (Седжвик 19.11).
    template <class G, class C = typename G::Traits> class SCTar_T {
        const G& g;
        size_t cnt; // топологический счетчик посещения вершин.
        size_t scnt; // количество сильных компонент.
        // st - стек вершин сильной компоненты.
        stack<size_t> st;
        // enter[v] - топологический номер вершины. (назначаемый при обходе в глубину).
        // low[v] - минимальный топологический номер вершины, достижимый из v.
        // ids[v] - номер сильной компоненты, в которой находится v.
        vector<size_t> enter, low, ids;
        
        friend void SCTrace<SCTar_T>(ostream&, const SCTar_T&);
        void dfs_ (size_t v) {
            size_t min = enter[v] = low[v] = cnt++;
            st.push(v);
            for (size_t w : g.adjacent(v)) {
                if ( enter[w] == -1 ) dfs_(w);
                if ( low[w] < min ) min = low[w];
            }
			if ( min < low[v]) {
				low[v] = min;
			} else { size_t w;
                do {
                    ids[ w = st.top() ] = scnt; st.pop();
                    low[w] = -1; // очень большое положительное число.
                } while( v != w );
                scnt++;
            }
        }
        
    public:
        SCTar_T( const G& g ) : g(g), cnt(0), scnt(0), enter(g.size(), -1), low(g.size()), ids(g.size()) { trace("SCTar_T");
            for( size_t v = 0; v < g.size(); v++ )
                if( enter[v] == -1 ) dfs_(v);
        }
        
        size_t size() const { return scnt; }
		size_t id(size_t v) const { return ids[v]; }
        bool connected( size_t v, size_t w ) const { return ids[v] == ids[w]; }
    };
    
    // Ускоритель вызова.
    template<class G> SCTar_T<G> SCTar(const G& g) { return SCTar_T<G>(g); }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Сильные компоненты (СК), алгоритм Габова. 1999 (Седжвик 19.12).
    // Идея - сжатие циклов.
    template <class G, class C = typename G::Traits> class SCGab_T {
        const G& g;
        size_t cnt; // топологический счетчик посещения вершин.
        size_t scnt; // количество СК.
        // st - стек вершин СК.
        // path - стек обхода вершин в глубину до первого обратного ребра.
        stack<size_t> st, path;
        // enter[v] - топологический номер вершины. (назначаемый при обходе в глубину).
        // ids[v] - номер СК, в которой находится v.
        vector<size_t> enter, ids;
        
        friend void SCTrace<SCGab_T>(ostream&, const SCGab_T&);
        void dfs_ (size_t v) {
            enter[v] = cnt++;
            st.push(v);
            path.push(v);
            for ( auto w : g.adjacent(v) ) {
				if ( enter[w] == -1 ) {
                    // Вершина остовного дерева (tree).
					dfs_(w);
				} else if ( ids[w] == -1 ) {
                    // Обнаружен цикл СК. Сжимаем его.
                    // Если попали не в древесную вершину и непринадлежащую ни одной СК, то откатываемся по пути до момента входа в нее.
					while ( enter[path.top()] > enter[w] ) {
						path.pop();
					}
				}
            }
            if ( path.top() == v ) {
                // Нашли сильную компоненту.
                path.pop();
                size_t w;
                do {
					w = st.top();
                    ids[w] = scnt;
                    st.pop();
                } while ( w != v );
                scnt++;
            }
        }
        
    public:
        SCGab_T( const G& g ) : g(g), cnt(0), scnt(0), enter(g.size(), -1), ids(g.size(), -1) { trace("SCGab_T");
            for ( size_t v = 0; v < g.size(); v++ )
                if ( enter[v] == -1 ) dfs_(v);
        }
        
        size_t size() const { return scnt; }
		size_t id(size_t v) const { return ids[v]; }
        bool connected( size_t v, size_t w ) const { return ids[v] == ids[w]; }
    };
    
    // Ускоритель вызова.
    template<class G> SCGab_T<G> SCGab(const G& g) { return SCGab_T<G>(g); }
}

#endif /* strongComponents_h */

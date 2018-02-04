//
//  searchGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 11.04.16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef search_h
#define search_h

#include "debug.h"
#include <cassert>
#include <iostream>
#include <string>
#include <limits>
#include <deque>
#include <stack>
#include <queue>

namespace Graph {
	
    // Визуализатор поиска.
    enum EdgeRole {
        Tree,
        Back,
        Forward,
        Cross
    };
    
    template<typename G, typename Enable = void>
    class SearchTrace_T {
        vector<size_t> p; // родители вершин в остовном лесе. -1 - корень дерева.
        
        const char* etStr(EdgeRole er) const {
            switch( er ) {
                case Tree:
                    return " tree";
                case Back:
                    return " back";
                case Forward:
                    return " forward";
                default:
                    assert(er == Cross);
                    break;
            };
            return " cross";
        }
        
    public:
        SearchTrace_T( const G& g) : p(g.size(), -1) {}
        
        void visit(typename G::Traits::EdgeType e, size_t depth, EdgeRole er) {
            size_t v(e.v), w(e.w);
            cout << string(depth * 3, ' ') << "[" << v << ", " << w << "]" << etStr(er) << endl;
        }
        
        void visit(typename GraphTraits::EdgeType e) {
            size_t v(e.v), w(e.w);
            if ( p[w] == -1 ) p[w] = v;
            for( size_t vp = p[v]; vp != -1; vp = p[vp] ) cout << string(3, ' ');
            cout << "[" << v << ", " << w << "]" << endl;
        }
        
        void visit (size_t v) {
            cout << v << endl;
        }
        
        void reset() { p.assign(p.size(), -1); }
    };
    // Ускоритель вызова
    template <class G>
    SearchTrace_T<G> searchTrace( const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////
    // Обход графа. Параметризуется методом из нижеприведенных классов:
    template <class G, class Method>
    void traverse(G& g, Method& m) {
        vector<bool> c(g.size()); // цвет вершины. 0 - не посещали, 1 - посещали.
        for( size_t v = 0; v < g.size(); v++ ) {
            if( !c[v] ) if(!m(v, c)) break;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // BFS Method for traverse. Обход в ширину.
	template <class G, class Inspector, class Context = typename G::Traits> class BFS_T {
        const G& g;
        Inspector& i;
        queue<size_t> q; // очередь просмотра вершин.
    public:
        BFS_T( const G& g, Inspector& i ) : g(g), i(i) {trace("BFS_T");}
        
        bool operator() (size_t v, vector<bool>& c ) {
            q.push(v);
            while( !q.empty() ) {
                c[v] = true;
                v = q.front(); q.pop();
                for( auto w : g.adjacent(v) ) {
                    
                    if(c[w] == false) {
                        c[w] = true;
                        q.push(w);
						i.visit( {v, w} );
                    }
                }
            }
            return true;
        }
    };
    
    // Ускоритель вызова.
    template <class G, class Inspector>
    BFS_T<G, Inspector> BFS( const G& g, Inspector& i) { return BFS_T<G, Inspector>(g, i); }

    
    ////////////////////////////////////////////////////////////////
    // DFS Method for traverse. Обход в глубину.
	template <class G, class Inspector, class Enable = void> class DFS_T {
        const G& g;
        Inspector& i;
    public:
        DFS_T( const G& g, Inspector& i ) : g(g), i(i) { trace("DFS_T undirected"); }
        
        bool operator() (size_t v, vector<bool>& c ) {
            c[v] = true;
            for( auto w : g.adjacent(v)) {
                if(c[w] == false) {
					i.visit( {v, w} );
                    (*this)(w, c);
                }
            }
            return true;
        }
    };
    
    // Ускоритель вызова.
    template <class G, class Inspector>
    DFS_T<G, Inspector> DFS( const G& g, Inspector& i ) { return DFS_T<G, Inspector>(g, i); }
    
    /////////////////////////////////////////////////////////////////
    // Connected Components. Связность.
	
	// Визуализатор связных компонент алгоритма SC.
	template <class SC> void SCTrace( ostream& os, const SC& sc ) {
		os << sc.scnt << " strong components\n";
		vector<vector<size_t>> cc(sc.scnt);
		for( size_t v = 0; v < sc.cnt; v++ ) {
			cc[sc.ids[v]].push_back(v);
		}
		for( size_t i = 0; i < cc.size(); i++ ) {
			for ( size_t j = 0; j < cc[i].size(); j++ )
				os << cc[i][j] << ", ";
			os << endl;
		}
	};

	// Базовый алгоритм для неориентированных графов.
    template <class G, class Context = typename G::Traits, class Enable = void> class CC_T {
        const G& g;
		size_t cnt;
        size_t scnt;
        vector<size_t> ids;
        
        void ccR(size_t v) {
            ids[v] = scnt;
            for( size_t w : g.adjacent(v) ) {
                if(ids[w] == -1) ccR(w);
            }
        }
        
        void dfsR (size_t v) {
            ccR(v);
            scnt++;
        }
		
		friend void SCTrace<CC_T>(ostream&, const CC_T&);

    public:
        CC_T( const G& g) : g(g), cnt(g.size()), ids(g.size(), -1), scnt(0) { trace("CC_T undirected");
            for ( size_t v = 0; v < g.size(); v++ )
                if( ids[v] == -1) dfsR(v);
        }
        
        size_t size() const { return scnt; }
		size_t id(size_t v) const { return ids[v]; }
        bool connected( size_t v, size_t w ) const { return ids[v] == ids[w]; }
    };

    // Ускоритель вызова.
    template <class G>
    CC_T<G> CC(const G& g) { return CC_T<G>(g); }
	
	/////////////////////////////////////////////////////////////////
	// Bipartite graph. Method for traverse. - Двудольность.
	template <class G> class BI_T {
		const G& g;
		bool ok;
		vector<bool> vc; // Вектор цветов вершин.
	public:
		BI_T(const G& g) : g(g), ok(false), vc(g.size()) { trace("BI_T"); }
		
		bool bipR(size_t v, vector<bool>& c, bool color) {
			c[v] = true;
			vc[v] = !color;
			for( size_t w : g.adjacent(v)) {
				if(c[w] == false) {
					if( !bipR(w, c, !color) ) return false;
				} else if ( vc[w] != color ) return false;
			}
			return true;
		}
		
		bool operator() (size_t v, vector<bool>& c ) {
			ok = bipR(v , c, false);
			return ok;
		}
		
		bool bipartite() const { return ok; }
		bool color(size_t v) const { return vc[v]; }
	};

	// Ускоритель вызова.
	template <class G>
	BI_T<G> BI(const G& g) { return BI_T<G>(g); }
    
    /////////////////////////////////////////////////////////////////
    // Bridges. Поиск мостов. На основе Седжвик 18.7.
    // O(V^2) для матрицы смежности, O(V+E) для списка смежности. 
    template <class G, class Inspector> class Bridges_T {
        const G& g;
        Inspector& i;
        size_t cnt; // счетчик порядка обхода.
        vector<size_t> enter; // порядок обхода по dfs.
        vector<size_t> low; // нижний номер enter достижимости по dfs c обратной связью из вершины v.
        using Edge = typename G::Edge;
        
        // Реберный DFS. Для корректной работы мы всегда должны знать из кокой вершины мы пришли.
        bool dfs(Edge e) {
            if(enter[e.w] == 0) {
                enter[e.w] = ++cnt;
                low[e.w] = cnt;
                bridges(e);
                return true;
            }
            // вершина e.w уже была посещена.
            return false;
        }
        
        void bridges(Edge e) {
            size_t v = e.w;
            for(size_t w : g.adjacent(v)) {
                if (w == e.v) {
                    continue; // Вот ради этой проверки нам нужен реберный DFS.
                }
                if (dfs(Edge(v, w))) {
                    if (low[w] < low[v]) {
                        low[v] = low[w];
                    }
                    if (low[w] == enter[w]) {
                        i.visit(Edge(v,w)); // обнаружен мост.
                    }
                } else if (low[v] > enter[w]) {
                    low[v] = enter[w]; // обнаружена обратная связь.
                }
            }
        }

    public:
        Bridges_T(const G& g, Inspector& i) : g(g), i(i), cnt(0), enter(g.size()), low(g.size()) {
            trace("Bridges_T");
            dfs(Edge(-1, 0)); // -1 означает - идем из ниоткуда. 0 можно ставить любой вершиной графа.
        }
    };
    
    // Ускоритель вызова.
    template <class G, class I>
    Bridges_T<G,I> Bridges(const G& g, I& i) { return Bridges_T<G,I>(g,i); }
    
    /////////////////////////////////////////////////////////////////
    // ArticulationPont. Поиск шарниров. На основе Седжвик 18.7.
    // O(V^2) для матрицы смежности, O(V+E) для списка смежности.
    template <class G, class Inspector> class ArtPoint_T {
        const G& g;
        Inspector& i;
        size_t cnt; // счетчик порядка обхода.
        vector<size_t> enter; // порядок обхода по dfs.
        vector<size_t> low; // нижний номер enter достижимости по dfs c обратной связью из вершины v.
        size_t root;
        
        // DFS.
        bool dfs(size_t v) {
            if(enter[v] == 0) {
                // Первый вход в вершину v.
                enter[v] = ++cnt;
                low[v] = cnt;
                artPoints(v);
                return true;
            }
            // вершина v уже была посещена.
            return false;
        }
        
        void artPoints(size_t v) {
            size_t children = 0; // потомки в которые мы заходим в dfs из этого узла.
            for(size_t w : g.adjacent(v)) {
                if (dfs(w)) {
                    children++;
                    // Если наш enter == low детей то мы - шарнир.
                    if (low[w] < low[v]) {
                        low[v] = low[w];
                    }
                    // Условие для не-корня. В принципе оно подходит и для корня, но тогда корень выведется 2 раза.
                    if (v != root && enter[v] == low[w]) {
                        i.visit(v);
                    }
                } else if (low[v] > enter[w]) {
                    low[v] = enter[w]; // обнаружена обратная связь.
                }
            }
            // Условие для корня.
            if (v == root && children > 1) {
                i.visit(v);
            }
        }
        
    public:
        ArtPoint_T(const G& g, Inspector& i) : g(g), i(i), cnt(0), enter(g.size()), low(g.size()) {
            trace("ArtPoint_T");
            root = 0; // 0 можно ставить любой вершиной графа.
            dfs(root);
        }
    };
    
    // Ускоритель вызова.
    template <class G, class I>
    ArtPoint_T<G,I> ArtPoints(const G& g, I& i) { return ArtPoint_T<G,I>(g,i); }
    
    ///////////////////////////////////////////////////////////////////////////
    // Вычисление количества кратчайших путей между вершинами А и В. O(V+E)
    // Считаем количество заходов в вершины по кратчайшим путям.
    template <class Graph>
    class CountBFS_T {
        const Graph& g;
        queue<size_t> q; // очередь просмотра вершин.
        struct VertexState {
            size_t count;
            bool leave;
            VertexState() : count(0), leave(false) {}
            friend ostream& operator<<(ostream& os, const VertexState& v) {
                os << v.count << ":" << v.leave;
                return os;
            };
        };
        vector<VertexState> state; // вектор количества захода в i-ю вершину.
        vector<size_t> currentDepth;
    public:
        CountBFS_T(const Graph& g) : g(g), state(g.size()), currentDepth(g.size()) {}
        
        size_t solve (size_t a, size_t b) {
            state.assign(g.size(), VertexState());
            currentDepth.clear();
            q.push(a);
            state[a].count = 1;
            state[a].leave = true;
            size_t currentCount = 0;
            size_t nextCount = 0;
            
            while( !q.empty() ) {
                a = q.front(); q.pop();
                size_t count = state[a].count;
                if (a == b) {
                    // Дошли до B
                    return count;
                }
                for( size_t i : g.adjacent(a) ) {
                    VertexState& vs = state[i];
                    if (vs.leave) continue; // Вершина принадлежит предыдущим ребрам пути.
                    if(vs.count == 0) {
                        // Первый заход в i.
                        q.push(i);
                        nextCount++;
                    }
                    vs.count += count;
                    currentDepth.push_back(i);
                }
                if (currentCount == 0) {
                    currentCount = nextCount - 1;
                    nextCount = 0;
                    for (auto v : currentDepth) {
                        state[v].leave = true;
                    }
                    currentDepth.clear();
                    // cout << state;
                } else {
                    currentCount--;
                }
            }
            return 0;
        }
    };
    
    // Ускоритель вызова.
    template <class G>
    CountBFS_T<G> countBFS(const G& g) { return CountBFS_T<G>(g); }

}

#endif /* search_h */

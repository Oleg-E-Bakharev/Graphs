//
//  smartGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 13/02/2018.
//  Copyright © 2018 Oleg Bakharev. All rights reserved.
//

// Граф на разреженной матрице. Память O(E). Вставка/удаление  O(1). Проверка ребра О(1).
// Итерация по рёбрам O(E).

#ifndef smartGraph_h
#define smartGraph_h

#include "graphBase.h"
#include "sparceMatrix.hpp"

/*
 Нужно в матрице кэшировать начальную позицию списка смежности и проходить последовательно по элементам пока они попадают
 под slice-итератор строки матрицы.
 */

// Специализация for_iter_t для итерации по смежным вершинам графа, заданного разреженной матрицей.
// Для контекстов, WeightType которых есть bool.
// Т - контенер. C - контекст.
template <class T, class C> class for_iter_t <T, C,
typename enable_if<
        is_same<typename T
        is_same<typename C::WeightType, bool>::value
    >::type>
{
    T& t;
    size_t pos;
public:
    for_iter_t(T& t) : t(t), pos(0) { if(!t[pos]) ++*this; }
    size_t operator*() { return pos; }
    bool operator != (const for_iter_t& f) const { assert(&f.t == &t); return pos != t.size(); }
    void operator++() { while(++pos < t.size() && t[pos] == false); }
};


namespace Graph {

    // Итератор по смежным вершинам графа
    template<typename GT> class GraphIter {
    public:
        GraphIter(size_t v)
    };
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    // Граф на разреженой матрице смежности.
    template<typename GT = GraphTraits>
    class Graph_T {
        using AdjMatrix = SparseMatrix<typename GT::WeightType, GT>;
        AdjMatrix _adj;
        size_t _edges = 0;
        // Вектор первой вершины смежности для каждой вершины.
        std::vector<size_t> _firstAdj;
        
    public:
        using Traits = GT;
        using Edge = typename Traits::EdgeType;
        using reference = typename AdjMatrix::reference::reference;
        using WeightType = typename Traits::WeightType;
        using NodeType = typename GT::AdjListNodeType;
        
        Graph_T(size_t v) : _adj(v, v) {}
        
        // Конструктор от другого графа c подходящими свосйствами.
        template<class G> Graph_T ( const G& g,
                                        typename enable_if<is_base_of<GT, typename G::Traits>::value>::type* = nullptr ) : _adj(g.size(), g.size())
        {
            for ( size_t v = 0; v < g.size(); v++ ) {
                for(auto& w : g.adjacent(v)) {
                    
                }
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
        
        void reweight(size_t v, const NodeType& node, WeightType weight) {
            _adj[v][node.dest] = weight;
        }
    };
    
    using Graph = Graph_T<GraphTraits>;
    using GraphD = Graph_T<DirectedGraphTraits>;
    using DAG = Graph_T<DAGTraits>;
}


#endif /* smartGraph_h */

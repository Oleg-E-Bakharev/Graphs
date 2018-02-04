//
//  mst.h minimal spanning tree - минимальное остовное дерево взвешенного графа
//  Graphs
//
//  Created by Oleg Bakharev on 07/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#ifndef mst_h
#define mst_h

#include "weightedGraph.h"
#include <set>
#include <vector>
#include <assert.h>
#include <iostream>
#include "disjointSet.h"

namespace Graph {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм Прима построения Минимального Остовного Дерева (МОД)
    // Minimal Spanning Tree (MST) Седжвик 20.3 O(E*lg(V))
    template<typename G> class MstPrim_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        using Node = typename Traits::AdjListNodeType;
        
        const G& _g;
        std::vector<bool> _used; // пройденные вершины.
        // истоки MST (родители MST). В этом векторе:
        // node.dest есть наоборот - исток ребра минимального пути.
        // node.weight вес пути до индексной точки.
        std::vector<Node> _mst;
        
        // Priority-first search.
        void pfs_(size_t v) {
            
            std::set<Node, WeightLess<Node>> _nodesQueue; // возрастающая очередь узлов(конечная вершина ребра + вес) с приоритетом веса.
            _nodesQueue.insert({v, 0.}); // Установим в начальную точку путь нулевого веса.

            while (_nodesQueue.size() > 0) {
                // Извлекаем узел (ребро) с минимальным весом.
                auto minPos = _nodesQueue.begin();
                Node next = *minPos;
                _nodesQueue.erase(minPos);
                v = next.dest;
                
                _used[v] = true;
                // Добавляем смежные ребра в множество ребер-кандидатов в MST.
                for (auto node : _g.adjacent(v)) {
                    if (_used[node.dest]) {
                        continue;
                    }
                    Node& prevNode = _mst[node.dest];
                    Weight weight = node.weight;
                    
                    // Если в вершину зашли с минимальным ребром. Обновляем MST.
                    if (weight < prevNode.weight) {
                        if (prevNode.dest != -1) {
                            // В вершину зашли не впервые и значит в очереди есть ребро с большим весом - удаляем его.
                            size_t cnt = _nodesQueue.erase({node.dest, prevNode.weight}); // lg(E)
                            assert(cnt == 1);
                        }
                        // Корректируем mst.
                        prevNode = {v, weight};
                        
                        _nodesQueue.insert(node); // Добавляем минимальный узел в ребер. lg(E) ;
                    }
                }
            }
        }
        
    public:
        MstPrim_T(const G& g) : _g(g), _used(g.size(), false),
            _mst(g.size(), {size_t(-1), std::numeric_limits<double>::max()})
        {
            for (size_t v = 0; v < g.size(); v++) {
                if (!_used[v]) {
                    pfs_(v);
                }
            }
        }
        
        std::vector<Edge> mst() const {
            std::vector<Edge> mst;
            for (size_t w = 0; w < _mst.size(); w++) {
                const Node& n = _mst[w];
                if (n.dest != -1) {
                    // Выставляем правильное направление ребра (актуально для направленных графов).
                    mst.push_back({n.dest, {w, n.weight}});
                }
            }
            return mst;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const MstPrim_T& m) {
            auto mst = m.mst();
            os << "MstPrim\n";
            for (const auto& e : mst) {
                os << e << "\n";
            }
            os << "\n";
            return os;
        }
    };
    
    // Ускоритель вызова
    template<typename G> MstPrim_T<G> mstPrim(const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм Крускала построения Минимального Остовного Дерева (МОД)
    // Minimal Spanning Tree (MST) Седжвик 20.4 O(E*lg(E))
    template<typename G> class MstKrus_T {
        using Edge = typename G::Traits::EdgeType;
        
        const G& _g;
        std::vector<Edge> _mst; // MST.
        
    public:
        MstKrus_T(const G& g) : _g(g)
        {
            size_t edgesCount = g.edgesCount();
            DisjointSet cc(g.size()); // Лес компонент связности.
			
			// Накопитель ребер.
            std::vector<Edge> storage(edges(g));
            _mst.reserve(edgesCount);

            // Сортируем ребра по возрастания веса.
            std::sort(storage.begin(), storage.end(), WeightLess<Edge>());
            
            for (const Edge& e : storage) {
                // Если вершины ребра не образуют цикл, добавляем ребро в mst.
                if (cc.uniteIfNotConnected( e.v, e.w)) {
                    _mst.push_back(e);
                }
            }
        }
        
        std::vector<Edge> mst() const {
            return _mst;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const MstKrus_T& m) {
            auto mst = m.mst();
            os << "MstKrus\n";
            for (const auto& e : mst) {
                os << e << "\n";
            }
            os << "\n";
            return os;
        }
    };
    
    // Ускоритель вызова
    template<typename G> MstKrus_T<G> mstKrus(const G& g) { return {g}; }
}

#endif /* mst_h */

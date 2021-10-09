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
        std::vector<Edge> _mstFinal;
        
        void calcMstFinal_() {
            // Превращаем _mst<Node> в _mstFinal<Edge> O(V)
            _mstFinal.reserve(_mst.size());
            for (size_t w = 0; w < _mst.size(); w++) {
                const Node& n = _mst[w];
                if (n.dest != -1) {
                    // Выставляем правильное направление ребра (актуально для направленных графов).
                    _mstFinal.push_back({n.dest, {w, n.weight}});
                }
            }
            _mst.clear();
        }
        
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
            
            calcMstFinal_();
        }
        
    public:
        MstPrim_T(const G& g) : _g(g), _used(g.size(), false),
            _mst(g.size(), {size_t(-1), std::numeric_limits<double>::max()})
        {
            for (size_t v = 0; v < g.size(); v++) { // O((V+E)lg(V))
                if (!_used[v]) {
                    pfs_(v);
                }
            }
        }
        
        const std::vector<Edge>& mst() const {
            return _mstFinal;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const MstPrim_T& m) {
            auto mst = m.mst();
            os << "\nMstPrim\n";
            Weight wt = 0;
            for (const auto& e : mst) {
                os << e << "\n";
                wt += e.weight;
            }
            return os << "MST weight: " << wt << "\n";
        }
    };
    
    // Ускоритель вызова
    template<typename G> MstPrim_T<G> mstPrim(const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм Крускала построения Минимального Остовного Дерева (МОД)
    // Minimal Spanning Tree (MST) Седжвик 20.4 O(E+X*lg(V))
    // X - количество ребер не превосходящих по длине самое длинное ребро MST.
    // lg(V) ~ lg(E) потому что maxE = V^2 => lg(maxE) = 2*lg(V).
    template<typename G> class MstKrus_T {
        using Edge = typename G::Traits::EdgeType;
        using Weight = typename G::Traits::WeightType;
        
        const G& _g;
        std::vector<Edge> _mst; // MST.
        
    public:
        MstKrus_T(const G& g) : _g(g)
        {
            DisjointSet cc(g.size()); // Лес компонент связности.
			
			// Накопитель ребер.
            std::vector<Edge> storage(edges(g)); // O(E)
            _mst.reserve(g.size());

            // Строим кучу из рёбер.
            std::make_heap(storage.begin(), storage.end(), WeightGreater<Edge>()); // O(E)
            
            for (size_t i = 0; i <= g.size(); i++) {
                pop_heap(storage.begin(), storage.end(), WeightGreater<Edge>()); // O(lgE)
                const Edge& e = storage.back();
                // Если вершины ребра не образуют цикл, добавляем ребро в mst.
                if (cc.uniteIfNotConnected( e.v, e.w)) { // O(1)
                    _mst.push_back(e);
                }
                storage.pop_back();
            }
        }
        
        const std::vector<Edge>& mst() const {
            return _mst;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const MstKrus_T& m) {
            auto mst = m.mst();
            os << "\nMstKruscal\n";
            Weight wt = 0;
            for (const auto& e : mst) {
                os << e << "\n";
                wt += e.weight;
            }
            return os << "MST weight: " << wt << "\n";
        }
    };
    
    // Ускоритель вызова
    template<typename G> MstKrus_T<G> mstKrus(const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм Борувки построения Минимального Остовного Дерева (МОД)
    // Minimal Spanning Tree (MST) Седжвик 20.5 O(E*lg(V))
    /*
     Изначально, пусть T — пустое множество рёбер MST (представляющее собой остовный лес, в который каждое ребро входит в
     качестве отдельного дерева).
     Для каждой компоненты связности (то есть, дерева в остовном лесе) в подграфе с рёбрами T, найдём самое дешёвое
     ребро, связывающее эту компоненту с некоторой другой компонентой связности. (Предполагается, что веса рёбер
     различны, или как-то дополнительно упорядочены так, чтобы всегда можно было найти единственное ребро с минимальным
     весом).
     Добавим все найденные рёбра в множество T.
    */
    template<typename G> class MstBoruvka_T {
        using Edge = typename G::Traits::EdgeType;
        using Weight = typename G::Traits::WeightType;
        
        const G& _g;
        std::vector<Edge> _mst; // MST.
        
    public:
        MstBoruvka_T(const G& g) : _g(g)
        {
            _mst.reserve(g.size());
            const std::vector<Edge> edges(::Graph::edges(g)); // O(E)
            
            // Индексы рёбер, еще не отвергнутых и не включённых в MST.
            std::vector<size_t> active;
            active.reserve(edges.size());
            for (size_t i = 0; i < edges.size(); i++) active.push_back(i);
            
            std::vector<size_t> nearest; // Ближайший сосед к компоненте, указанной индексом.
            DisjointSet cc(g.size()); // Лес компонент связности.
            
            // O(lg(E)) потому что на каждой следующей итерации |edges| уменьшается вдвое.
            for	(size_t i = active.size(), next = 0; i != 0; i = next) {
                next = 0;
                // 1. Строим вектор ближайших соседей.
                nearest.assign(active.size(), 0);
                for (size_t j = 0; j < i; j++) {
                    size_t k = active[j]; // индекс текущего ребра.
                    const Edge& e = edges[k];
                    
                    // Сравниваем компоненты связности
                    size_t ccV = cc.find(e.v); // O(1)
                    size_t ccW = cc.find(e.w);
                    if (ccV == ccW) {
                        continue; // За счёт этого |edges| уменьшается вдвое на каждом шаге по i.
                    }
                    if (!nearest[ccV] || e.weight < edges[nearest[ccV]].weight ) nearest[ccV] = k;
                    if (!nearest[ccW] || e.weight < edges[nearest[ccW]].weight ) nearest[ccW] = k;
                    active[next++] = k;
                }
                
                // 2. Добавляем ближайших соседей в MST.
                for (size_t j = 0; j < next; j++) {
                    if (nearest[j] != 0) {
                        const Edge& e = edges[nearest[j]];
                        if (cc.uniteIfNotConnected(e.v, e.w)) { // O(1)
                            _mst.push_back(e);
                        }
                    }
                }
            }
        }
        
        const std::vector<Edge>& mst() const {
            return _mst;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const MstBoruvka_T& m) {
            auto mst = m.mst();
            os << "\nMstBoruvka\n";
            Weight wt = 0;
            for (const auto& e : mst) {
                os << e << "\n";
                wt += e.weight;
            }
            return os << "MST weight: " << wt << "\n";
        }
    };
    
    // Ускоритель вызова
    template<typename G> MstBoruvka_T<G> mstBoruvka(const G& g) { return {g}; }
}

#endif /* mst_h */

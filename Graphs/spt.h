//
//  spt.h shortest paths tree - дерево наикратчайших путей взвешенного графа.
//  Graphs
//
//  Created by Oleg Bakharev on 10/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#ifndef shortestPath_h
#define shortestPath_h

#include <vector>
#include <set>
#include <limits>
#include <algorithm>
#include <queue>
#include <assert.h>

namespace Graph {
    // Алгоритм поиска кратчайших путей Дейкстры. Седжвик 21.1 O(E*lg(V))
    // Также реализует функцию поиска наиболее удаленной вершины для решения задачи вычисления диаметра Графа.
    template<typename G> class SptDijkstra_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        using Node = typename Traits::AdjListNodeType;
        
        const G& _g;
        // истоки SPT (родители SPT). В этом векторе:
        // node.dest есть наоборот - исток ребра минимального пути.
        // node.weight накопденный минимальный вес пути до индексной точки.
        std::vector<Node> _spt;
        
        // Точка останова. Если не была указана, то после вычислений будет указывать на максимально удаленную мершину от исходной.
        size_t _finish;
        
        // Priority-first search.
        void pfs_(size_t v) {
            
            _spt[v] = {size_t(-1), 0.}; // Начальному элементу ставим 0-й вес.
            std::priority_queue<Node, std::vector<Node>, WeightGreater<Node>> _nodesQueue;
            _nodesQueue.push({v, 0.}); // Установим в начальную точку путь нулевого веса.
            
            size_t farest = v; // Наиболее удаленная вершина от v.
            
            // Применение вектора посещенности позволяет избегать удаления неоптимальных ребер из очереди и
            // зацикливания на отрицательных циклах.
            vector<bool> used(_g.size());
            
            while (!_nodesQueue.empty()) {
                Node from = _nodesQueue.top();
                _nodesQueue.pop();
                size_t v = from.dest;
                
                if (used[v]) {
                    continue;
                }
                used[v] = true;
                
                if (v == _finish) {
                    return;
                }
                
                // Добавляем смежные ребра в множество ребер-кандидатов в MST.
                for (auto node : _g.adjacent(v)) {
                    Weight& weight = node.weight;
                    weight += from.weight;
					
                    Node& prevNode = _spt[node.dest];
                    
                    // Если в вершину зашли с минимальным ребром. Обновляем MST.
                    if ( weight < prevNode.weight ) {
                        // Корректируем mst.
                        prevNode = {v, weight};
						//  Занесение нового ребра не изменит логику выборки, поскольку новое ребро
						// встанет в очередь перед существущим.
                        _nodesQueue.push(node); // lg(v).
                        
                        // Вычисляем наиболее удаленную вершину.
                        if (weight > _spt[farest].weight) {
                            farest = node.dest;
                        }
                    }
                }
            }
            
            assert(_finish == -1 || _finish == farest);
            _finish = farest;
        }
        
    public:
        // Из точки А в точку B.
        SptDijkstra_T(const G& g, size_t a, size_t b = -1) : _g(g), _finish(b),
        _spt(g.size(), {size_t(-1), std::numeric_limits<double>::max()}) {
            pfs_(a);
        }
        
        // Целевая вершина. Если в конструкторе не была указана, то вернет наиболее удаленную вершину.
        size_t dest() const { return _finish; }
        
        // Исток последнего ребра пути.
        size_t source(size_t v) const { return _spt[v].dest; }
        // Вес (расстояние) кратчайшего пути.
        // Если конечная точка не указана - будет возвращен вес (расстояние) кратчайшего пути в указанную в конструкторе или наиболее удаленную вершину от исходной.
        // Иначе будет возвращен путь будет в указанную вершину. При этом, если в конструкторе была указана конечная вершина, то результат может быть недостоверен.
        Weight distance(size_t dest = -1) const {
            if (dest == -1) {
                dest = _finish;
            }
            return _spt[dest].weight;
        }

        // Формирование вектора кратчайшего пути.
        // Если конечная точка не указана - будет сформерован вектор кратчайшего пути в указанную в конструкторе или наиболее удаленную вершину от исходной.
        // Иначе путь будет в указанную вершину. При этом, если в конструкторе была указана конечная вершина, то результат может быть недостоверен.
        std::vector<Edge> spt(size_t dest = -1) const {
            std::vector<Edge> spt;

            if (dest == -1) {
                dest = _finish;
            }
            for(Node n = _spt[dest]; n.dest != -1; n =_spt[n.dest]) {
                // Выставляем правильное направление ребра (актуально для направленных графов).
                spt.push_back({n.dest, {dest, n.weight}});
                dest = n.dest;
            }
            reverse(spt.begin(), spt.end());
            return spt;
        }
        
        // Вывод MST.
        friend std::ostream& operator<<(std::ostream& os, const SptDijkstra_T& s) {
            auto spt = s.spt();
            os << "SptDijkstra\n";
            for (auto e : spt) {
                os << e << "\n";
            }
            os << "\n";
            return os;
        }
    };
    
    // Ускоритель вызова
    template<typename G> SptDijkstra_T<G> sptDijkstra(const G& g, size_t a, size_t b = -1) { return {g, a, b}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм поиска кратчайших путей (SPT) Бэллмана-Форда - наивная реализация.
    template<typename G> class SptBFNaive_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        const Weight INF = std::numeric_limits<double>::max();
        
        const G& _g;
        vector<size_t> _parent; // Истоки ребер кратчайшего пути (родители SPT).
        vector<Weight> _distance; // Расстояние от исходной вершины до i-й.
        
        void tryRelax_(const Edge& e) {
            if (_distance[e.w] > _distance[e.v] + e.weight) {
                _distance[e.w] = _distance[e.v] + e.weight;
                _parent[e.w] = e.v; // Обновляем кратчайшее ребро.
            }
        }
        
    public:
        SptBFNaive_T(const G& g, size_t v) : _g(g), _parent(g.size(), -1), _distance(g.size(), INF)
        {
			// Список ребер.
			// Для неориентированного графа - нам нужны и обратные ребра тоже!
            std::vector<Edge> storage(edges(g, false));
            _distance[v] = 0.;
            
            for (size_t i = 0; i < g.size(); i++) {
                for (const Edge& e : storage) {
					tryRelax_(e);
                }
            }
        }
        
        // исток последнего ребра и вес пути в v.
        size_t source(size_t v) const { return _parent[v]; }
        Weight distance(size_t v) const { return _distance[v]; }
        
        // Формирование вектора кратчайшего пути.
        std::vector<Edge> spt() const {
            std::vector<Edge> spt;
                for (size_t v = 0; v < _parent.size(); v++) {
                    if (_parent[v] != -1) {
                        spt.push_back({_parent[v], v, _distance[v]});
                    }
                }
            return spt;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SptBFNaive_T& m) {
            auto spt = m.spt();
            os << "SptBFNaive\n";
            for (const auto& e : spt) {
                os << e << "\n";
            }
            os << "\n";
            return os;
        }
    };
    
    // Ускоритель вызова
    template<typename G> SptBFNaive_T<G> sptBFNaive(const G& g, size_t v) { return {g, v}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм поиска кратчайших путей (SPT) Бэллмана-Форда - продвинутая реализация.
    template<typename G> class SptBFAdvanced_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        const Weight INF = std::numeric_limits<double>::max();
        
        const G& _g;
        vector<size_t> _parent; // Истоки ребер кратчайшего пути (родители SPT).
        vector<Weight> _distance; // Расстояние от исходной вершины до i-й.
    
        void bfs_(size_t v) {
            _distance[v] = 0.;
            
            // Используем очередь всех вершин, для которых может оказаться эффективной релаксация.
            std::queue<size_t> q;
            
            size_t limit = _g.size(); // Разделитель проходов. Позволяет считать проходы и остановиться после g->size проходов.
            size_t count = 0;
            
            q.push(v);
            q.push(limit);
            
            while (!q.empty()) {
                // Извлекаем следующую вершину и проверяем, не равна ли она разделителю.
                v = q.front();
                q.pop();
                
                if (v == limit) {
                    if (q.empty()) {
                        // Кратчайший путь найден.
                        return;
                    }
                    if (++count > limit) {
                        // Прошли максимальное количество проходов + 1.
                        // В графе присутствует отрицательный цикл.
                        _hasNegativeCycle = true;
                        return;
                    }
                    q.push(limit);
                    continue;
                }
                
                assert(v != limit);
                assert(_distance[v] != INF);
                
                for (auto node : _g.adjacent(v)) {
                    if (tryRelax_({v, node})) {
                        // Если релаксация привела к успеху, то заносим вершину в очередь.
                        q.push(node.dest);
                    }
                }
            }
        }
    
        bool tryRelax_(const Edge& e) {
            if (_distance[e.w] > _distance[e.v] + e.weight) {
                _distance[e.w] = _distance[e.v] + e.weight;
                _parent[e.w] = e.v; // Обновляем кратчайшее ребро.
                return true;
            }
            return false;
        }
        
        bool _hasNegativeCycle = false;
        
    public:
        SptBFAdvanced_T(const G& g, size_t v) : _g(g), _parent(g.size(), -1), _distance(g.size(), INF)
        {
            bfs_(v);
        }
        
        bool hasNegativeCycle() const { return _hasNegativeCycle; }
        
        // исток последнего ребра и вес пути в v.
        size_t source(size_t v) const { return _parent[v]; }
        Weight distance(size_t v) const { return _distance[v]; }

        // Формирование вектора кратчайшего пути.
        // Если конечная точка указана - будет сформеровано дерево всех кратчайших путей из начальной точки.
        std::vector<Edge> spt() const {
            std::vector<Edge> spt;
            for (size_t v = 0; v < _parent.size(); v++) {
                if (_parent[v] != -1) {
                    spt.push_back({_parent[v], v, _distance[v]});
                }
            }
            return spt;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SptBFAdvanced_T& m) {
            auto spt = m.spt();
            os << "SptBFAdvanced\n";
            for (const auto& e : spt) {
                os << e << "\n";
            }
            os << "\n";
            return os;
        }
    };
    
    // Ускоритель вызова
    template<typename G> SptBFAdvanced_T<G> sptBFAdvanced(const G& g, size_t v) { return {g, v}; }
    
} // namespace Graph

#endif /* shortestPath_h */

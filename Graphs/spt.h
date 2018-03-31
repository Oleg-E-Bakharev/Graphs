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
        const Weight INF = std::numeric_limits<double>::max();
        
        const G& _g;
        vector<size_t> _parent; // Истоки ребер кратчайшего пути (родители SPT).
        vector<Weight> _distance; // Расстояние от исходной вершины до i-й.

        // Точка останова. Если не была указана, то после вычислений будет указывать на максимально удаленную вершину от исходной.
        size_t _finish;
        
        // Priority-first search.
        void pfs_(size_t s) {
            _distance[s] = 0.; // Установим в начальную точку пути нулевой вес.
            // Очередь вершин по приоритету возрастания веса.
            auto compare = [this](size_t l, size_t r)->bool { return this->_distance[l] > this->_distance[r]; };
            std::priority_queue<size_t, std::vector<size_t>, decltype(compare)> pfsQueue(compare);
            pfsQueue.push(s);
            
            size_t farest = s; // Наиболее удаленная вершина от v.
            
            // Защита от зацикливания на отрицательных циклах.
            vector<bool> used(_g.size());
            
            while (!pfsQueue.empty()) {
                const size_t v = pfsQueue.top(); pfsQueue.pop();
                
                if (v == _finish) return;

                if (used[v]) continue;
                used[v] = true;
                
                const Weight distanceV = _distance[v];
                
                // Добавляем вершины в множество вершин-кандидатов в SPT.
                for (auto node : _g.adjacent(v)) {
                    const size_t w = node.dest;
                    if (w == s) continue; // Все пути в исходную вершину игнорируем чтобы выполнилось правило parent[s] == -1.
                    const Weight distance = distanceV + node.weight;
					
                    // Если в вершину зашли с минимальным ребром. Обновляем SPT.
                    if ( distance < _distance[w] ) {
                        // Корректируем spt.
                        _parent[w] = v;
                        _distance[w] = distance;
                        // Нестрашно повтороное занесение вершины, поскольку выберется вершина один раз с минимальным весом.
                        pfsQueue.push(w); // lg(V).
                        
                        // Вычисляем наиболее удаленную вершину.
                        if (distance > _distance[farest]) {
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
        SptDijkstra_T(const G& g, size_t s, size_t t = -1) : _g(g), _finish(t), _parent(g.size(), -1), _distance(g.size(), INF) {
            pfs_(s);
            assert(_parent[s] == -1);
        }
        
        // Целевая вершина. Если в конструкторе не была указана, то вернет наиболее удаленную вершину.
        size_t dest() const { return _finish; }
        
        // Исток последнего ребра пути.
        size_t source(size_t v) const { return _parent[v]; }
        // Вес (расстояние) кратчайшего пути.
        // Если конечная точка не указана - будет возвращен вес (расстояние) кратчайшего пути в указанную в конструкторе или наиболее удаленную вершину от исходной.
        // Иначе будет возвращен путь будет в указанную вершину. При этом, если в конструкторе была указана конечная вершина, то результат может быть недостоверен.
        Weight distance(size_t dest = -1) const {
            if (dest == -1) dest = _finish;
            return _distance[dest];
        }

        // Формирование вектора кратчайшего пути.
        // Если конечная точка не указана - будет сформерован вектор кратчайшего пути в указанную в конструкторе или наиболее удаленную вершину от исходной.
        // Иначе путь будет в указанную вершину. При этом, если в конструкторе была указана конечная вершина, то результат может быть недостоверен.
        // Поскольку расчёт ведётся только до указанной в конструкторе вершины.
        std::vector<Edge> spt(size_t w = -1) const {
            std::vector<Edge> spt;
            // Мы не сохраняем исходную вершину, но соблюдаем правило что parent[s] == -1.
            if (w == -1) w = _finish;
            for(auto v = _parent[w]; v != -1; w = v, v =_parent[w]) {
                spt.push_back({v, {w, _distance[w]}});
            }
            reverse(spt.begin(), spt.end());
            return spt;
        }
        
        // Вывод SPT.
        friend std::ostream& operator<<(std::ostream& os, const SptDijkstra_T& s) {
            os << "SptDijkstra\n";
            for (auto e : s.spt()) {
                os << e << "\n";
            }
            return os << "\n";
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
            
            size_t count = 0; // счётчик проходов.
            size_t limit = _g.size(); // Разделитель проходов. Позволяет считать проходы.
            
            q.push(v);
            q.push(limit);
            
            // Отслеживаем изменения SPT.
            bool sptChanged = false;
            
            while (!q.empty()) {
                // Извлекаем следующую вершину и проверяем, не равна ли она разделителю.
                v = q.front();
                q.pop();
                
                if (v == limit) {
                    if (q.empty()) return; // Кратчайший путь найден.
                    
                    if (++count == limit || !sptChanged) {
                        // Прошли максимальное количество проходов + 1 или
                        // Прошли проход при котором дерево SPT не изменилось, но очередь вершин непуста.
                        // В графе присутствует отрицательный цикл.
                        _hasNegativeCycle = true;
                        return;
                    }
                    sptChanged = false;
                    q.push(limit);
                    continue;
                }
                
                assert(v != limit);
                assert(_distance[v] != INF);
                
                for (auto node : _g.adjacent(v)) {
                    sptChanged |= tryRelax_({v, node}, q);
                }
            }
        }
    
        // Возвращает true, если SPT изменился.
        bool tryRelax_(const Edge& e, std::queue<size_t>& q) {
            if (_distance[e.w] > _distance[e.v] + e.weight) {
                _distance[e.w] = _distance[e.v] + e.weight;
                // Если ребро релакcировалось, то заносим вершину в очередь.
                q.push(e.w);
                if (_parent[e.w] != e.v) {
                    _parent[e.w] = e.v; // Обновляем кратчайшее ребро.
                    return  true;
                }
                return false;
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

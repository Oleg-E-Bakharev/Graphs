//
//  maxFlow.h
//  Graphs
//
//  Created by Oleg Bakharev on 10.04.17.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//
// Задача вычисления максимального потока в транспортной сети и получения минимального разреза.
// Каждому ребру назначаем целый положительный вес - ёмкость (пропускная способность) ребра.
// А также целый поток <= емкости. При этом в каждой вершине приток равен оттоку.
// Методы Форда-Фалкерсона(Расширения Пути) и Проталкивания Напора.

#ifndef maxFlow_hpp
#define maxFlow_hpp

#include <iostream>
#include <functional>
#include <limits>
#include <queue>
#include <memory>
#include "disjointSet.h"
#include "weightedGraph.h"

namespace Graph {
	
    // "вес" ребра остаточной сети.
    class ResidualInfo {
        size_t _from; // Исходная вершина ребра.
        int _capacity; // Ёмкость ребра.
        int _flow; // Поток ребра.
        
        bool from_(size_t v) const { return _from == v; }
    public:
        ResidualInfo(size_t from, int capacity):_from(from), _capacity(capacity), _flow(0) {}
        // Остаточная ёмкость ребра.
        int residualCapacityTo(size_t v) const { return from_(v) ? _flow : _capacity - _flow; }
        void addFlowTo(size_t v, int f) { _flow += from_(v) ? -f : +f; }
        
        // Сравнение по ссылке.
        bool isSame( const ResidualInfo& ri) const { return this == &ri; }
        
        friend std::ostream& operator<< (std::ostream& os, const ResidualInfo& ri) {
            os << ri._flow << "/" << ri._capacity;
            return os;
        }
    };
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Построение минимального сечения (разреза) (min-cut) графа из истока s в сток t на основе информации о заполненой
	// до максимального потока остаточной сети.
	// g - исходная сеть, rg - заполненая остаточная сеть.
	// s - исток, t - сток.
	// В заполненой до максимального потока остаточной сети отсутствует пути в сток.
	// Находим множество достижимости S из истока s - это будут левые вершины ребер минимального разреза.
	// Все ребра из вершин S исходного графа не ведущие в S будут составлять минимальный разрез исходного рафа G
	// межу вершинами s и t.
	template<class G, class RG>
	class MinCut_T {
		using Edge = typename G::Edge;
		
		const G& _g;
		const RG& _rg;
		DisjointSet _rs; // Множество достижимости (Reachability set) из истока.
		std::vector<Edge> _minCut;
		
		// Построение множества достижимости из стока по остаточной сети.
		void buildReachabilitySet_(size_t s) {
			// Проходим BFS-ом по _rg и заносим вершины в _rs.
			std::queue<size_t> bfs;
			bfs.push(s);
			while (!bfs.empty()) {
				size_t v = bfs.front(); bfs.pop();
				for (auto& node : _rg.adjacent(v)) {
					size_t w = node.dest;
					ResidualInfo& ri = node.weight;
					if ( !_rs.isConnected(s, w) && ri.residualCapacityTo(w) > 0) {
						bfs.push(w);
						_rs.uniteIfNotConnected(v, w);
					}
				}
			}
		}
		
		// Построение минимального разреза исходной сети проходом по вершинам множества достижимости остаточной сети и
		// собиая ребра, которые идут в вершины исходной сети и отсутствующие в множестве достижимости остаточной сети.
		void buildMinCut_(size_t s) {
			for (size_t v = 0; v < _g.size(); v++) {
				if (_rs.isConnected(s, v)) {
					for (auto& node : _g.adjacent(v)) {
						size_t w = node.dest;
						if (!_rs.isConnected(v, w)) {
							_minCut.push_back({v, node});
						}
					}
				}
			}
		}
		
	public:
		MinCut_T ( const G& g, const RG& rg, size_t s) : _g(g), _rg(rg), _rs(g.size()) {
			buildReachabilitySet_(s);
			buildMinCut_(s);
		}
		
		const std::vector<Edge>& operator()() const { return _minCut; }
	};
	
	template <class G, class RG>
	MinCut_T<G, RG> minCut(const G& g, const RG& rg, size_t s) {
		return {g, rg, s};
	}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Реализация метода Форда-Фалкерсона поиска максимального потока транспортной сети. O(V^3)
    template <class G> class MaxFlowFF_T {
        // В остаточной сети храним ссылки на ResidualInfo потому что прямое и обратное ребро остаточной сети должны быть
		// одним и тем же объектом ResidualInfo.
        using ResidualInfoRef = std::reference_wrapper<ResidualInfo>;
        // Остаточная сеть неориентированная!
        using ResidualNetwork = SparseGraph_T<WeightedGraphTraits<ResidualInfoRef>>;
        // Узел списка смежности остаточной сети.
        using ResidualNode = typename ResidualNetwork::NodeType;
        const int MAX = std::numeric_limits<int>::max();
        ResidualInfo nullInfo = {size_t(-1), 0};
        
        const G& _g;
        std::vector<ResidualInfo> _residualData; // Узлы, ссылки на которые хранятся в остаточной сети.
        ResidualNetwork _rn; // Остаточная сеть.
        // Дерво пути из s в t по которому можно увеличить поток.
        // В i-м элементе предок i.
        // Чтобы пройти по пути надо итерироваться с конца (от t) в начало (к s).
        std::vector<ResidualNode> _st;
        
        int _maxFlow = 0;
		
		size_t _s; // исток.
		using MinCut = MinCut_T<G, ResidualNetwork>;
		mutable shared_ptr<MinCut> _minCutPtr; // Минимальный разрез.
        
        // Построение остаточной сети.
        void buildResidualNetwork_(size_t s) {
            _residualData.reserve(_g.edgesCount() * 2); // Чтобы не было перевыделений памяти.
            for ( size_t v = s; v < _g.size(); v++) {
                for (const auto& n : _g.adjacent(v)) {
                    _residualData.push_back({v, n.weight});
                    auto& rd = _residualData.back();
                    _rn.insert({v, n.dest, rd});
                }
            }
        }
        
        // Расширение потока. Сначала проходим по пути вычисляя минимальную остаточную емкость.
        // Потом проходим по пути снова расширяя поток на значение минимальной остаточной емкости.
        void augment_(size_t s, size_t t) {
            cout << "augment step\n";                                           // Debug
            int minResidualFlow = MAX;
            for(size_t v = t; v != s; v = _st[v]) {
                ResidualInfo& ri = _st[v].weight;
                cout << v << "-" << _st[v] << "|";                              // Debug
                int residualFlow = ri.residualCapacityTo(v);
                cout << residualFlow << endl;
                minResidualFlow = std::min(residualFlow, minResidualFlow);
            }
            cout << "augment min residual flow: " << minResidualFlow << endl;   // Debug
            _maxFlow += minResidualFlow;
            for(size_t v = t; v != s; v = _st[v].dest) {
                ResidualInfo& ri = _st[v].weight;
                ri.addFlowTo(v, minResidualFlow);
            }
        }
        
        // Найти путь по которому можно еще увеличить поток транспортной сети.
        // Строит _st. Возможны различные стратегии, например путь с наименьшим числом ребер (BSF) или
        // Путь с приоритетом остаточной емкости (Дейкстра по остаточному весу).
        // Конечный результат от стратегии не изменится.
        // Мы делаем BFS - Стратегия Эдмондса-Карпа.
        bool pfs_(size_t s, size_t t) {
            cout << "bfs step\n";                                               // Debug
            _st.assign(_g.size(), {size_t(-1), nullInfo});
            std::queue<size_t> q;
            q.push(s);
            vector<bool> visited(_g.size());
            visited[s] = true;
            while(!q.empty()) {
                size_t v = q.front(); q.pop();
                cout << "pop " << v << endl;                                    // Debug
                visited[v] = true;
                for(const ResidualNode& n : _rn.adjacent(v)) {
                    ResidualInfo& ri = n.weight;
                    int residualCapacity = ri.residualCapacityTo(n.dest);
                    ResidualInfo& riDest = _st[n.dest].weight;
                    // Второе условие исключает зацикливание.
                    if (residualCapacity > 0 && riDest.isSame(nullInfo)) {
                        cout << v << "-" << n << "|" << residualCapacity << endl; // Debug
                        _st[n.dest] = {v, ri};
                        if (n.dest == t) {
                            return true;
                        }
                        if (!visited[n.dest]) {
                            cout << "push " << n.dest << endl;                  // Debug
                            q.push(n.dest);
                            visited[n.dest] = true;
                        } else {
                            cout << "skip " << n.dest << endl;                   // Debug
                        }
                    }
                }
            }
            return false;
        }
    public:
        MaxFlowFF_T(const G& g, size_t s, size_t t) : _g(g), _rn(g.size()), _s(s) {
            std::cout << "Ford-Fulkerson\n";                                    // Debug
            buildResidualNetwork_(s);
            std::cout << "Residual network at begin:\n" << _rn;                 // Debug
            while (pfs_(s, t)) {
                augment_(s, t);
            }
            std::cout << "Residual network at end:\n" << _rn;                   // Debug
        }
        
        int operator()(void) const { return _maxFlow; }
		
		// Минимальный разрез из графа G из истока s.
		const std::vector<typename G::Edge>& minCut() const {
			if (!_minCutPtr) {
				_minCutPtr = make_shared<MinCut>(_g, _rn, _s);
			}
			return (*_minCutPtr)();
		}
    };
    
    // Ускоритель вызова
    template<typename G> MaxFlowFF_T<G> maxFlowFF(const G& g, size_t s, size_t t) { return {g, s, t}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Проталкивание напора (Preflow-Push). Goldberg Tagjan 1986. O(V^2E)
    template <class G> class MaxFlowPP_T {
        // В остаточной сети храним ссылки на ResidualInfo потому что прямое и обратное ребро остаточной сети должны быть
		// одним и тем же объектом ResidualInfo.
        using ResidualInfoRef = std::reference_wrapper<ResidualInfo>;
        // Остаточная сеть неориентированная!
        using ResidualNetwork = SparseGraph_T<WeightedGraphTraits<ResidualInfoRef>>;
        // Узел списка смежности остаточной сети.
        using ResidualNode = typename ResidualNetwork::NodeType;
        const int MAX = std::numeric_limits<int>::max();
  
        const G& _g;
        std::vector<size_t> _heights; // Высоты узлов остаточной сети.
        std::vector<ResidualInfo> _residualData; // Узлы, ссылки на которые хранятся в остаточной сети.
        ResidualNetwork _rn; // Остаточная сеть.
        int _maxFlow; // Максимальный поток в сети.
		
		size_t _s; // исток.
		using MinCut = MinCut_T<G, ResidualNetwork>;
		mutable shared_ptr<MinCut> _minCutPtr; // Минимальный разрез.

        void initHeights_(size_t s, size_t t) {
            // Идем BFS из стока остаточной сети, назначая вершинам расстояния от стока,
            std::queue<size_t> bfs;
            bfs.push(t);
            vector<bool> visited(_rn.size());
            while (!bfs.empty()) {
                size_t v = bfs.front(); bfs.pop();
                visited[v] = true;
                for (auto& n : _rn.adjacent(v)) {
                    size_t w = n.dest;
                    if (!visited[w]) {
                        _heights[w] = _heights[v] + 1;
                        bfs.push(w);
                    }
                }
            }
        }
        
        // Построение остаточной сети.
        void buildResidualNetwork_(size_t s) {
            _residualData.reserve(_g.edgesCount() * 2); // Чтобы не было перевыделений памяти.
            for ( size_t v = s; v < _g.size(); v++) {
                for (const auto& n : _g.adjacent(v)) {
                    _residualData.push_back({v, n.weight});
                    auto& rd = _residualData.back();
                    _rn.insert({v, n.dest, rd});
                }
            }
        }
        
        // Проталкивание напора. Возвращает максимальный поток из стока.
        int preflowPush_(size_t s, size_t t) {
            // Избыточный поток вершин.
            std::vector<int> overflows(_g.size(), 0);
            overflows[s] = MAX;
            
            // Очередь активных вершин.
            std::queue<size_t> active;
            active.push(s);
            
            // Флаги активности вершин.
            std::vector<bool> isActive(_g.size(), false);
            isActive[s] = true;
            // Вершина t никогда в active не добавляется.
            isActive[t] = true;
            
            while (!active.empty()) {
                size_t v = active.front();
                std::cout << "get " << v << " with height " << _heights[v] << " and overflow " << overflows[v] << endl; // Debug
                for (const auto& node : _rn.adjacent(v)) {
                    size_t w = node.dest;
                    ResidualInfo& ri = node.weight;
                    int destResidualCapacity = ri.residualCapacityTo(w);
                    int destFlow = destResidualCapacity < overflows[v] ? destResidualCapacity : overflows[v];
                    // Поток проталкиваем только в подходящие(eliglible) ребра (h[w] = h[v] - 1.
                    if (destFlow > 0 && _heights[w] == _heights[v] - 1) {
                        std::cout << "push flow " << destFlow << " from " << v << " to " << w << endl;   // Debug
                        // Если протолкнуть поток удается то активизируем конечную вершину ребра, если она еще не активна.
                        ri.addFlowTo(w, destFlow);
                        overflows[v] -= destFlow;
                        overflows[w] += destFlow;
                        if (!isActive[w]) {
                            std::cout << "activate " << w << " with height " << _heights[w] << " overflow " << overflows[w] << endl; // Debug
                            active.push(w);
                            isActive[w] = true;
                        }
                    }
                }
                // Если наша вершина деактивировалась, убираем ее из очереди. Иначе увеличиваем ее высоту.
                if (v == s) {
                    // Вершина s извлекается из очереди сразу и никогда больше в нее не возвращается.
                    active.pop();
                } else if (overflows[v] == 0) {
                    active.pop();
                    isActive[v] = false;
                    std::cout << "deactivate " << v << endl;                                    // Debug
                } else {
                    _heights[v]++;
                    std::cout << "increase height of " << v << " to " << _heights[v] << endl;   // Debug
                }
            }
            return overflows[t];
        }

    public:
        MaxFlowPP_T( const G& g, size_t s, size_t t ) : _g(g), _rn(g.size()), _heights(g.size(), 0), _s(s) {
            buildResidualNetwork_(s);
            std::cout << "\nResidual network at begin:\n" << _rn;               // Debug
            std::cout << "Preflow-Push\nHeights: ";                             // Debug
            initHeights_(s, t);
            for (size_t v : _heights) std::cout << v << ", ";                   // Debug
            _maxFlow = preflowPush_(s, t);
            std::cout << "\nResidual network at end:\n" << _rn;                 // Debug
        }
        
        int operator()(void) const { return _maxFlow; }
		
		// Минимальный разрез из графа G из истока s.
		const std::vector<typename G::Edge>& minCut() const {
			if (!_minCutPtr) {
				_minCutPtr = make_shared<MinCut>(_g, _rn, _s);
			}
			return (*_minCutPtr)();
		}
    };
    
    // Ускоритель вызова
    template<typename G> MaxFlowPP_T<G> maxFlowPP(const G& g, size_t s, size_t t) { return {g, s, t}; }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Алгоритм Диница (Dinic). Ефим Диниц 1979. O(V^2E)
    // Идея - итеративно вычисляем расстояние до достижимых из истока вершин bfs-ом (получаем слоистую сеть) после чего
    // делаем dfs по слоистой сети (по ребрам связывающим предыдущий слой с последующим) наращивая поток на максимально
    // возможное значение. Итерации завершаем когда на этапе bfs не достигаем стока.
    // http://e-maxx.ru/algo/dinic
    template <class G> class MaxFlowD_T {
        // В остаточной сети храним ссылки на ResidualInfo потому что прямое и обратное ребро остаточной сети должны быть
        // одним и тем же объектом ResidualInfo.
        using ResidualInfoRef = std::reference_wrapper<ResidualInfo>;
        // Остаточная сеть неориентированная!
        using ResidualNetwork = SparseGraph_T<WeightedGraphTraits<ResidualInfoRef>>;
        // Узел списка смежности остаточной сети.
        using ResidualNode = typename ResidualNetwork::NodeType;
        const int MAX = std::numeric_limits<int>::max();
        
        const G& _g;
        std::vector<size_t> _heights; // Высоты узлов остаточной сети.
        std::vector<ResidualInfo> _residualData; // Узлы, ссылки на которые хранятся в остаточной сети.
        ResidualNetwork _rn; // Остаточная сеть.
        std::vector<ResidualNetwork::AdjIter::iterator> _next; // для вершины v - вершина _next[v] первого неудаленного инцедентного реьбра остаточной сети.
        int _maxFlow; // Максимальный поток в сети.
        
        size_t _s; // исток.
        size_t _t; // сток.
        using MinCut = MinCut_T<G, ResidualNetwork>;
        mutable shared_ptr<MinCut> _minCutPtr; // Минимальный разрез.
        
        // Построение остаточной сети.
        void buildResidualNetwork_() {
            _residualData.reserve(_g.edgesCount() * 2); // Чтобы не было перевыделений памяти.
            for ( size_t v = _s; v < _g.size(); v++) {
                for (const auto& n : _g.adjacent(v)) {
                    _residualData.push_back({v, n.weight});
                    auto& rd = _residualData.back();
                    _rn.insert({v, n.dest, rd});
                }
            }
        }

        bool bfsHeights_() {
            // Идем BFS из истока остаточной сети, назначая вершинам расстояния от истока,
            // возвращем достижимость стока.
            std::queue<size_t> bfs;
            _heights.assign(_g.size(), -1);
            bfs.push(_s);
            _heights[_s] = 0;
            while (!bfs.empty()) {
                size_t v = bfs.front(); bfs.pop();
                if (v == _t) {
                    break;
                }
                for (auto& node : _rn.adjacent(v)) {
                    size_t w = node.dest;
                    ResidualInfo& ri = node.weight;
                    if (_heights[w] == -1 && ri.residualCapacityTo(w) > 0) {
                        _heights[w] = _heights[v] + 1;
                        bfs.push(w);
                    }
                }
            }
            return _heights[_t] == -1;
        }
        
        // DFS по слоистой сети с получением максимального добавленного потока.
        int layeredDfs_(size_t v, int flow) {
            if (flow == 0 || v == _t) {
                return flow;
            }
            for (auto& it = _next[v]; it != _rn.adjacent(v).end(); it++ ) {
                ResidualNode& node = *it;
                size_t w = node.dest;
                // Пропускаем вершины не следующего слоя.
                if (_heights[w] != _heights[v] + 1) {
                    continue;
                }
                ResidualInfo& ri = node.weight;
                int pushed = layeredDfs_(w, min(flow, ri.residualCapacityTo(w)));
                if (pushed > 0) {
                    ri.addFlowTo(w, pushed);
                    return pushed;
                }
            }
            
            return 0;
        }
        
    public:
        MaxFlowD_T( const G& g, size_t s, size_t t ) : _g(g), _next(g.size()), _rn(g.size()), _s(s), _t(t), _maxFlow(0) {
            // Остаточная сеть.
            buildResidualNetwork_();
            // Пока есть t достижим из s.
            while (!bfsHeights_()) {
                // Инициализация _next;
                for( size_t v = 0; v < g.size(); v++ ) {
                    _next[v] = _rn.adjacent(v).begin();
                }
                // Заполняем блокирующий поток.
                while(int flow = layeredDfs_(s, MAX)) {
                    _maxFlow += flow;
                }
            }
        }
        
        int operator()(void) const { return _maxFlow; }
        
        // Минимальный разрез из графа G из истока s.
        const std::vector<typename G::Edge>& minCut() const {
            if (!_minCutPtr) {
                _minCutPtr = make_shared<MinCut>(_g, _rn, _s);
            }
            return (*_minCutPtr)();
        }
    };
    
    // Ускоритель вызова
    template<typename G> MaxFlowD_T<G> maxFlowD(const G& g, size_t s, size_t t) { return {g, s, t}; }
}

void maxFlowTest();
#endif /* maxFlow_hpp */

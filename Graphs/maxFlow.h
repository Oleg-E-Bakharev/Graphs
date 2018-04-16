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
    
    /*
     Определения из Седжвика.
     Опр.22.1. Сеть с вершиной s, выбранной в качестве истока и вершиной t, выбранной в качестве стока, называется
     st-сетью.
     
     Опр.22.2. Транспортная сеть (flow network) - это st-сеть с положительными весами рёбер, которые мы будем называть
     пропускными способностями (capacity). Поток (flow) в транспортной сети - это множество неотрицательных весов рёбер
     которые называются рёберными потоками (edge flow) и удовлетворяют условиям: поток в любом ребре не превышает
     пропускную способность этого ребра, а суммарный поток, входящий в каждую внутреннюю вершину (приток вершины), равен
     суммарному потоку исходящему из этой вершины (отток вершины).
     
     Лемма 22.1. Любой st-поток обладает тем свойством, что отток из вершины s равен притоку в вершину t.
     
     Следствие: Величина потока объединения двух множеств вершин равна сумме величин каждого потока этих двух множеств
     минус сумма весов рёбер, соединяющих вершину одного множества с вершиной другого.
     
     Опр. Сечение или разрез (cut) графа есть разбиение множества вершин графа на два непересекающихся подмножества, а
     перекрестное ребро (crossing edge) - это ребро соединяющее вершину одного подмнодества, с вергиной другого.
     
     Опр. 22.3. st-сечение - это сечение, которое помещает вершину s в одно из подмножеств, а вершину t в другое.
     
     Лемма 22.3. Для любого st-потока поток через произворльное st-сечение равен величине всего потока.
     
     Лемма 22.4. Величина st-потока не может превышать пропускной способности любого st-сечения.
     
     Лемма 22.5. (Теорема о максимальном потоке и минимальном сечении). Максимальный поток из всех st-потоков в сети
     равен минимальной пропускной способности из всех st-сечений.
     
     Опр. 22.4. Пусть задана транспортная сеть и поток в ней. Остаточная сеть (residual network) для данного потока
     содержит те же вершины, что и исходная сеть, и одно или два ребра на каждое ребро исходной сети, которые
     определяются следующим образом: Пусть f-поток, а c - пропускная способность произвольного ребра v-w из исходной
     сетис. Если f положительно, то в остаточную сеть включается ребро w-v пропускной способностью f (обратное ребро);
     и если f < c, то в остаточную сеть включается ребро v-w (прямое ребро) c пропускной способностью c-f.
     */
	
    // "вес" ребра остаточной сети (ОС).
    class ResidualInfo {
        size_t _from; // Исходная вершина ребра.
        int _capacity; // Ёмкость ребра.
        int _flow; // Поток ребра.
        
        // Возвращает true если v соотвтетсвует истоку ребра ОС.
        bool from_(size_t v) const { return _from == v; }
    public:
        ResidualInfo(size_t from, int capacity):_from(from), _capacity(capacity), _flow(0) {}
        // Остаточная ёмкость ребра.
        int residualCapacityTo(size_t v) const { return from_(v) ? _flow : _capacity - _flow; }
        void addFlowTo(size_t v, int f) { _flow += from_(v) ? -f : +f; }
        
        // Сравнение по ссылке.
        bool isSame(const ResidualInfo& ri) const { return this == &ri; }
        
        friend std::ostream& operator<< (std::ostream& os, const ResidualInfo& ri) {
            return os << ri._flow << "/" << ri._capacity;
        }
    };
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Построение секущего множества (cut set) минимального сечения (разреза) (min cut) графа из истока s в сток t на
    // основе информации о заполненой до максимального потока остаточной сети.
	// g - исходная сеть, rg - заполненая остаточная сеть.
	// s - исток, t - сток.
	// В заполненой до максимального потока остаточной сети отсутствует пути в сток.
	// Находим множество достижимости S из истока s - это будут левые вершины ребер минимального разреза.
	// Все ребра из вершин S исходного графа не ведущие в S будут составлять минимальный разрез исходного рафа G
	// межу вершинами s и t.
	template<class G, class RG>
	class MinCutSet_T {
		using Edge = typename G::Edge;
		
		std::vector<Edge> _minCutSet;
		
		// Построение множества достижимости из стока по остаточной сети.
        DisjointSet buildReachabilitySet_(const RG& rg, size_t s) {
			// Проходим BFS-ом по rg и заносим вершины в rs.
            DisjointSet rs(rg.size()); // Множество достижимости (Reachability set) из истока.
			std::queue<size_t> bfs;
			bfs.push(s);
			while (!bfs.empty()) {
				size_t v = bfs.front(); bfs.pop();
				for (auto& node : rg.adjacent(v)) {
					size_t w = node.dest;
					ResidualInfo& ri = node.weight;
					if ( !rs.isConnected(s, w) && ri.residualCapacityTo(w) > 0) {
						bfs.push(w);
						rs.uniteIfNotConnected(v, w);
					}
				}
			}
            return rs;
		}
		
		// Построение минимального разреза исходной сети проходом по вершинам множества достижимости остаточной сети и
		// собиая ребра, которые идут в вершины исходной сети и отсутствующие в множестве достижимости остаточной сети.
		void buildMinCutSet_(const G& g, size_t s, const DisjointSet& rs) {
			for (size_t v = 0; v < g.size(); v++) {
				if (rs.isConnected(s, v)) {
					for (auto& node : g.adjacent(v)) {
						size_t w = node.dest;
						if (!rs.isConnected(v, w)) {
							_minCutSet.push_back({v, node});
						}
					}
				}
			}
		}
		
	public:
		MinCutSet_T ( const G& g, const RG& rg, size_t s) {
			DisjointSet&& rs = buildReachabilitySet_(rg, s);
			buildMinCutSet_(g, s, rs);
		}
		
		const std::vector<Edge>& operator()() const { return _minCutSet; }
	};
	
	template <class G, class RG>
	MinCutSet_T<G, RG> minCut(const G& g, const RG& rg, size_t s) {
		return {g, rg, s};
	}

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Реализация метода Форда-Фалкерсона поиска максимального потока транспортной сети. O(V^3) достигнуто  О(V^2E).
    // Для разреженой сети достигнуто V^2lgMlgV, где M - Максимальная пропускная способность рёбер в сети.
    template <class G> class MaxFlowFF_T {
        // В остаточной сети храним ссылки на ResidualInfo потому что прямое и обратное ребро остаточной сети должны быть
		// одним и тем же объектом ResidualInfo.
        using ResidualInfoRef = std::reference_wrapper<ResidualInfo>;
        // Остаточная сеть неориентированная!
        using ResidualNetwork = SparseGraph_T<WeightedGraphTraits<ResidualInfoRef>>;
        // Узел списка смежности остаточной сети.
        using ResidualNode = typename ResidualNetwork::NodeType;
        const int MAX = std::numeric_limits<int>::max();
        
        const G& _g;
        std::vector<ResidualInfo> _residualData; // Узлы, ссылки на которые хранятся в остаточной сети.
        ResidualNetwork _rn; // Остаточная сеть.
        // Дерво пути из s в t по которому можно увеличить поток.
        // В i-м элементе предок i.
        // Чтобы пройти по пути надо итерироваться с конца (от t) в начало (к s).
        std::vector<ResidualNode> _st;
        
        int _maxFlow = 0;
		
		size_t _s; // исток.
		using MinCutSet = MinCutSet_T<G, ResidualNetwork>;
		mutable shared_ptr<MinCutSet> _minCutPtr; // Минимальный разрез.
        
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
                    if (!visited[n.dest] && residualCapacity > 0) {
                        cout << v << "-" << n << "|" << residualCapacity << endl; // Debug
                        _st[n.dest] = {v, ri};
                        if (n.dest == t) {
                            return true;
                        }
                        cout << "push " << n.dest << endl;                  // Debug
                        q.push(n.dest);
                        visited[n.dest] = true;
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
            // Заполняем вектор путьи расширения произвольными значениями. Вачно чтобы он был нужного размера.
            _st.assign(_g.size(), ResidualNode{size_t(-1), _residualData[0]});
            while (pfs_(s, t)) {
                augment_(s, t);
            }
            std::cout << "Residual network at end:\n" << _rn;                   // Debug
        }
        
        int operator()(void) const { return _maxFlow; }
		
		// Минимальный разрез из графа G из истока s.
		const std::vector<typename G::Edge>& minCutSet() const {
			if (!_minCutPtr) {
				_minCutPtr = make_shared<MinCutSet>(_g, _rn, _s);
			}
			return (*_minCutPtr)();
		}
    };
    
    // Ускоритель вызова
    template<typename G> MaxFlowFF_T<G> maxFlowFF(const G& g, size_t s, size_t t) { return {g, s, t}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Проталкивание напора (Preflow-Push). Goldberg Tagjan 1986. O(V^2E) достигнуто ~O(VE).
    /*
     Опр. 22.5. Напор (preflow) в транспортных сетях это множество положительных потоков в рёбрах, удовлетворяющих
     условию: поток в каждом ребре не превышает пропускную способность этого ребра, и приток в каждой внутренней вершине
     не меньше оттока. Активная вершина - ёто внутренняя вершина, приток в которой больше оттока (по согласованию, исток
     и сток не могут быть активными).
     
     Опр. 22.6. Функция высоты для данного потока в транспортной сети - это множество неотрицательных весов вершин
     h(0) - h(V-1), такое что h(t) = 0 для стока t, и h(u) <= h(v) + 1 для каждого ребра u-v остаточной сети этого
     потока. Активное (подходящее) ребро (eligible edge) это такое ребро остаточной сети, что h(u) = h(v) + 1.
     
     Лемма 22.9. Для любого потока и связанной с ним функции высоты, высота любой вершины не превосходит длины
     кратчайшего пути из этой вершины в сток основной сети.
     
     Следствие. Если высота вершины больше V, то в остаточной сети не существует пути из этой вершины в сток.
     
     Суть алгоритма: Выбираем активную вершину. Проталкиваем поток через некоторое активное ребро (по возможности
     заполняем его), исходящее из этой вершины (если оно есть), и продолжаем так, пока вершина не перестаёт быть
     активной или не останется активных рёбер. В последнем случае увеличиваем высоту вершины.
     
     Лемма 22.10 Рёберный алгоритм проталкивания напора сохраняет адекватность функции высоты.
     */
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
		using MinCutSet = MinCutSet_T<G, ResidualNetwork>;
		mutable shared_ptr<MinCutSet> _minCutPtr; // Минимальный разрез.

        void initHeights_(size_t s, size_t t) {
            // Идем BFS из стока остаточной сети, назначая вершинам расстояния от стока.
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
		const std::vector<typename G::Edge>& minCutSet() const {
			if (!_minCutPtr) {
				_minCutPtr = make_shared<MinCutSet>(_g, _rn, _s);
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
    /*
     Блокирующим потоком в данной сети называется такой поток, что любой путь из истока s в сток t содержит насыщенное
     этим потоком ребро. Иными словами, в данной сети не найдётся такого пути из истока в сток, вдоль которого можно
     беспрепятственно увеличить поток.
     
     Блокирующий поток не обязательно максимален. Теорема Форда-Фалкерсона говорит о том, что поток будет максимальным
     тогда и только тогда, когда в остаточной сети не найдётся s-t пути; в блокирующем же потоке ничего не утверждается
     о существовании пути по рёбрам, появляющимся в остаточной сети.
     
     Слоистая сеть для данной сети строится следующим образом. Сначала определяются длины кратчайших путей из истока s
     до всех остальных вершин; назовём уровнем level[v] вершины её расстояние от истока. Тогда в слоистую сеть включают
     все те рёбра (u,v) исходной сети, которые ведут с одного уровня на какой-либо другой, более поздний, уровень, т.е.
     level[u] + 1 = level[v] (почему в этом случае разница расстояний не может превосходить единицы, следует из свойства
     кратчайших расстояний). Таким образом, удаляются все рёбра, расположенные целиком внутри уровней, а также рёбра,
     ведущие назад, к предыдущим уровням.
     
     Очевидно, слоистая сеть ациклична. Кроме того, любой s-t путь в слоистой сети является кратчайшим путём в исходной сети.
     
     Построить слоистую сеть по данной сети очень легко: для этого надо запустить обход в ширину по рёбрам этой сети,
     посчитав тем самым для каждой вершины величину level[], и затем внести в слоистую сеть все подходящие рёбра.
     
     Алгоритм представляет собой несколько фаз. На каждой фазе сначала строится остаточная сеть, на ней строится
     слоистая сеть (обходом в ширину), а в ней ищется произвольный блокирующий поток. Найденный блокирующий поток
     прибавляется к текущему потоку, и на этом очередная итерация заканчивается.
     
     Этот алгоритм схож с алгоритмом Эдмондса-Карпа, но основное отличие можно понимать так: на каждой итерации поток
     увеличивается не вдоль одного кратчайшего s-t пути, а вдоль целого набора таких путей (ведь именно такими путями
     и являются пути в блокирующем потоке слоистой сети).
     */
    
    template <class G> class MaxFlowD_T {
        // В остаточной сети храним ссылки на ResidualInfo потому что прямое и обратное ребро остаточной сети должны
        // быть одним и тем же объектом ResidualInfo.
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
        // для вершины v - вершина _next[v] первого неудаленного инцедентного реьбра остаточной сети.
        std::vector<ResidualNetwork::AdjIter::iterator> _next;
        int _maxFlow; // Максимальный поток в сети.
        
        size_t _s; // исток.
        size_t _t; // сток.
        using MinCutSet = MinCutSet_T<G, ResidualNetwork>;
        mutable shared_ptr<MinCutSet> _minCutPtr; // Минимальный разрез.
        
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
        const std::vector<typename G::Edge>& minCutSet() const {
            if (!_minCutPtr) {
                _minCutPtr = make_shared<MinCutSet>(_g, _rn, _s);
            }
            return (*_minCutPtr)();
        }
    };
    
    // Ускоритель вызова
    template<typename G> MaxFlowD_T<G> maxFlowD(const G& g, size_t s, size_t t) { return {g, s, t}; }
}

void maxFlowTest();
#endif /* maxFlow_hpp */

//
//  graphGen.h
//  Graphs
//
//  Created by Oleg Bakharev on 12/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef graphGen_h
#define graphGen_h

#include <vector>
#include <random>
#include <algorithm>
#include "matrix.h"

namespace Graph {
    
    // Вставляет в граф список ребер.
    template <class G>
    void insertEdges( G& g, const std::initializer_list<typename G::Edge>&& es) {
        for( auto&& e : es ) {
            g.insert(e);
        }
    }
    
    // Вставляет в граф список ребер из списка смежности
    template <class G>
    void insertEdges( G& g, std::vector<std::vector<size_t>>&& adj) {
        assert(g.size() == adj.size());
        for ( size_t v = 0; v < adj.size(); v++ ) {
            auto row = adj[v];
            for ( size_t w = 0; w < row.size(); w++ )
                g.insert({v, row[w]});
        }
    }
    
    // Вставляет в граф список ребер из матрицы смежности.
    template <class G>
	void insertEdges( G& g, matrix<typename G::Traits::Weight>&& adj) {
        assert(g.size() == adj.h() && g.size() == adj.w());
        for ( size_t v = 0; v < adj.w(); v++ )
			for ( size_t w = 0; w < adj.h(); w++ ) {
				auto weight = adj[v][w];
                if ( weight != 0 ) g.insert({v, w, weight});
			}
    }
	
    // Вставляет в граф список ребер.
    template <class G>
    void insertEdges( G& g, const std::vector<typename G::Edge>& es) {
        for( auto& e : es ) {
            g.insert(e);
        }
    }
    
    // Генератор случайных ребер для графа.
    template <typename Edge>
    std::vector<Edge> randE(size_t e) {
        using namespace std;
        random_device r;
        ranlux48_base gen(r());
        uniform_real_distribution<> dis;
        vector<Edge> ve;
        ve.reserve(e);
        for( size_t i = 0; i < e; i++ ) {
            ve.push_back({size_t(e * dis(gen)), size_t(e * dis(gen))});
        }
        return ve;
    }

    // Добавление в граф E случайных ребер.
    template <class G> void randE(G& g, size_t e) {
        using namespace std;
        random_device r;
        ranlux48_base gen(r());
        uniform_real_distribution<> dis;
        for( size_t i = 0; i < e; i++ ) {
            size_t v = size_t(g.size() * dis(gen));
            size_t w = size_t(g.size() * dis(gen));
            g.insert({v, w});
        }
    }
    
    // Генератор всех ребер графа размером n, но каждое ребро вставляется с такой вероятностью p,
    // чтобы результирующее количество ребер примерно равнялось e = p * n * (n-1)/2.
    template <typename Edge>
    std::vector<Edge> randG(size_t n, size_t e) {
        using namespace std;
        random_device r;
        ranlux48_base gen(r());
        uniform_real_distribution<> dis;
        vector<Edge> ve;
        ve.reserve(size_t(e * 1.2));
        double p = 2. * e / n / (n - 1);
        for( size_t i = 0; i < n; i++ )
            for( size_t j = 0; j < i; j++ )
                if ( dis(gen) < p )
                    ve.push_back({i, j});
        return ve;
    }

    // Добавление в граф всех ребер, но каждое ребро вставляется с такой вероятностью p,
    // чтобы результирующее количество ребер примерно равнялось e = p * n * (n-1)/2.
    template <class G> void randG(G& g, size_t e) {
        std::random_device r;
        std::ranlux48_base gen(r());
        std::uniform_real_distribution<> dis;
        double p = 2. * e / g.size() / ( g.size() - 1);
        for( size_t i = 0; i < g.size(); i++ )
            for( size_t j = 0; j < i; j++ )
                if (dis(gen) < p)
                    g.insert({i, j});
    }
    
    // k-соседний граф.
    // Генератор случайных ребер, которые соединяют вершины отстоящие не более чем на k.
    // Добавление в граф всех k-соседних ребер, но каждое ребро вставляется с такой вероятностью p,
    // чтобы результирующее количество ребер примерно равнялось e = p * n * (n-1)/2.
    template <class G> void k_neighbor(G& g, size_t e, size_t k) {
        std::random_device r;
        std::ranlux48_base gen(r());
        std::uniform_real_distribution<> dis;
        double factor = g.directed() ? 2. : 0.5;
        double p = factor * e / g.size() / k;
        for( size_t i = 0; i < g.size(); i++ )
            for( size_t j = i - k; j != i + k + 1; j++ ) {
                // Исключаем соединения ребер вначале и в конце.
                size_t l = (j + g.size()) % g.size();
                if ( dis(gen) < p ) {
                    g.insert({i, l});
                }
            }
    }
    
}

#endif /* graphGen_h */

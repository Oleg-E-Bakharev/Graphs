//
//  spAllDag.h
//  Graphs
//
//  Created by Oleg Bakharev on 31/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#ifndef spAllDag_h
#define spAllDag_h

#include <iostream>
#include <vector>
#include <memory>
#include "matrix.h"
#include "weightedGraph.h"
#include "dag.h"

namespace Graph {
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Наидлиннейшие пути в Dag. O(VE). Седжвик 21.6
    template <class G> class SPAllDagMax_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        using Node = typename Traits::AdjListNodeType;
        
        std::vector<Weight> _distances; // Кратчайшие расстояния из топологического истока в i-ю вершину.
        std::vector<size_t> _sources; // Минимальный остовный лес. Истоки ребер со стоком в i-й вершине.
        
    public:
        SPAllDagMax_T( const G& g ) :_distances(g.size(), 0), _sources(g.size(), -1) {
            
            auto ts = TS(g);
            assert(ts.isDAG());
            // Проходим по всем вершинам в топологическом порядке.
            for (size_t v : ts.ts()) {
                for (Node node : g.adjacent(v)) {
                    // релаксация.
                    if (_distances[node.dest] < _distances[v] + node.weight) {
                        _distances[node.dest] = _distances[v] + node.weight;
                        _sources[node.dest] = v;
                    }
                }
            }
        }
        
        // Вес пути из истока в v.
        double distance(size_t v) const {
            return _distances[v];
        }
        
        // Кратчайший путь из истока в v.
        std::vector<Edge> path(size_t v) const {
            std::vector<Edge> path;
            while (_sources[v] != -1) {
                path.push_back({_sources[v], v, _distances[v]});
                v = _sources[v];
            }
            return path;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SPAllDagMax_T& spAll) {
            using namespace std;
            os << "SPAllDagMax_T\n";
            for (size_t i = 0; i < spAll._sources.size(); i++ ) {
                os << setw(4) << i << "|";
            }
            os << "\nSources\n";
            for (size_t source : spAll._sources) {
                os << setw(4) << (source == -1 ? 999 : source) << "|";
            }
            os << "\nDistances:\n";
            for (Weight weight : spAll._distances) {
                os << setw(4) << weight << "|";
            }
            return os;
        }
    };
    
    template<class G> SPAllDagMax_T<G> spAllDagMax(const G& g) { return {g}; }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Кратчайшие пути в Dag. O(VE). Седжвик 21.6
    template <class G> class SPAllDagMin_T {
        using Traits = typename G::Traits;
        using Edge = typename Traits::EdgeType;
        using Weight = typename Traits::WeightType;
        using Node = typename Traits::AdjListNodeType;
        const Weight INF = std::numeric_limits<Weight>::max();
        
        std::vector<Weight> _distances; // Кратчайшие расстояния из топологического истока в i-ю вершину.
        std::vector<size_t> _sources; // Минимальный остовный лес. Истоки ребер со стоком в i-й вершине.
        
        Weight inf_() const { return INF; }
        
    public:
        SPAllDagMin_T( const G& g ) :_distances(g.size(), INF), _sources(g.size(), -1) {
            
            auto ts = TS(g);
            assert(ts.isDAG());
            // Проходим по всем вершинам в топологическом порядке.
            for (size_t v : ts.ts()) {
                for (Node node : g.adjacent(v)) {
                    if (_distances[v] == INF) {
                        // Истокам назначаем вес 0.
                        _distances[v] = 0;
                    }
                    // релаксация.
                    if (_distances[node.dest] > _distances[v] + node.weight) {
                        _distances[node.dest] = _distances[v] + node.weight;
                        _sources[node.dest] = v;
                    }
                }
            }
        }
        
        // Вес пути из самого дальнего истока в v.
        double distance(size_t v) const {
            return _distances[v];
        }
        
        // Кратчайший путь из самого дальнего истока в v.
        std::vector<Edge> path(size_t v) const {
            std::vector<Edge> path;
            while (_sources[v] != -1) {
                path.push_back({_sources[v], v, _distances[v]});
                v = _sources[v];
            }
            return path;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SPAllDagMin_T& spAll) {
            using namespace std;
            os << "SPAllDagMin_T\n";
            for (size_t i = 0; i < spAll._sources.size(); i++ ) {
                os << setw(4) << i << "|";
            }
            os << "\nSources\n";
            for (size_t source : spAll._sources) {
                os << setw(4) << (source == -1 ? 999 : source) << "|";
            }
            os << "\nDistances:\n";
            for (Weight weight : spAll._distances) {
                os << setw(4) << (weight == spAll.inf_() ? 0 : weight) << " ";
            }
            return os;
        }
    };
    
    template<class G> SPAllDagMin_T<G> spAllDagMin(const G& g) { return {g}; }
}

void spAllDagTest();

#endif /* spAllDag_h */

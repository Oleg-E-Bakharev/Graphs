//
//  weightedGraph.h
//  Graphs
//
//  Created by Oleg Bakharev on 02/03/17.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#ifndef weightedGraph_h
#define weightedGraph_h

#include <iostream>
#include <limits>
#include "denseGraph.h"
#include "sparseGraph.h"

namespace Graph {

    // По соглашению, если во взешенном графе ребро имеет нулевой вес, то в матрицу смежности должно заноситься
    // минимально значащий вес (numeric_limits<Weight>::epsilon()).
    // Это сделано для того чтобы различать в матрице смежности ребро нулевого веса и отсутствие ребра.
    
	template<class Weight>
    struct WeightedAdjListNode;
	
    // Ребро графа.
	template<typename Weight = double>
    struct WeightedGraphEdge : GraphEdge {
        Weight weight;
        
        WeightedGraphEdge(size_t v, size_t w, Weight weight) : GraphEdge(v, w), weight(weight) {}
        WeightedGraphEdge(size_t v, const WeightedAdjListNode<Weight>& n);// : GraphEdge(v, n.v), wt(n.weight) {}
        operator Weight() const { return weight; }
        WeightedGraphEdge inverse() const { return {w, v, weight}; }
        
        friend std::ostream& operator<<(std::ostream& os, const WeightedGraphEdge& e) {
            os << "(" << e.v << "-" << e.w << ", " << e.weight << ")";
            return os;
        }
    };
	
	// Элемент списка смежности. Конечная вершина ребра + вес.
	template<typename Weight = double>
    struct WeightedAdjListNode {
        size_t dest; // Вершина куда направлено ребро.
        Weight weight; // Вес ребра.
        
        WeightedAdjListNode(size_t dest, Weight weight) : dest(dest), weight(weight) {}
        WeightedAdjListNode(const WeightedGraphEdge<Weight>& e) : dest(e.w), weight(e.weight) {}
        
        operator size_t&() { return dest; }
        operator const size_t&() const { return dest; }
        
        friend std::ostream& operator<<(std::ostream& os, const WeightedAdjListNode& n) {
            os << "(" << n.dest << "," << n.weight << ")";
            return os;
        }
    };
    
    // Разрыв циклической зависимости WeightedGraphEdge и WeightedAdjListNode.
	template<typename Weight>
    inline WeightedGraphEdge<Weight>::WeightedGraphEdge(size_t v, const WeightedAdjListNode<Weight>& n) : GraphEdge(v, n.dest), weight(n.weight) {}
	
	template<class Weight = double>
    struct WeightedGraphTraits : public GraphTraits {
        static const bool weighted = true;
        using EdgeType = WeightedGraphEdge<Weight>;
        using AdjListNodeType = WeightedAdjListNode<Weight>;
        using WeightType = Weight;
    };
	
	template<class Weight = double>
    struct WeightedDirectedGraphTraits : public WeightedGraphTraits<Weight> {
        static const bool directed = true;
    };
    
    // Сокращения именований взвешенных графов.
    using DenseGraphW = DenseGraph_T<WeightedGraphTraits<double>>;
    using SparseGraphW = SparseGraph_T<WeightedGraphTraits<double>>;
    using DenseGraphWD = DenseGraph_T<WeightedDirectedGraphTraits<double>>;
    using SparseGraphWD = SparseGraph_T<WeightedDirectedGraphTraits<double>>;

	// Компаратор по убыванию веса узла.
	template<typename T> struct WeightLess {
		bool operator()( const T& lhs, const T& rhs ) const { return lhs.weight < rhs.weight; }
	};
	
	// Компаратор по возрастанию веса узла.
	template<typename T> struct WeightGreater {
		bool operator()( const T& lhs, const T& rhs ) const { return lhs.weight > rhs.weight; }
	};

} // namespace graph

// Специализация for_iter_t для итерации по смежным вершинам взвешенных графов.
// Для контекстов, производных от WeightedGraphTraits.
template <class T, class C> class for_iter_t <T, C,
typename enable_if<is_base_of<Graph::WeightedGraphTraits<typename T::value_type>, C>::value>::type> {
    T& t;
    size_t pos;
    using Weight = typename C::WeightType;
    const Weight EPS = std::numeric_limits<Weight>::epsilon();
public:
    for_iter_t(T& t) : t(t), pos(0) { if(!t[pos]) ++*this; }
    typename C::AdjListNodeType operator*() {
        Weight w = t[pos];
        if (std::abs(w) <= EPS) w = 0.; // Поправка на нулевой вес.
        return {pos, w};
        
    }
    bool operator != (const for_iter_t& f) const { assert(&f.t == &t); return pos != t.size(); }
    void operator++() { while(++pos < t.size() && t[pos] == 0); }
};

#endif /* weightedGraph_h */

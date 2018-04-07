//
//  disjointSet.h
//  Graphs
//
//  Created by Oleg Bakharev on 16/03/2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//
// Класс неперсекающихся динамических множеств

#ifndef disjointSet_h
#define disjointSet_h

#include <vector>

// Лес непересекающихся множеств на основе деревьев связанных элементов с эвристиками объединения по рангу и сжатия пути.
// Кормен Глава 21. (Седжвик 4.11) амортизированно ~O(1).
class DisjointSet {
    std::vector<size_t> _root; // Дерево множества - элементы корни деревьев связных множеств.
    std::vector<size_t> _rank; // Ранг элемента (высота дерева) - ~log[n], n - кол-во дочерних элементов.
    
    // Объединяет два связных множество в одно, реализуя эвристику по рангу.
    // x, y - корни непересекающихся связных множеств.
    void link_(size_t x, size_t y) {
        if (_rank[x] > _rank[y]) {
            _root[y] = x;
        } else {
            _root[x] = y;
            if (_rank[x] == _rank[y]) {
                ++_rank[y];
            }
        }
    }
    
public:
    DisjointSet(size_t n) : _rank(n) {
        _root.reserve(n);
        while (n-- != 0) {
            _root[n] = n;
        }
    }

    bool isConnected(size_t x, size_t y) {
        return find(x) == find(y);
    }
    
    // Возвращает представителя связного множества - корень дерева связности, реализуя эвристку сжатия пути.
    size_t find(size_t x) {
        while (x != _root[x]) {
            x = _root[x];
        }
        return x;
    }
    
    // Связывает два связных множества в которых находятся элементы x и y в одно.
    // Возвращает true если объединение произошло, false - если элементы уже находятся в одном связном множестве.
    bool uniteIfNotConnected(size_t x, size_t y) {
        x = find(x);
        y = find(y);
        if (x == y) {
            return false;
        }
        link_(x, y);
        return true;
    }
};

#endif /* disjointSet_h */

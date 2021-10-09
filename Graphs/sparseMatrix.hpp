//
//  sparseMatrix.hpp
//  Graphs
//
//  Created by Oleg Bakharev on 05/02/2018.
//  Copyright © 2018 Oleg Bakharev. All rights reserved.
//

#ifndef sparseMatrix_hpp
#define sparseMatrix_hpp

#include <iostream>
#include "sparseArray.hpp"
#include "matrix.h"
#include <algorithm>
#include <limits>
#include <functional>

using offset_function = std::function<size_t()>;

// Специализация for_iter_t для итерации по элементам разреженной матрицы.
// Использует SparseArray lowerCol/Row для инициализации итерации.
template <typename T, typename Context, typename Enable>
class for_iter_t <basic_slice_iter<SparseArray<T>, Context>, Context, Enable> {
    using slice_iter = basic_slice_iter<SparseArray<T>, Context>;
    using container = SparseArray<T>;
    using iterator = typename container::iterator;
    using item_type = typename container::item_type;
    using reference = item_type&;

    slice_iter& iter;
    iterator pos; // Итератор разреженного вектора.
public:
    for_iter_t(const slice_iter& iter) : iter(const_cast<slice_iter&>(iter)),
                                         pos(iter.v.begin() + iter.getOffset())
    {
        // Вызов iter.v.begin() гарантированно отсортирует ссылки в разреженном массиве.
        // Что повлечёт за собой вызов  соотв. события и вычисление отступов строк и столбцов.
        // И после этого мы получим свой отступ через getOffset.
    }

    reference operator*() { return *pos; }
    bool operator != (const for_iter_t& end) const {
        // У итератора нет оператора == но есть оператор !=.
        if (!(pos != iter.v.end())) return false; // Проверка переполнения разреженного массива на последнем элементе.
        size_t last = end.iter.upper_index();
        size_t first = pos->first;
        return first < last; }
    void operator++() { ++pos; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Задача специализации итератора: передать SparseArray lowerCol/Row в for_iter
template<typename T, typename Context> class basic_slice_iter<SparseArray<T>, Context> {
    using Container = SparseArray<T>;
    using ForIter = for_iter_t<basic_slice_iter, Context>;
    Container& v;
    slice s;
    offset_function getOffset;
    
    // Вместо T& используем typename vector<T>::reference для совместимости с vector<bool>
    typename Container::reference ref(size_t i) const { return (v)[s.start() + i * s.stride()]; }
    friend ForIter;
public:
    using value_type = typename Container::value_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;
    basic_slice_iter(Container& v, slice s, const offset_function& f) : v(v), s(s), getOffset(f) {}
    
    // Заменитель конструктора для константных экземпляров. Обычный конструктор "возвратил бы" не const итератор.
    static const basic_slice_iter ct(const Container& v, slice s, const offset_function& f) {
        return basic_slice_iter( const_cast<Container&>(v), s, f );
    }
    
    size_t size() const { return s.size(); }
    const_reference operator[](size_t i) const { return ref(i); }
    reference operator[](size_t i) { return ref(i); }
    
    size_t lower_index() const { return s.start(); }
    size_t upper_index() const { return s.start() + s.size() * s.stride(); }
    
    // Для for(:)
    ForIter begin() { return {*this}; }
    ForIter end() { return {*this}; }
    ForIter begin() const { return {*this}; }
    ForIter end() const { return {*this}; }
    
    // Вывод в поток
    friend std::ostream& operator << (std::ostream& os, const basic_slice_iter<Container, Context>& v) {
        for (auto x : v ) {
            os << setw(2) << x << ", ";
        }
        return os << endl;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Context>
class basic_matrix<SparseArray<T>, Context> {
    using item_type = typename SparseArray<T>::item_type;
    
    size_t _h;
    size_t _w;
    SparseArray<T> _m;
    
    // Для i-й строки индекс первой ячейки в разреженном массиве.
    std::vector<size_t> _lowerCol;
    // Для j-го столбца индекс первой ячейки в разреженном массиве.
    std::vector<size_t> _lowerRow;
    
    void init_() {
        // Как показал каллстек. Анонимные функции вызываются в два раза быстрее чем bind.
        _m.order_did_fix += [this]{ this->calculate_rowcols_(); };//std::bind(&basic_matrix::calculate_rowcols_, &*this);
    }
    
    void calculate_rowcols_() {
        // Вычисляем отступы столбцов и строк. отступ - первая имеющаяся ячейка в строке/столбце матрицы.
        size_t INF = _m.size();
        _lowerCol.assign(_h, INF);
        _lowerRow.assign(_w, INF);
        size_t i = 0;
        for (item_type& item : _m) {
            size_t pos = item.first;
            size_t row = pos / _w;
            size_t col = pos % _w;
            if (_lowerCol[row] == INF) _lowerCol[row] = i;
            if (_lowerRow[col] == INF) _lowerRow[col] = i;
            i++;
        }
    }
    
    offset_function colOffset(size_t row) const {
        return [this, row] {
            return this->_lowerCol[row];
        };
    }
    
    offset_function rowOffset(size_t col) const {
        return [this, col] {
            return this->_lowerRow[col];
        };
    }
    
public:
    using context_type = Context;
    using container = SparseArray<T>;
    using vec = basic_slice_iter<container, Context>;
    using value_type = vec;
    using reference = vec; // vec это и value_type и reference.
    using const_reference = const vec;
    
    basic_matrix(size_t w, size_t h) : _h(h), _w(w), _m(w * h) { init_(); }
    basic_matrix(size_t w, size_t h, T def) : _h(h), _w(w), _m(w * h, def) { init_(); }
    
    basic_matrix(const basic_matrix&) = delete;
    
    basic_matrix(initializer_list<initializer_list<T>> l) : _h(l.size()),
                                                            _w(_h > 0 ? l.begin()->size() : 0),
                                                            _m(_h * _w)
    {
        size_t pos = 0;
        for( initializer_list<T> const& rowList : l ) {
            assert(rowList.size() == _w);
            for( const T& value : rowList) {
                _m[pos] = value;
                pos++;
            }
        }
        init_();
    }
    
    size_t w() const { return _w; }
    size_t h() const { return _h; }
    
    vec col(size_t x) { return vec( _m, slice(x, _h, _w), rowOffset(x) ); }
    const vec col(size_t x) const { return vec::ct( _m, slice(x, _h, _w), rowOffset(x) ); }
    
    vec row(size_t y) { return vec( _m, slice( y * _w, _w, 1), colOffset(y) ); }
    const vec row(size_t y) const { return vec::ct( _m, slice(y * _w, _w, 1), colOffset(y) ); }
    
    vec operator[] (size_t y) { return row(y); }
    const vec operator[] (size_t y) const { return row(y); }
    
    size_t size() const { return _h; }
    
    // Для for(:)
    auto begin() { return for_iter(*this); }
    auto end() { return for_iter(*this); }
    auto begin() const { return for_iter(*this); }
    auto end() const { return for_iter(*this); }
    
    // Вывод в поток
    friend std::ostream& operator << (std::ostream& os, const basic_matrix<container, Context>& m) {
        for( auto row : m ) {
            os << row;
        }
        return os << "\n\n";
    }
};

template <typename T, typename Context = void>
using SparseMatrix = basic_matrix<SparseArray<T>, Context>;

void testSparseMatrix();

#endif /* sparseMatrix_hpp */

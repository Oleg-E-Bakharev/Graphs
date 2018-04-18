//
//  matrix.h
//  Sorting
//
//  Created by Oleg Bakharev on 30/03/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef matrix_h
#define matrix_h

#include <iostream>
#include <iomanip>
#include <valarray>
#include <vector>
#include <cassert>
#include <initializer_list>
#include "for_iter.h"

using namespace std;

// А-ля Страуструп: http://www.stroustrup.com/matrix.c
template<typename Container, typename Context = void> class basic_slice_iter {
    using ForIter = for_iter_t<basic_slice_iter, Context>;
	Container& v;
	slice s;
	// Вместо T& используем typename vector<T>::reference для совместимости с vector<bool>
	typename Container::reference ref(size_t i) const { return (v)[s.start() + i * s.stride()]; }
    friend ForIter;
public:
    using value_type = typename Container::value_type;
	using reference = typename Container::reference;
	using const_reference = typename Container::const_reference;
	basic_slice_iter(Container& v, slice s) : v(v), s(s) {}
	
	// Заменитель конструктора для константных экземпляров. Обычный конструктор "возвратил бы" не const итератор.
	static const basic_slice_iter ct(const Container& v, slice s) { return basic_slice_iter( const_cast<Container&>(v), s ); }
	
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
template <typename T, typename Context = void>
using slice_iter = basic_slice_iter<std::vector<T>, Context>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename Container, typename Context = void>
class basic_matrix {
	size_t _h;
    size_t _w;
	Container _m;
	
public:
    using T = typename Container::value_type;
	using vec = basic_slice_iter<Container, Context>;
	using value_type = vec;
	using reference = vec; // vec это и value_type и reference.
	using const_reference = const vec;
	
    basic_matrix(size_t h, size_t w) : _h(h), _w(w), _m(w * h) {}
    basic_matrix(size_t h, size_t w, T def) : _h(h), _w(w), _m(w * h, def) {}
    
    basic_matrix(const basic_matrix&) = default;
	
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
	}
	
	size_t w() const { return _w; }
	size_t h() const { return _h; }
	
	vec col(size_t x) { return vec( _m, slice(x, _h, _w) ); }
	const vec col(size_t x) const { return vec::ct( _m, slice(x, _h, _w) ); }
	
	vec row(size_t y) { return vec( _m, slice( y * _w, _w, 1) ); }
	const vec row(size_t y) const { return vec::ct( _m, slice(y * _w, _w, 1) ); }
	
	vec operator[] (size_t y) { return row(y); }
	const vec operator[] (size_t y) const { return row(y); }
		
	size_t size() const { return _h; }
	
	// Для for(:)
    for_iter_t<basic_matrix> begin() { return for_iter(*this); }
    for_iter_t<basic_matrix> end() { return for_iter(*this); }
    for_iter_t<const basic_matrix> begin() const { return for_iter(*this); }
    for_iter_t<const basic_matrix> end() const { return for_iter(*this); }
	
	// Вывод в поток
	friend std::ostream& operator << (std::ostream& os, const basic_matrix<Container, Context>& m) {
		for( auto row : m ) {
            os << row;
		}
		return os << "\n\n";
	}
	
	//	typename vec::reference operator () (size_t x, size_t y) { return _m[ _w * y + x ]; }
};

template <typename T, typename Context = void>
using matrix = basic_matrix<std::vector<T>, Context>;

#endif /* matrix_h */

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
template<typename T, typename Context = void> class slice_iter {
	typedef vector<T> VT;
	VT& v;
	slice s;
	// Вместо T& используем typename vector<T>::reference для совместимости с vector<bool>
	typename VT::reference ref(size_t i) const { return (v)[s.start() + i * s.stride()]; }
    
public:
	typedef T value_type;
	typedef typename VT::reference reference;
	typedef typename VT::const_reference const_reference;
	slice_iter( VT& v, slice s ) : v(v), s(s) {}
	
	// Заменитель конструктора для константных экземпляров. Обычный конструктор "возвратил бы" не const итератор.
	static const slice_iter ct(const VT& v, slice s) { return slice_iter( const_cast<VT&>(v), s ); }
	
	size_t size() const { return s.size(); }
	const_reference operator[](size_t i) const { return ref(i); }
	reference operator[](size_t i) { return ref(i); }
	
	// Для for(:)
	for_iter_t<slice_iter, Context> begin() { return for_iter_t<slice_iter, Context>(*this); }
	for_iter_t<slice_iter, Context> end() { return for_iter_t<slice_iter, Context>(*this); }
	for_iter_t<const slice_iter, Context> begin() const { return for_iter_t<slice_iter, Context>(*this); }
	for_iter_t<const slice_iter, Context> end() const { return for_iter_t<slice_iter, Context>(*this); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Context = void>
class matrix {
	size_t _w;
	size_t _h;
	vector<T> _m;
	
public:
	using vec = slice_iter<T, Context>;
	using value_type = vec;
	using reference = vec; // vec это и value_type и reference.
	using const_reference = const vec;
	
	matrix(size_t w, size_t h) : _w(w), _h(h), _m(w * h) {}
    matrix(size_t w, size_t h, T def) : _w(w), _h(h), _m(w * h, def) {}
    
    matrix(const matrix&) = default;
	
	matrix(initializer_list<initializer_list<T>> l) {
		_h = l.size();
		_w = _h > 0 ? l.begin()->size() : 0;
		_m.resize( _w * _h );
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
	const vec row(size_t y) const { return vec::ct( _m, slice( y * _w, _w, 1) ); }
	
	vec operator[] (size_t y) { return row(y); }
	const vec operator[] (size_t y) const { return row(y); }
		
	size_t size() const { return _h; }
	
	// Для for(:)
	for_iter_t<matrix> begin() { return for_iter(*this); }
	for_iter_t<matrix> end() { return for_iter(*this); }
	for_iter_t<const matrix> begin() const { return for_iter(*this); }
	for_iter_t<const matrix> end() const { return for_iter(*this); }
	
	// Вывод в поток
	friend std::ostream& operator << (std::ostream& os, const matrix<T, Context>& m) {
		for( auto row : m ) {
			for (auto x : row ) {
				cout << setw(2) << x << ", ";
			}
			cout << endl;
		}
		cout << "\n\n";
		return os;
	}
	
	//	typename vec::reference operator () (size_t x, size_t y) { return _m[ _w * y + x ]; }
};

#endif /* matrix_h */

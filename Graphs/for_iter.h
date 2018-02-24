//
//  for_iter.h
//  Sorting
//
//  Created by Oleg Bakharev on 08/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef for_iter_h
#define for_iter_h

// Универсальный итератор, который преобразовывает любой random-access-iterable класс в sequence-iterable-class
// Для использования в range-based for
// Для использования T должен поддерживать [size_t] и size()
// А также тип reference как ссылка на хранимый тип.
// Через контекст можно специализировать итератор для специальных целей (напр в матрице смежности графа).
template <typename T, typename Context = void, typename Enable = void> class for_iter_t {
    T& t;
    size_t pos;
public:
    // const_cast - для возможности работать и с константными и с неконстантными итераторами.
    for_iter_t(const T& t) : t(const_cast<T&>(t)), pos(0) {}
    typename T::reference operator*() { return t[pos]; }
    bool operator != (const for_iter_t& f) const { assert(&f.t == &t); return pos != t.size(); }
    void operator++() { ++pos; }
};

// Функция для инстанциирования for_iter_t чтобы параметр шаблона вывелся компилятором.
template<class T> for_iter_t<T> for_iter(T& t) { return for_iter_t<T>(t); };

#endif /* for_iter_h */

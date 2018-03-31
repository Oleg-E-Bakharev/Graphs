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
// Для использования Collection должен поддерживать [size_t] и size()
// А также тип reference как ссылка на хранимый тип.
// Через контекст можно специализировать итератор для специальных целей (напр в матрице смежности графа).
template <typename Collection, typename Context = void, typename Enable = void> class for_iter_t {
    Collection& _col;
    size_t _pos;
public:
    // const_cast - для возможности работать и с константными и с неконстантными итераторами.
    for_iter_t(const Collection& col) : _col(const_cast<Collection&>(col)), _pos(0) {}
    typename Collection::reference operator*() { return _col[_pos]; }
    bool operator != (const for_iter_t& f) const { assert(&f._col == &_col); return _pos != _col.size(); }
    void operator++() { ++_pos; }
};

// Функция для инстанциирования for_iter_t чтобы параметр шаблона вывелся компилятором.
template<class Collection> for_iter_t<Collection> for_iter(Collection& col) { return for_iter_t<Collection>(col); };

#endif /* for_iter_h */

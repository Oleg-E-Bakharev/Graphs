//
//  for_iter.h
//  Sorting
//
//  Created by Oleg Bakharev on 08/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef for_iter_h
#define for_iter_h

/**
 Универсальный итератор, который преобразовывает любой random-access-iterable класс в sequence-iterable класс
 Для использования в range-based for
 Для использования T должен поддерживать индексатор [size_t] и метод size()
 А также тип reference как ссылка на хранимый тип.
 Применение:
 В классе-контенере реализовать методы begin() и end() как:
 auto begin() { return for_iter(*this); }
 auto end() { return for_iter(*this); }
 **/

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

///// Механизм создания различных типов for_iter_t в зависимости от наличия в Collection типа Context
//template <typename Collection, typename Context = void> struct for_iter_builder {
//    /// Если в Collection не определён тип Context то будет вызван этот метод.
//    static inline auto for_iter_build(Collection& col) {
//        return for_iter_t<Collection, Context>(col);
//    }
//};
//
//template <typename Collection> struct for_iter_builder<Collection, typename Collection::context_type> {
//    /// Если в Collection определён тип Context то будет вызван этот метод.
//    static inline auto for_iter_build(Collection& col) {
//        return for_iter_t<Collection, typename Collection::context_type>(col);
//    }
//};
//
//template <typename Collection>
//inline auto for_iter(Collection& col) { return for_iter_builder<Collection>::for_iter_build(col); }

// Функция для инстанциирования for_iter_t чтобы параметр шаблона вывелся компилятором.
template<class Collection> auto for_iter(Collection& col) { return for_iter_t<Collection>(col); };

#endif /* for_iter_h */

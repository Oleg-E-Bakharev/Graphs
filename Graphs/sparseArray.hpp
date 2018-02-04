// Copyright (С) 2018 Oleg Bakharev. All rights reserved.
// Description: Разреженный массив.

#ifndef SparseArray_hpp
#define SparseArray_hpp

#include <iostream>
#include <vector>
#include <initializer_list>
#include <assert.h>

// Разреженный массив на основе хэш таблицы с открытой адресацией.
// Можно использовать как будто массив изначально заполнен значениями по умолчанию для хранимого типа,
// например нулями для числовых типов.
// Доступ к элементам за O(1).
// Итерация по элементам в порядке возрастания ключей за O(1).
// В процессе итерации можно как угодно изменять значения элементов - но не их индексы!
// Между операцией вызывающей вставку элемента и итерированием автоматически происходит сортировка ссылок O(Klog(K)) -
// K - количество задействованых элементов массива.
// Идеальное решение для стортировок подсчётом и построения гистограмм.
// Кстати, итератор - с произвольным доступом.
template <typename T> class SparseArray {
	
	// Элемент таблицы.
	// first - индекс элемента в массиве. Изменять недопустимо! Хотя возможно и это недоделка.
	// second - значение элемента. Можно менять как угодно.
	struct Item : std::pair<size_t, T> { // Пара индекс - значение.
		using base = std::pair<size_t, T>;
		
		Item() = default;
		Item(const Item&) = default;
		Item(const size_t& k, const T& t) : base(k, t) {}
		Item(const base& b) : base(b) {}
		
        bool operator<(const Item& i) const { return base::first < i.first; }
		
        friend std::ostream& operator << (std::ostream& os, const Item& it) {
            return os << "{" << it.first << ", " << it.second << "}"; }
	};
	
    // Буфер данных c двукратным резервированием. Внутри себя реализует хеш-таблицу с открытой адресацией с
    // линейным пробированием. Рзмер таблицы всегда простое число. Максимальный  размер 2^32 степени.
    class Data {
        std::vector<Item> _data; // Массив данных.
        std::vector<bool> _status; // Флаги занятости ячеек _data.
        
        using References = std::vector<std::reference_wrapper<Item>> ;
        References _refs;// Cсылки на элементы _data в отсортированном порядке.
        
        bool _fixed = false; // Флаг соотвтствия _data и _refs.
        
        inline size_t hash_(size_t pos) const { return pos % _data.size(); }

        // Очень желательно, чтобы размер таблицы был простым числом, тогда остатки от деления хэшкодов на размер таблицы будут
        // максимально распределены по таблице. Возвращаем ближайшее простое число к ближайшей степени 2, меньшей указанного
        // количества.
        size_t tableSize_(size_t itemsCount) const {
            if (itemsCount <= 32) return 31;
            // Вычисляем log2(itemsCount) для этого находим старший ненулевой разряд.
            size_t i = (sizeof(itemsCount) << 3) - 1;
            for (; i > 0; i--) {
                if ((itemsCount >> i & 1) != 0) break;
            }
            assert(i > 5 && i < 32);
            // Массив чисел, вычитание которых из степени 2 (начиная с 6) даёт простое число.
            // Взято из Седжвика "Алгоритмы на Java" табл. 3.4.2
            static size_t Deltas[] = {3, 1, 5, 3, 3, 9, 3, 1, 3, 19, 15, 1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1};
            size_t prime = (1 << i) - Deltas[i - 6];
            return prime;
        }

        void fix_() {
            /// Сортирует ссылки для последовательной итерации.
            if (!_fixed) {
                std::sort(_refs.begin(), _refs.end(), std::less<Item>());
                _fixed = true;
            }
        }
        
        // Предотвращаем копирование и присваивание.
        Data(const Data&) = delete;
        void operator=(const Data&) = delete;
        
    public:
        Data(size_t size) : _data(tableSize_(size << 1)), _status(_data.size()) {}
        
        // Разрешаем перемещение.
        Data(Data&& d) : _data(std::move(d._data)), _status(std::move(d._status)), _refs(std::move(d._refs)), _fixed(d._fixed)
        {
        }

        void operator=(Data&& d) {
            _data = std::move(d._data);
            _status = std::move(d._status);
            _refs = std::move(d._refs);
            _fixed = d._fixed;
        }
        
        size_t size() const { return _refs.size(); }
        
        // Кладёт значение. Возвращает true если буфер пора увеличивать.
        bool put(size_t pos, const T& t) {
            size_t i = hash_(pos);
            for ( ; _status[i]; i = (i + 1) % _data.size()) {
                if (_data[i].first == pos) {
                    _data[i].second = t;
                    return false;
                }
            }
            assert(_refs.size() <= _data.size() >> 1);
            _fixed = false;
            _data[i] = {pos, t};
            _status[i] = true;
            _refs.push_back(_data[i]);
            return  (_refs.size() >= _data.size() >> 1);
        }
        
        Item& get(size_t pos, bool& shouldGrow) {
            shouldGrow = false;
            for (size_t i = hash_(pos); _status[i]; i = (i + 1) % _data.size()) {
                if (_data[i].first == pos) return _data[i];
            }
            // Создаём пустой элемент и возвращаем его.
            shouldGrow = put(pos, T());
            return _refs.back();
        }
        
        using iterator = typename References::iterator;

        iterator begin() {
            fix_();
            return _refs.begin();
        }
        
        iterator end() {
            fix_();
            return _refs.end();
        }
        
        // Возвращает заполненный своей копией буфер с вдвое увеличенным размером.
        Data grow() {
            Data data(_data.size() << 1);
            // В данном случае нам не важен порядок ссылок, по-этому не вызываем fix_()
            for (auto i = _refs.begin(); i != _refs.end(); ++i) {
                Item& item = *i;
                data.put(item.first, item.second);
            }
            return data;
        }
    };
    
    Data _data;
    
	// Ссылка.
    class Reference {
        SparseArray& _parent;
        size_t _pos;
        friend class SparseArray;
        
        Reference (SparseArray& parent, size_t pos)  : _parent(parent), _pos(pos) {}
        
    public:
        operator T&() {
            bool shouldGrow = false;
            Item& ptr = _parent._data.get(_pos, shouldGrow);
            if (shouldGrow) {
                _parent._data = std::move(_parent._data.grow());
                ptr = _parent._data.get(_pos, shouldGrow);
                assert(!shouldGrow);
            }
            return ptr.second;
        }
        
        void operator=(const T& value) {
            if (_parent._data.put(_pos, value)) {
                // Здесь произойдёт не копирование (оно запрещено) а перемещение.
                _parent._data = _parent._data.grow();
            }
        }
    };

	// Итератор с произвольным доступом.
	class Iterator {
		using Pos = typename std::vector<std::reference_wrapper<Item>>::iterator;
		Pos _pos;
		Iterator(const Pos& pos) : _pos(pos) {}
		friend class SparseArray;
	public:
		Item& operator*() const {return *_pos;}
		Item* operator->() const {return _pos;}
		void operator++() { ++_pos; }
		bool operator!=(const Iterator& it) const { return _pos != it._pos; }
		Iterator operator+(size_t offset) const { return Iterator(_pos + offset); }
	};
	
public:
    using value_type = Item;
	using iterator = Iterator;
	using reference = Reference;

	SparseArray(size_t size = 0) : _data(size) {}
	
	// Количество используемых элементов.
    size_t size() const { return _data.size(); }
	
	// Доступ к элементам по ссылке.
	reference operator[](size_t pos) { return Reference(*this, pos); }
	
	// Начальный итератор.
    iterator begin() { return _data.begin(); }
	
	// Конечный итератор.
    iterator end() { return _data.end(); }
};

void testSparseArray();
#endif /*SparseArray_hpp */
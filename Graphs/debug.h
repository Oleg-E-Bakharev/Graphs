//
//  debug.h
//  Graphs
//
//  Created by Oleg Bakharev on 23/04/16.
//  Copyright © 2016 Oleg Bakharev. All rights reserved.
//

#ifndef debug_h
#define debug_h

template<typename Array>
typename std::enable_if<
std::is_same<Array, std::vector<typename Array::value_type>>::value
|| std::is_same<Array, std::valarray<typename Array::value_type>>::value
|| std::is_same<Array, std::array<typename Array::value_type, sizeof(Array) / sizeof(typename Array::value_type)>>::value
, std::ostream &>::type
operator<< (std::ostream& out, const Array& v) {
    out << "[";
    auto space = "";
    for (auto i = 0; i < v.size(); i++) {
        std::cout << space;
        std::cout << setw(2) << v[i];
        space = " ";
    }
    out << "]" << std::endl;
    return out;
}

#ifdef DEBUG
    #define	trace(s)	(cout << endl << s << endl)
	#define debug(s)	s
#else
    #define	trace(s)
	#define debug(s)
#endif

#endif /* debug_h */

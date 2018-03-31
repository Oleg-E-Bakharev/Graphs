//
//  hashArray.cpp
//  Sorting
//
//  Created by Oleg Bakharev on 26.11.2017.
//  Copyright © 2017 Oleg Bakharev. All rights reserved.
//

#include "sparseArray.hpp"
#include <string>

using namespace std;

//std::ostream& operator << (std::ostream& os, const SparseArray<wchar_t>::value_type& it) {
//    // Для упрощения в вывод отладочной консоли.
//    return os << "{" << char(it.first) << ", " << it.second << "}";
//}

void testSparseArray()
{
    wstring str = L"asdfjkafhjnvjncmmmriutiuyq[powitcjmdvitjnacoiptnvncihdvnnvkjzhngbnvzndds;hlnjkghnmfcdvnontvz;td";
	SparseArray<wchar_t> ha;
	for (auto wc : str) {
		ha[wc]++;
	}
    
    ha['Z'] = 10;
	
    cout << ha;

	cout << "\nMediane: " << *(ha.begin() + ha.size() / 2) << "\n";
}

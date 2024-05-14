#include "multiset.h"
#include <iostream>
#include <cstdio>
#include <set>

class xint {
private:
    bool copy;
    int x;
public:
    xint(int x) : copy(false), x(x) {}
    xint(const xint &t): copy(true), x(t.x){}
    bool operator<(const xint &rhs) const {return x < rhs.x;}
    bool operator==(const xint &rhs) const {return x == rhs.x;}
    operator int() const {return x;}
    xint operator=(const xint &rhs) = delete;
    ~xint() {
        if(copy)
            printf("Destruct %d\n", x);
    }
    friend std::ostream &operator<<(std::ostream &os, const xint &t) {
        os << t.x;
        return os;
    }
};

template <class T = YX::multiset<int>>
int maint(){
    T ms({100, 98, 99, 97, 96, 59, 58, 58, 58,
    58, 58, 58, 58, 1, 2, 3, 4, 5, 5,
    58, 5, 5, 6, 7, 58, 32, 77, 80, 58});
    printf("Traversal Test: ");
    for(auto i = ms.begin(); i != ms.end(); i++){
        std::cout << *i << " ";
    }
    std::cout << std::endl;
    printf("Reverse Traversal Test: ");
    for(auto i = ms.rbegin(); i != ms.rend(); i++){
        std::cout << *i << " ";
    }
    std::cout << std::endl;
    printf("Const Traversal Test: ");
    const T & ms2 = ms;
    for(auto i = ms2.cbegin(); i != ms2.cend(); i++){
        std::cout << *i << " ";
    }
    std::cout << std::endl;
    printf("Size Test: ");
    std::cout << ms.size() << std::endl;
    printf("Find Test: \n");
    if(ms.find(59) != ms.end()){
        std::cout << "100 is in the multiset." << std::endl;
    }
    if(ms.find(101) == ms.end()){
        std::cout << "101 is not in the multiset." << std::endl;
    }
    auto ft = ms.find(58);
    
    printf("Erase Test: \n");
    ms.erase(ft);
    ms.erase(5);
    ms.erase(58);
    ms.erase(6);
    ms.erase(7);
    ms.erase(98);
    ms.erase(77);
    
    printf("Size: %d\n", (int)ms.size());
    printf("Advance Test: \n");
    auto it = ms.begin();
    std::advance(it, 7);
    printf("The 8th element is %d\n", (int)*it);
    printf("Distance Test: \n");
    std::cout << std::distance(it, ms.end()) << std::endl;
    printf("Traversal Test: ");
    for(auto i = ms.cbegin(); i != ms.cend(); i++){
        std::cout << *i << " ";
    }
    std::cout << std::endl;
    printf("Clear Test: \n");
    ms.clear();
    std::cout << ms.size() << std::endl;
    return 0;
}

int main(){
    printf("=================YX::multiset====================\n");
    maint<YX::multiset<xint>>();
    printf("=================std::multiset====================\n");
    maint<std::multiset<xint>>();
    printf("================================================\n");
    return 0;
}
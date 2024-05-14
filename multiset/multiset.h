// -*- C++ -*-
//===----------------------------------------------------------------------===//
// Version 1.0
// By Yi Xiang
// Multiset implementation (AVL Tree)
// Part of Y.X's C++ Standard Template Library (XY STL)
// (c) 2024
//===----------------------------------------------------------------------===//

#pragma once

#include <limits>
#include <initializer_list>
#include <memory>
#include <iterator>
#include <cstddef>

namespace YX {

template <class _Tp, class _Alloc = std::allocator<_Tp>>
struct _Node{
    _Tp* data;
    _Node* left;
    _Node* right;
    _Node* parent;
    _Node* next;
    _Node* prev;
    typedef _Alloc allocator_type;
    allocator_type alloc;
    _Node(): left(nullptr), right(nullptr), next(nullptr), parent(nullptr), prev(nullptr){}
    _Node(const _Tp& d, _Node* l = nullptr, _Node* r = nullptr, _Node* p = nullptr): 
    left(l), right(r), parent(p), next(nullptr), prev(nullptr) {
        data = alloc.allocate(1);
        alloc.construct(data, d);
    }
    void move_data(_Node* node){
        alloc.destroy(data);
        alloc.deallocate(data, 1);
        data = node->data;
        node->data = nullptr;
    }
    _Tp&& operator*() { return *data; }
    operator _Tp() { return *data; }
    void l(_Node<_Tp> * left) { 
        auto cur = this;
        cur->left = left;
        while(cur->next != nullptr) {
            cur->left = left;
            cur = cur->next;
        }
    }
    void r(_Node<_Tp>* right) { 
        auto cur = this;
        cur->right = right;
        while(cur->next != nullptr) {
            cur->right = right;
            cur = cur->next;
        }
    }
    void p(_Node<_Tp>* parent) { 
        auto cur = this;
        while(cur->next != nullptr) {
            cur->parent = parent;
            cur = cur->next;
        }
    }
    enum {
        BALANCED = 0,
        LL,
        RR,
        LR,
        RL,
        UNKNOWN
    } AVLTYPE;
    size_t height() {
        size_t l = left == nullptr ? 0 : left->height();
        size_t r = right == nullptr ? 0 : right->height();
        return 1 + (l > r ? l : r);
    }
    size_t length(){
        size_t l = 1;
        auto cur = this;
        while(cur->next != nullptr) {
            l++;
            cur = cur->next;
        }
        return l;
    }
    long bfactor() {
        size_t l = left == nullptr ? 0 : left->height();
        size_t r = right == nullptr ? 0 : right->height();
        return (long)r - (long)l;
    }
    int avl_type(){
        if(balance()){
            return BALANCED;
        }
        if(bfactor() == -2){
            if(left->bfactor() == -1){
                return LL;
            }else if(left->bfactor() == 1){
                return LR;
            }
        }else if(bfactor() == 2){
            if(right->bfactor() == -1){
                return RL;
            }else if(right->bfactor() == 1){
                return RR;
            }
        }
        return UNKNOWN;
    }
    inline bool balance(){
        return bfactor() >= -1 && bfactor() <= 1;
    }
    ~_Node(){
        if(data != nullptr) {
            alloc.destroy(data);
            alloc.deallocate(data, 1);
            data = nullptr;
        }
    }
    void clear(){
        if(prev == nullptr){
            if(left != nullptr) {
                left->clear();
                delete left;
                left = nullptr;
            }
            if(right != nullptr) {
                right->clear();
                delete right;
                right = nullptr;
            }
        }
        if (next != nullptr) {
            next->clear();
            delete next;
            next = nullptr;
        }
    }
    void update(){
        if(left != nullptr) {
            left->parent = this;
        }
        if(right != nullptr) {
            right->parent = this;
        }
        if(next != nullptr) {
            next->prev = this;
        }
    }
};

template <class _Tp>
class less {
public:
    less() = default;
    bool operator()(const _Tp& x,const _Tp& y) {
        return x < y;
    }
};

template <class _Tp, class _Compare = std::less<_Tp>, class _Alloc = std::allocator<_Tp>>
class multiset {
private:

typedef _Node<_Tp, _Alloc> Node;
size_t _size;
Node *root;

Node * erase_node(Node * node){
    if(node->left == nullptr && node->right == nullptr) {
        if(!node->parent) {
            root = nullptr;
            return nullptr;
        }else if(node->parent->left == node) {
            node->parent->left = nullptr;
        }else{
            node->parent->right = nullptr;
        }
        auto tmp = node->parent;
        delete node;
        return tmp;
    }else if(node->left == nullptr) {
        if(!node->parent) {
            root = node->right;
            root->parent = nullptr;
        }else if(node->parent->left == node) {
            node->parent->left = node->right;
            node->right->parent = node->parent;
        }else{
            node->parent->right = node->right;
            node->right->parent = node->parent;
        }
        auto tmp = node->right;
        delete node;
        return tmp;
    }else if(node->right == nullptr) {
        if(!node->parent) {
            root = node->left;
            root->parent = nullptr;
        }else if(node->parent->left == node) {
            node->parent->left = node->left;
            node->left->parent = node->parent;
        }else{
            node->parent->right = node->left;
            node->left->parent = node->parent;
        }
        delete node;
        return node->left;
    }else{
        auto tmp = node->right;
        if(tmp->left == nullptr && tmp->right == nullptr) {
            //node->data = tmp->data;
            node->move_data(tmp);
            node->next = tmp->next;
            if(tmp->next != nullptr) {
                tmp->next->prev = node;
            }
            node->prev = tmp->prev;
            node->right = nullptr;
           
        }else if(tmp->left == nullptr) {
            //node->data = tmp->data;
            node->move_data(tmp);
            node->next = tmp->next;
            if(tmp->next != nullptr) {
                tmp->next->prev = node;
            }
            node->prev = tmp->prev;
            node->right = tmp->right;
            tmp->right->parent = node;
        }else{
            while(tmp->left != nullptr) {
                tmp = tmp->left;
            }
            //node->data = tmp->data;
            node->move_data(tmp);
            node->next = tmp->next;
            node->prev = tmp->prev;
            tmp->parent->left = tmp->right;
            if(tmp->right != nullptr) {
                tmp->right->parent = tmp->parent;
            }
            if(node->next != nullptr) {
                node->next->prev = node;
            }
            if(node->prev != nullptr) {
                node->prev->next = node;
            }
        }
        auto walk = node;
        while(true) {
            if(walk == nullptr) {
                break;
            }
            if(!walk->balance()){
                switch(walk->avl_type()){
                    case Node::LL:
                        RotateR(walk);
                        break;
                    case Node::RR:
                        RotateL(walk);
                        break;
                    case Node::LR:
                        RotateLR(walk);
                        break;
                    case Node::RL:
                        RotateRL(walk);
                        break;
                    default:
                        break;
                }
            }
            walk = walk->parent;
        }
        delete tmp;
        return node;
    }
}

Node* update_simple_node(Node* old_node, Node* new_node){
    if(old_node->parent == nullptr) {
        root = new_node;
    }else if(old_node->parent->left == old_node) {
        old_node->parent->left = new_node;
    }else{
        old_node->parent->right = new_node;
    }
    if(new_node != nullptr) {
        new_node->parent = old_node->parent;
    }
    new_node->left = old_node->left;
    if(new_node->left != nullptr) {
        new_node->left->parent = new_node;
    }
    new_node->right = old_node->right;
    if(new_node->right != nullptr) {
        new_node->right->parent = new_node;
    }
    return new_node;
}

protected:

Node* RotateL (Node* & ptr) {
    Node* subL = ptr;
    ptr = subL->right;
    subL->right = ptr->left;
    ptr->left = subL;
    if(subL->parent != nullptr) {
        if(subL->parent->left == subL) {
            subL->parent->left = ptr;
        }else{
            subL->parent->right = ptr;
        }
    }else{
        root = ptr;
    }
    ptr->parent = subL->parent;
    subL->update();
    ptr->update();
    return ptr;
}

Node* RotateR (Node* & ptr) {
    Node* subR = ptr;
    ptr = subR->left;
    subR->left = ptr->right;
    ptr->right = subR;
    if(subR->parent != nullptr) {
        if(subR->parent->left == subR) {
            subR->parent->left = ptr;
        }else{
            subR->parent->right = ptr;
        }
    }else{
        root = ptr;
    }
    ptr->parent = subR->parent;
    subR->update();
    ptr->update();
    return ptr;
}

Node* RotateLR (Node* & ptr) {
    auto subL = ptr->left;
    RotateL(subL);
    RotateR(ptr);
    return ptr;
}

Node* RotateRL (Node* & ptr) {
    auto subR = ptr->right;
    RotateR(subR);
    RotateL(ptr);
    return ptr;
}

public:
    multiset(): _size(0), root(nullptr) {}
    size_t size() const { return _size; }
    bool empty() const { return _size == 0; }
    size_t height() const { return root == nullptr ? 0 : root->height(); }
    size_t max_size() const { return std::numeric_limits<size_t>::max(); }
    void clear() {
        if(root != nullptr) {
            root->clear();
            delete root;
            root = nullptr;
        }
        _size = 0;
    }
    ~multiset() { clear(); }

    multiset(std::initializer_list<_Tp> il): _size(0), root(nullptr) {
        for(auto i = il.begin(); i != il.end(); i++) {
            insert(*i);
        }
    }

    multiset(const multiset& ms): _size(ms._size), root(nullptr) {
        for(auto i = ms.begin(); i != ms.end(); i++) {
            insert(*i);
        }
    }

    multiset(multiset&& ms): _size(ms._size), root(ms.root) {
        ms._size = 0;
        ms.root = nullptr;
    }

    class iterator;

    class iterator: public std::iterator<std::bidirectional_iterator_tag, _Tp> {
    friend class multiset;
    private:
        Node* _node;
        Node* _root;
    public:
        iterator() = default;
        iterator(Node* node, Node * root = nullptr): _node(node), _root(root){}
        iterator(const iterator &) = default;
        iterator& operator=(const iterator&) = default;
        bool operator==(const iterator& val) { return _node == val._node; }
        bool operator!=(const iterator& val) { return _node != val._node; }
        Node* address(){return _node;}
        ~iterator() = default;
        _Tp& operator*() { return *_node->data; }
        void info(){ _node->info();}

        iterator operator++(int){
            iterator tmp = *this; 
            if (!_node) {
                return tmp; 
            }
            if(_node->next != nullptr) {
                _node = _node->next;
                return tmp;
            }
            while(_node->prev != nullptr) {
                _node = _node->prev;
            }
            if (_node->right != nullptr) {
                _node = _node->right;
                while (_node->left != nullptr) {
                    _node = _node->left;
                }
            } else {
                while (_node->parent != nullptr && _node == _node->parent->right) {
                    _node = _node->parent;
                }
                _node = _node->parent; 
            }
            return tmp; 
        }

        iterator operator--(int){
            iterator tmp = *this; 
            if (!_node) {
                if(!_root) {
                    return tmp;
                }
                _node = _root;
                while(_node->right != nullptr) {
                    _node = _node->right;
                }
                while(_node->next != nullptr) {
                    _node = _node->next;
                }
                return tmp;
            }
            if (_node->prev != nullptr) {
                _node = _node->prev;
                return tmp;
            }
            if (_node->left != nullptr) {
                _node = _node->left;
                while (_node->right != nullptr) {
                    _node = _node->right;
                }
            } else {
                while (_node->parent != nullptr && _node == _node->parent->left) {
                    _node = _node->parent;
                }
                _node = _node->parent; 
            }
            while(_node!=nullptr && _node->next != nullptr) {
                _node = _node->next;
            }
            return tmp; 
        }
        iterator operator++(){
            operator++(0);
            return *this;
        }
        iterator operator--(){
            operator--(0);
            return *this;
        }
    };

    class reverse_iterator: public std::iterator<std::bidirectional_iterator_tag, _Tp>{
        private:
            iterator _it;
        public:
            reverse_iterator(iterator it): _it(it){}
            reverse_iterator(const reverse_iterator&) = default;
            reverse_iterator& operator=(const reverse_iterator&) = default;
            bool operator==(const reverse_iterator& val) { return _it == val._it; }
            bool operator!=(const reverse_iterator& val) { return _it != val._it; }
            _Tp& operator*() { return *_it; }
            reverse_iterator operator++(int){
                _it.operator--(0);
                return *this;
            }
            reverse_iterator operator--(int){
                _it.operator++(0);
                return *this;
            }
            reverse_iterator operator++(){
                _it.operator--();
                return *this;
            }
            reverse_iterator operator--(){
                _it.operator++();
                return *this;
            }
    };

    iterator begin() {
        Node* cur = root;
        if(cur == nullptr) {
            return iterator(nullptr);
        }
        while(cur->left != nullptr) {
            cur = cur->left;
        }
        iterator it(cur, root);
        return it;
    }
    iterator end() {
        return iterator(nullptr, root);
    }

    iterator cbegin() const {
        Node* cur = root;
        if(cur == nullptr) {
            return iterator(nullptr);
        }
        while(cur->left != nullptr) {
            cur = cur->left;
        }
        return iterator(cur);
    }

    iterator cend() const {
        return iterator(nullptr, root);
    }

    reverse_iterator rbegin() {
        Node* cur = root;
        while(cur->right != nullptr) {
            cur = cur->right;
        }
        return reverse_iterator(cur);
    }

    reverse_iterator crbegin() const {
        Node* cur = root;
        while(cur->right != nullptr) {
            cur = cur->right;
        }
        return reverse_iterator(cur);
    }

    reverse_iterator rend() {
        return reverse_iterator(nullptr);
    }

    reverse_iterator crend() const {
        return reverse_iterator(nullptr);
    }

    iterator insert(const _Tp& val) {
        _size++;
        Node* ret = nullptr;
        if(root == nullptr) {
            root = new Node(val);
            return iterator(root);
        }else{
            Node* cur = root;
            while(true) {
                if(_Compare()(val, *cur->data)) {
                    if(cur->left == nullptr) {
                        cur->left = new Node(val, nullptr, nullptr, cur);
                        cur->l(cur->left);
                        ret = cur->left;
                        break;
                    }else{
                        cur = cur->left;
                    }
                }else if(_Compare()(*cur->data, val)){
                    if(cur->right == nullptr) {
                        cur->right = new Node(val, nullptr, nullptr, cur);
                        cur->r(cur->right);
                        ret = cur->right;
                        break;
                    }else{
                        cur = cur->right;
                    }
                }else{
                    if(cur->next == nullptr) {
                        cur->next = new Node(val, cur->left, cur->right, cur->parent);
                        cur->next->prev = cur;
                        ret = cur->next;
                        break;
                    }else{
                        cur = cur->next;
                    }
                }
            }
            while(cur -> prev != nullptr) {
                cur = cur->prev;
            }
            auto walk = cur;
            while(true) {
                if(walk == nullptr) {
                    break;
                }
                if(!walk->balance()){
                    switch(walk->avl_type()){
                        case Node::LL:
                            RotateR(walk);
                            break;
                        case Node::RR:
                            RotateL(walk);
                            break;
                        case Node::LR:
                            RotateLR(walk);
                            break;
                        case Node::RL:
                            RotateRL(walk);
                            break;
                        default:
                            break;
                    }
                }
                walk = walk->parent;
            }
        }
        return iterator(ret);
    }

    iterator find(const _Tp& val) {
        Node* cur = root;
        while(cur != nullptr) {
            if(_Compare()(val, *cur->data)) {
                cur = cur->left;
            }else if(_Compare()(*cur->data, val)) {
                cur = cur->right;
            }else{
                return iterator(cur, root);
            }
        }
        return end();
    }

    iterator erase(iterator&& it){
        return erase(it);
    }

    iterator erase(iterator& it){
        Node* cur = it._node;
        auto tmp = cur;
        if(cur == nullptr) {
            return end();
        }
        _size--;
        if(cur->next == nullptr && cur->prev == nullptr) {
            return iterator(erase_node(cur), root);
        }else if(cur->next != nullptr && cur->prev == nullptr){
            cur = cur->next;
            cur->prev = nullptr;
            update_simple_node(tmp, cur);
        }else if(cur->next == nullptr && cur->prev != nullptr){
            cur = cur->prev;
            cur->next = nullptr;
        }else if(cur->next != nullptr && cur->prev != nullptr){
            cur = cur->next;
            cur->prev = tmp->prev;
            tmp->prev->next = cur;
        }
        delete tmp;
        return iterator(cur, root);
    }

    iterator erase(const _Tp& val){
        auto it = find(val);
        if(it == end()) {
            return end();
        }
        auto node = it._node;
        if(node->next == nullptr) {
            return erase(it);
        }else{
            auto tmp = node;
            while(tmp->next != nullptr) {
                tmp = tmp->next;
            }
            while(tmp->prev != nullptr) {
                auto it = iterator(tmp, root);
                auto nx = tmp->prev;
                erase(it);
                tmp = nx;
            }
            return erase(iterator(tmp, root));
        }
    }

    size_t count(const _Tp& val) {
        auto it = find(val);
        if(it == end()) {
            return 0;
        }else{
            auto node = it._node;
            return node->length();
        }
    }
};


}
#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 11 >
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type &;
  using const_reference = const key_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_compare = std::less<key_type>;   // B+-Tree
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing

private:
  struct Element {
    Element* next = nullptr;
    key_type key;

    ~Element(){
      delete next;
    }

  };

  Element** table = nullptr;
  size_type table_size = 0;
  size_type number_of_elements = 0;
  float max_load_factor = 0.7;

  size_type hash(const key_type &k) const {
    return hasher{}(k) % table_size;
  }

  void rehash(const size_type& n){

    Element** old_table = table;
    size_type old_table_size = table_size;

    number_of_elements = 0;

    table_size = n;
    table = new Element*[table_size];

    for(size_type i = 0; i < table_size; ++i){
      table[i] = nullptr;
    }

    Element* tmp = nullptr;

    for (size_type i = 0; i < old_table_size; ++i) {

        tmp = old_table[i];

        while (tmp != nullptr) {

            insert_direct(tmp->key);

            tmp = tmp->next;

        }
    }

    tmp = nullptr;
    delete tmp;


    for(size_type i = 0; i < old_table_size; ++i){
      delete old_table[i];
    }

    delete[] old_table;

  }

  Element *insert_direct(const key_type &k){

    size_type index = hash(k);

    Element *tmp = new Element();

    tmp->next = table[index];
    tmp->key = k;
    table[index] = tmp;

    ++number_of_elements;

    if (table_size * max_load_factor < number_of_elements){
        rehash(table_size * 2);
    }

    tmp = nullptr;
    delete tmp;

    return table[index];

  }

  Element *find_position(const key_type &k) const {

    size_type index = hash(k);

    if (table[index] == nullptr){

        return nullptr;

    } else {

        Element* e = table[index];

        while (e != nullptr){

            if (key_equal{}(e->key, k)){
              return e;
            }

            e = e->next;
        }

        e = nullptr;
        delete e;

    }

    return nullptr;

  }

  void reserve(const size_type& n) {

    if (n > table_size * max_load_factor) {

        size_type new_table_size = table_size;

        do {
            new_table_size = new_table_size * 2 + 1;
        } while (n > new_table_size * max_load_factor);

        rehash(new_table_size);

    }

  }

public:
  ADS_set(){
    rehash(N);

  }

  ADS_set(std::initializer_list<key_type> ilist): ADS_set{}{
    insert(ilist);
  }

  template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{}{
    insert(first, last);
  }

  ADS_set(const ADS_set &other): ADS_set{}{

    reserve(other.number_of_elements);

    for (const auto &k: other) {
      insert_direct(k);
    }

  }

  ~ADS_set(){

    for(size_type i = 0; i < table_size; ++i){
      delete table[i];
    }

    table_size = 0;
    number_of_elements = 0;

    delete[] table;

  }

  ADS_set &operator=(const ADS_set &other){

    if (this == &other) {
        return *this;
    }

    ADS_set tmp = other;
    swap(tmp);

    return *this;

  }

  ADS_set &operator=(std::initializer_list<key_type> ilist){

    ADS_set tmp = ilist;
    swap(tmp);

    return *this;

  }

  size_type size() const{
    return number_of_elements;
  }

  bool empty() const{
    return number_of_elements == 0;
  }

  size_type count(const key_type &k) const{
    return !!find_position(k);
  }

  iterator find(const key_type &k) const{

    size_t index = hash(k);

    Element* e = table[index];

    for (; e != nullptr; e = e->next){

        if (key_equal{}(e->key, k)) {
            return const_iterator{this, e, index, table_size};
        }

    }

    e = nullptr;
    delete e;

    return end();

  }

  void clear(){

    ADS_set tmp;
    swap(tmp);

  }

  void swap(ADS_set &other){

    using std::swap;

    swap(table, other.table);
    swap(number_of_elements, other.number_of_elements);
    swap(table_size, other.table_size);
    swap(max_load_factor, other.max_load_factor);

  }

  void insert(std::initializer_list<key_type> ilist){
    insert(std::begin(ilist), std::end(ilist));
  }

  std::pair<iterator,bool> insert(const key_type &k){

    if(count(k)){
        return std::make_pair(find(k), false);
    } else {
        insert_direct(k);
        return std::make_pair(find(k), true);
    }

  }

  template<typename InputIt> void insert(InputIt first, InputIt last){

    for (auto it = first; it != last; ++it) {

        if (!count(*it)) {

          reserve(number_of_elements+1);
          insert_direct(*it);

        }

    }

  }

  size_type erase(const key_type &k){

    size_type index = hash(k);

    if(find_position(k)){

        if (table[index] != nullptr){

            Element *this_element = table[index];
            Element *before_element = table[index];
            int counter = 0;

            while(this_element != nullptr){

                if (key_equal{}(this_element->key, k)){
                    if (counter == 0){

                        table[index] = this_element->next;

                        --number_of_elements;

                        this_element->next = nullptr;
                        delete this_element;
                        before_element = nullptr;
                        delete before_element;

                        return 1;

                    } else{

                        before_element->next = this_element->next;

                        --number_of_elements;

                        this_element->next = nullptr;
                        delete this_element;
                        before_element = nullptr;
                        delete before_element;

                        return 1;

                    }
                }

                ++counter;

                before_element = this_element;

                this_element = this_element->next;

            }

        }

    }

    return 0;

  }

  const_iterator begin() const{

    for(size_t index = 0; index < table_size; ++index){

        if (table[index] != nullptr) {
            return const_iterator(this, table[index], index, table_size);
        }

    }

    return end();

  }

  const_iterator end() const{
    return const_iterator();
  }

  void dump(std::ostream &o = std::cerr) const{

    o<< "number_of_elements = "<<number_of_elements<< "\n";
    o<< "table_size = " <<table_size<<"\n";

    for (size_type i = 0; i < table_size; ++i) {

         o<< i <<": ";

        if (table[i] == nullptr) {
            o<< i << "..."<<'\n';
            continue;

        } else {

            for (Element* e = table[i]; e != nullptr; e = e->next) {
                o<< e->key;

                if (e->next != nullptr){
                    o<< " -> ";
                }
            }

            o<< '\n';
        }

    }

  }

  friend bool operator==(const ADS_set &lhs, const ADS_set &rhs){

    if (lhs.number_of_elements != rhs.number_of_elements){
        return false;
    }

    for (const auto& a : rhs) {

        if (!lhs.find_position(a)){
            return false;
        }

    }

    return true;

  }

  friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs){
    return !(lhs == rhs);
  }

};


template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type &;
  using pointer = const value_type *;
  using iterator_category = std::forward_iterator_tag;
private:
  const ADS_set<Key, N> *set;
  Element *it_pos;
  size_type it_index;
  size_type it_table_size;
public:
  explicit Iterator(const ADS_set *set = nullptr, Element *it_pos = nullptr, size_type it_index = 0, size_type it_table_size = 0): set(set), it_pos(it_pos), it_index(it_index), it_table_size(it_table_size){

  }

  reference operator*() const{
    return it_pos->key;
  }

  pointer operator->() const{
    return &it_pos->key;
  }

  Iterator &operator++(){

    while (it_index < it_table_size) {

        if (it_pos->next != nullptr) {
            it_pos = it_pos->next;
            return *this;
        } else {
            ++it_index;
        }

        if (it_index == it_table_size) {
            it_pos = nullptr;
            return *this;
        }

        auto a = set->table[it_index];

        if (a != nullptr) {
            it_pos = a;
            return *this;
        }

    }

    return *this;

  }

  Iterator operator++(int){

    auto rc {*this};
    ++*this;

    return rc;

  }

  friend bool operator==(const Iterator &lhs, const Iterator &rhs){
    return lhs.it_pos == rhs.it_pos;
  }

  friend bool operator!=(const Iterator &lhs, const Iterator &rhs){
    return !(lhs == rhs);
  }

};

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) {
  lhs.swap(rhs);
}

#endif // ADS_SET_H

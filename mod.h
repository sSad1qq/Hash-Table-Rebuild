#ifndef MOD_H
#define MOD_H
#include <iostream>
#include <fstream>
#include <random>
using namespace std;

const int MAX_VALUE = 1000000000;

typedef struct node{
    struct node* next;
    int key;
    int value;
} TableNode;

int generate_useful_value();
void create_file_with_useful_value(const char *path, int count);
int read_from_file_to_hashmaps(const char *path, TableNode **Chain, int chainSize,
                               TableNode **Open, int openSize);

int hash_fold(int key, int size);
int hash_double(int key, int i, int size);
void insert_chain(TableNode **Table, int size, int key, int value);
TableNode *find_chain(TableNode **Table, int size, int key);


// Открытая адресация с двойным хешированием
int find_open_index(TableNode **Table, int size, int key);
bool insert_open(TableNode **Table, int size, int key, int value);
TableNode *find_open(TableNode **Table, int size, int key);
bool delete_open(TableNode **Table, int size, int key);
// Возвращает независимую таблицу или nullptr при ошибке (время тогда равно 0).
// После использования вызови clear_open и delete[] для результата.
TableNode **rebuildTable(TableNode *const *Table, int size, long long &elapsedNs);
void print_open(TableNode **Table, int size);
void clear_open(TableNode **Table, int size);

void print_find_result(TableNode *x, int key);

#endif // MOD_H
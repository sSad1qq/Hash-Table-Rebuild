#include "mod.h"
#include <chrono>
#include <new>

// Специальная ячейка для обозначения удалённых элементов при открытой адресации
static TableNode deletedCell;
static TableNode *const DELETED = &deletedCell;

int generate_useful_value(){
    static mt19937 generator(random_device{}());
    uniform_int_distribution<int> dist(0, MAX_VALUE);
    return dist(generator);
}

void create_file_with_useful_value(const char *path, int count){
    ofstream file(path);

    if (!file.is_open()){
        cout << "Не удалось создать файл " << path << '\n';
        return;
    }

    for (int i = 0; i < count; ++i){
        file << generate_useful_value() << ' ' << generate_useful_value() << '\n';
    }
    file.close();
}

TableNode *find_chain(TableNode **Table, int size, int key){
    int index = hash_fold(key, size);

    for (TableNode *cur = Table[index]; cur != nullptr; cur = cur->next){
        if (cur->key == key){
            return cur;
        }
    }
    return nullptr;
}

void insert_chain(TableNode **Table, int size, int key, int value){
    TableNode *x = find_chain(Table, size, key);
    if (x != nullptr){
        x->value = value;
        return;
    }

    int index = hash_fold(key, size);
    TableNode *newNode = new TableNode;
    newNode->key = key;
    newNode->value = value;
    newNode->next = Table[index];
    Table[index] = newNode;
}


int read_from_file_to_hashmaps(const char *path, TableNode **Chain, int chainSize,
                               TableNode **Open, int openSize){
    ifstream file(path);

    if (!file.is_open()){
        cout << "Не удалось открыть файл " << path << '\n';
        return -1;
    }

    int key, value;
    int line = 0;
    int count = 0;
    while (file >> key >> value){
        ++line;
        if (key < 0 || key > MAX_VALUE || value < 0 || value > MAX_VALUE){
            cout << "Строка " << line << ": пара вне диапазона пропущена: "
                 << key << ' ' << value << '\n';
            continue;
        }
        insert_chain(Chain, chainSize, key, value);
        if (!insert_open(Open, openSize, key, value)){
            cout << "Строка " << line << ": таблица с открытой адресацией заполнена, ключ "
                 << key << " не добавлен\n";
        }
        ++count;
    }

    if (!file.eof()){
        cout << "Ошибка формата файла в строке " << line + 1 << '\n';
    }
    file.close();
    return count;
}


int hash_fold(int key, int size){
    int part = 1;
    while (part < size){
        part *= 10;
    }

    int sum = 0;
    while (key > 0){
        sum += key % part;
        key /= part;
    }
    return sum % size;
}


int hash_double(int key, int i, int size){
    int h1 = hash_fold(key, size);
    int h2 = 1 + key % (size - 1);
    return (h1 + i * h2) % size;
}

int find_open_index(TableNode **Table, int size, int key){
    for (int i = 0; i < size; ++i){
        int index = hash_double(key, i, size);
        if (Table[index] == nullptr){
            return -1;
        }
        if (Table[index] != DELETED && Table[index]->key == key){
            return index;
        }
    }
    return -1;
}


bool insert_open(TableNode **Table, int size, int key, int value){
    int found = find_open_index(Table, size, key);
    if (found >= 0){
        Table[found]->value = value;
        return true;
    }

    for (int i = 0; i < size; ++i){
        int index = hash_double(key, i, size);
        if (Table[index] == nullptr || Table[index] == DELETED){
            TableNode *newNode = new TableNode;
            newNode->key = key;
            newNode->value = value;
            newNode->next = nullptr;
            Table[index] = newNode;
            return true;
        }
    }
    return false;
}

TableNode *find_open(TableNode **Table, int size, int key){
    int index = find_open_index(Table, size, key);
    if (index < 0){
        return nullptr;
    }
    return Table[index];
}

bool delete_open(TableNode **Table, int size, int key){
    int index = find_open_index(Table, size, key);
    if (index < 0){
        return false;
    }

    delete Table[index];
    Table[index] = DELETED;
    return true;
}

TableNode **rebuildTable(TableNode *const *Table, int size, long long &elapsedNs){
    elapsedNs = 0;
    if (Table == nullptr || size < 2){
        return nullptr;
    }

    auto start = chrono::steady_clock::now();
    TableNode **Rebuilt = new (nothrow) TableNode*[size]();
    if (Rebuilt == nullptr){
        return nullptr;
    }

    try {
        for (int i = 0; i < size; ++i){
            const TableNode *entry = Table[i];
            if (entry == nullptr || entry == DELETED){
                continue;
            }
            if (!insert_open(Rebuilt, size, entry->key, entry->value)){
                clear_open(Rebuilt, size);
                delete[] Rebuilt;
                return nullptr;
            }
        }
    } catch (const bad_alloc &){
        clear_open(Rebuilt, size);
        delete[] Rebuilt;
        return nullptr;
    }

    auto finish = chrono::steady_clock::now();
    elapsedNs = chrono::duration_cast<chrono::nanoseconds>(finish - start).count();
    return Rebuilt;
}

void print_open(TableNode **Table, int size){
    bool empty = true;
    for (int i = 0; i < size; ++i){
        if (Table[i] == nullptr){
            continue;
        }
        empty = false;
        cout << '[' << i << "] ";
        if (Table[i] == DELETED){
            cout << "DELETED";
        } else {
            cout << '(' << Table[i]->key << ": " << Table[i]->value << ')';
        }
        cout << '\n';
    }
    if (empty){
        cout << "Таблица пуста\n";
    }
}

void clear_open(TableNode **Table, int size){
    for (int i = 0; i < size; ++i){
        if (Table[i] != nullptr && Table[i] != DELETED){
            delete Table[i];
        }
        Table[i] = nullptr;
    }
}

void print_find_result(TableNode *x, int key){
    if (x != nullptr){
        cout << "Ключ " << key << " найден, значение = " << x->value << '\n';
    } else {
        cout << "Ключ " << key << " не найден\n";
    }
}
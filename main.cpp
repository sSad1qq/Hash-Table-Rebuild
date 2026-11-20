#include "mod.h"
#include <algorithm>
#include <chrono>
#include <clocale>
#include <iomanip>
#include <numeric>
#include <vector>

const int TABLE_SIZE = 1013; // Простая ёмкость для двойного хеширования.
const int KEY_COUNT = (TABLE_SIZE * 8 / 10) / 2 * 2;

long long measureSearch(TableNode **Table, const vector<int> &Queries, int &found){
    found = 0;
    auto start = chrono::steady_clock::now();
    for (int key : Queries){
        if (find_open(Table, TABLE_SIZE, key) != nullptr){
            ++found;
        }
    }
    auto finish = chrono::steady_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(finish - start).count();
}

bool compareTables(TableNode **Original, TableNode **Rebuilt, const vector<int> &Keys,
                   mt19937 &generator, int run, long long rebuildNs){
    int deletedCount = KEY_COUNT / 2;
    for (int i = 0; i < KEY_COUNT; ++i){
        for (auto table : {Original, Rebuilt}){
            TableNode *entry = find_open(table, TABLE_SIZE, Keys[i]);
            if (i < deletedCount ? entry != nullptr : !entry || entry->value != i){
                return false;
            }
        }
    }

    uniform_int_distribution<int> missing(0, deletedCount - 1);
    uniform_int_distribution<int> present(deletedCount, KEY_COUNT - 1);
    cout << "\nНабор " << run + 1 << ", перестроение: " << rebuildNs << " нс\n";
    cout << "┌───────────┬─────────┬──────────────┬─────────────────┬──────────────────┐\n"
         << "│   Промахи │ Запросы │    С метками │           После │     Перестроение │\n"
         << "│         % │     шт. │    поиск, нс │       поиск, нс │      + поиск, нс │\n"
         << "├───────────┼─────────┼──────────────┼─────────────────┼──────────────────┤\n";
    for (int missRate : {10, 50, 90}){
        for (int count : {100, 1000, 10000}){
            int misses = count * missRate / 100;
            vector<int> Queries(count);
            for (int i = 0; i < count; ++i){
                int index = i < misses ? missing(generator) : present(generator);
                Queries[i] = Keys[index];
            }
            shuffle(Queries.begin(), Queries.end(), generator);
            int foundOld, foundNew;
            long long oldNs, newNs;
            if (run % 2 == 0){
                oldNs = measureSearch(Original, Queries, foundOld);
                newNs = measureSearch(Rebuilt, Queries, foundNew);
            } else {
                newNs = measureSearch(Rebuilt, Queries, foundNew);
                oldNs = measureSearch(Original, Queries, foundOld);
            }
            if (foundOld != count - misses || foundNew != foundOld){
                return false;
            }
            cout << "│ " << setw(9) << missRate << " │ " << setw(7) << count
                 << " │ " << setw(12) << oldNs << " │ " << setw(15) << newNs
                 << " │ " << setw(16) << rebuildNs + newNs << " │\n";
        }
        if (missRate != 90){
            cout << "├───────────┼─────────┼──────────────┼─────────────────┼──────────────────┤\n";
        }
    }
    cout << "└───────────┴─────────┴──────────────┴─────────────────┴──────────────────┘\n";
    return true;
}

int main(){
    setlocale(LC_CTYPE, "");
    mt19937 generator(31);
    vector<int> Keys(TABLE_SIZE * 10);
    iota(Keys.begin(), Keys.end(), 0);

    for (int run = 0; run < 4; ++run){
        shuffle(Keys.begin(), Keys.end(), generator);
        TableNode *Original[TABLE_SIZE] = {};
        for (int i = 0; i < KEY_COUNT; ++i){
            if (!insert_open(Original, TABLE_SIZE, Keys[i], i)){
                cerr << "Ошибка заполнения таблицы\n";
                clear_open(Original, TABLE_SIZE);
                return 1;
            }
        }
        for (int i = 0; i < KEY_COUNT / 2; ++i){
            delete_open(Original, TABLE_SIZE, Keys[i]);
        }

        long long rebuildNs;
        TableNode **Rebuilt = rebuildTable(Original, TABLE_SIZE, rebuildNs);
        bool correct = Rebuilt && compareTables(Original, Rebuilt, Keys, generator, run, rebuildNs);
        clear_open(Original, TABLE_SIZE);
        if (Rebuilt){
            clear_open(Rebuilt, TABLE_SIZE);
            delete[] Rebuilt;
        }
        if (!correct){
            cerr << "Ошибка перестроения или проверки результатов поиска\n";
            return 1;
        }
    }
    return 0;
}

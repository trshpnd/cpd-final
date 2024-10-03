// hash-utils.hpp
// trshpnd 2024

#include <iostream>
#include <vector>
#include <string>

using namespace std;

template <typename T>
struct HashTable{
    vector<vector<T>> table;

    HashTable(size_t tableSize){
        table.resize(tableSize);
    }
};

// Retorna o indice para a inserção do item na tabela hash.
unsigned long long polynomialHash(int number, int mod, int base = 31){
    unsigned long long hashIndex = 0;
    unsigned long long basePower = 1;

    while (number > 0) {
        int digit = number % 10;
        hashIndex = (hashIndex + digit * basePower) % mod;
        basePower = (basePower * base) % mod;
        number /= 10;
    }

    return hashIndex;
}

// Insere o item na tabela.
template <typename T>
void hashInsert(HashTable<T> &hashTable, T oItem){
    int hashIndex = polynomialHash(oItem.id, hashTable.table.size());
    hashTable.table[hashIndex].push_back(oItem);
}

// Função de busca. Receba uma HashTable, uma chave (int) e um ponteiro para o item.
// O item é retornado por referência.
template <typename T>
void hashSearch(HashTable<T> &hashTable, int key, T* &oItem){
    bool found = false;

    int hashIndex = polynomialHash(key, hashTable.table.size());

    if(!hashTable.table[hashIndex].empty()){
        for(auto &item : hashTable.table[hashIndex]){
            if(item.id == key){
                oItem = &item;      // pointer to target
                found = true;
                break;
            }
        }
        if(!found) oItem = nullptr; // Retorna nullptr quando não encontrado.
    }
    else oItem = nullptr;          // Retorna null pointer quando vazio.
}

// Função de avaliação dos stats da hash table. Apenas para debug. 
template <typename T>
void hashStats(HashTable<T> hashTable, bool verbose = false){
    int empty = 0;
    int maxListSize = 0;
    int entries = 0;
    int avgListSize = 0;

    for(int i = 0; i < hashTable.table.size(); i++){

        if(verbose != false){
            cout << i << "\t";
        }

        if(!hashTable.table[i].empty()){

            if(hashTable.table[i].size() > maxListSize){
                maxListSize = hashTable.table[i].size();
            }

            for(const auto &item : hashTable.table[i]){
                entries++;
                if(verbose != false){
                    cout << item.id << "\t";
                }
            }
        }else empty++;

        if(verbose != false) cout << endl;
    }

    avgListSize = entries/(hashTable.table.size() - empty);
    
    cout << "Total Entries: \t" << entries << endl;
    cout << "Max List Size: \t" << maxListSize << endl;
    cout << "Avg List Size: \t" << avgListSize << endl;
    cout << "Empty Indexes: \t" << empty << endl;
    cout << "------------------------" << endl;
}
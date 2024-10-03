// trie-utils.hpp
// trshpnd 2024

#include <iostream>
#include <algorithm>
#include <vector>

class TrieNode {
public:
    std::vector<TrieNode*> children;
    bool isEndOfWord;
    std::vector<int> values; // Vetor para armazenar os valores associados ao nodo folha.

    TrieNode() : children(128, nullptr), isEndOfWord(false) {}
};

string toLowerCase(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

string toUpperCase(const std::string& str) {
    string upperStr = str;
    transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
}

class Trie {
private:
    TrieNode* root;

    // Coleciona valores, usado na função de pesquisa por prefixo. Resultado 
    // retornado por referência no vetor.
    void collectValues(TrieNode* node, vector<int>& result) {
        // Se for o nodo final, insere resultados no vetor.
        if (node->isEndOfWord) {
            result.insert(result.end(), node->values.begin(), node->values.end());
        }
        // Para cada nodo filho, coletar os valores. Chamada recursiva.
        for (TrieNode* child : node->children) {
            if (child != nullptr) {
                collectValues(child, result);
            }
        }
    }

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(const string& word, int value) {
        // Normaliza o input em letras minusculas.
        string lowerWord = toLowerCase(word);
        TrieNode* node = root;

        // Percorre a trie e cria nodos caso necessário.
        for (char ch : lowerWord) {
            if (node->children[ch] == nullptr) {
                // se não houver node, cria novo nodo
                node->children[ch] = new TrieNode();
            }
            // passa ao próximo nodo
            node = node->children[ch];
        }

        // Ao término da string, marca a flag como true.
        node->isEndOfWord = true;

        // Certifica-se de que o valor é único antes de adicioná-lo.
        if (find(node->values.begin(), node->values.end(), value) == node->values.end()) {
            node->values.push_back(value);
        }
    }

    // Busca de string exata na trie. Recebe a string a ser buscada e um vector que
    // será povoado com os dados satélites nas folhas do nó.
    bool search(const string& word, vector<int>& values) {
        // Normaliza o input em minúsculas.
        std::string lowerWord = toLowerCase(word);
        TrieNode* node = root;

        // Percorre a trie, caso encontre null pointer, a palavra não está presente.
        for (char ch : lowerWord) {
            if (node->children[ch] == nullptr) {
                return false;
            }
            node = node->children[ch]; // Passa ao próximo nó.
        }
        if (node->isEndOfWord) {
            values = node->values; // Retorna os valores caso encontre a palavra.
            return true;
        }
        return false;
    }

    // Busca de prefixo. Utiliza chamadas recursivas de 'collectValues' para retornar
    // todos os dados satélites de strings que contém o prefixo desejado.
    // Recebe o prefixo e um vetor que receberá os dados por referência.
    bool startsWith(const string& prefix, vector<int>& values) {
        string lowerPrefix = toLowerCase(prefix);
        TrieNode* node = root;
        for (char ch : lowerPrefix) {
            if (node->children[ch] == nullptr) {
                return false;
            }
            node = node->children[ch];
        }
        collectValues(node, values);
        return !values.empty();
    }
};

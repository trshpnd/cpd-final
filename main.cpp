// Roteiro do trabalho
// 1. Estruturas de dados de pesquisa:
//      1.1. Hash para armazenar dados de jogadores 
//      1.2. Estrutura para busca de strings de nomes 
//      1.3. Estrutura para guardar reviews de usuários 
//      1.4. Estrutura para guardar tags 
// 2. Pesquisas
//      2.1. Prefixos de nomes de jogadores - player <prefix>
//      2.2. Jogadores revisados por usuarios - user <userID>
//      2.3. Top jogadores de determinada posicao - top <N> <position>
//      2.4. Jogadores contendo x tags - tags <list of tags>
//
//      trshpnd 2024

#include "csv-parser/parser.hpp"
#include "hash-utils.hpp"
#include "trie-utils.hpp"

#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <iomanip>

#define PLAYERS_DIR     "arquivos-parte1//players.csv"  
#define RATING_DIR      "rating20M//rating.csv" //"arquivos-parte1//minirating.csv"
#define TAGS_DIR        "arquivos-parte1//tags.csv"

#define ID_FIELD_WIDTH      6
#define SHORT_FIELD_WIDTH   20
#define LONG_FIELD_WIDTH    40
#define POS_FIELD_WIDTH     15
#define NATION_FIELD_WIDTH  15
#define CLUB_FIELD_WIDTH    25
#define LEAGUE_FIELD_WIDTH  30
#define COUNT_FIELD_WIDTH   6
#define RATING_FIELD_WIDTH  9

struct Player{
    int id;
    string short_name;
    string long_name;
    string player_positions;
    string nationality;
    string club_name;
    string league_name;
    int total_ratings = 0;
    float rating = 0; 
};

struct Rating{
    int id;
    float rating;
};

struct User{
    int id;
    vector<Rating> user_ratings;
};

void buildHash(HashTable<Player> &playersHash, HashTable<User> &usersHash, string player_dir, string rating_dir){
    
    using namespace aria::csv;

    std::ifstream f(player_dir);
    std::ifstream g(rating_dir);

    Player oPlayer;

    cout << "Processando " << player_dir << "... ";
    CsvParser parser(f);

    for(auto& row : parser){
        auto field = parser.next_field();

        // Check for EOF
        switch(field.type){
            case FieldType::CSV_END:
                break; 
        }

        // Field 1: sofifa_id
        oPlayer.id = stoi(*field.data);

        // Field 2: short_name
        field = parser.next_field();
        oPlayer.short_name = *field.data;

        // Field 3: long_name
        field = parser.next_field();
        oPlayer.long_name = *field.data;

        // Field 4: player_positions
        field = parser.next_field();
        oPlayer.player_positions = *field.data;

        // Field 5: nationality
        field = parser.next_field();
        oPlayer.nationality = *field.data;

        // Field 6: club_name
        field = parser.next_field();
        oPlayer.club_name = *field.data;

        // Field 7: league_name
        field = parser.next_field();
        oPlayer.league_name = *field.data;

        hashInsert(playersHash, oPlayer);
    }

    cout << "Pronto. \nProcessando " << rating_dir << "... ";
    CsvParser parser_ratings(g);

    for(auto& row : parser_ratings){
        User oUser;
        Rating oRating;

        auto field = parser_ratings.next_field();

        // Check for EOF
        switch(field.type){
            case FieldType::CSV_END:
                break; 
        }

        // Field 1: user_id
        oUser.id = stoi(*field.data);

        // Field 2: sofifa_id
        field = parser_ratings.next_field();
        oRating.id = stoi(*field.data);

        // Field 3: rating
        field = parser_ratings.next_field();
        oRating.rating = stof(*field.data);

        User* userptr = nullptr;

        hashSearch(usersHash, oUser.id, userptr);

        if(userptr) userptr->user_ratings.push_back(oRating); // Se o usuario ja existe, apenas adiciona o novo rating.
        else{
            hashInsert(usersHash, oUser); // Se usuario n existe/indice vazio, cria novo usuario e insere rating.
            hashSearch(usersHash, oUser.id, userptr);
            userptr->user_ratings.push_back(oRating);
        }

        Player* playerptr = nullptr;

        // Pesquisa jogador no Player Hash e incrementa 'total ratings' e 'avg rating'.
        hashSearch(playersHash, oRating.id, playerptr);

        if(playerptr){
            playerptr->total_ratings++;
            playerptr->rating = oRating.rating + playerptr->rating;
        }
    }

    cout << "Pronto." << endl;
    cout << "Calculando media para cada jogador baseando-se nas avaliacoes de usuarios... ";
    

    // Re-itera player hash e calcula nota media de cada jogador.
    for(int i = 0; i < playersHash.table.size(); ++i){
        if(!playersHash.table[i].empty()){
            for(auto &player : playersHash.table[i]){
                if(player.rating != 0) player.rating = (float) player.rating/player.total_ratings;
            }
        }
    }

    cout << "Pronto." << endl;
}

void buildPlayerTrie(HashTable<Player> &playersHash, Trie &playerNames){
    for(int i =0; i< playersHash.table.size();i++){
        if(!playersHash.table[i].empty()){
            for(const auto &player : playersHash.table[i]){
                playerNames.insert(player.long_name, player.id);
            }
        }
    }
}

void buildTagsTrie(string tags_dir, Trie &playerTags){
    //parseia documento
    using namespace aria::csv;

    std::ifstream f(tags_dir);

    int oSofifa_id;
    string oTag;

    cout << "Processando "<< tags_dir <<"... ";
    CsvParser parser(f);

    for(auto& row : parser){
        auto field = parser.next_field();

        // Check for EOF
        switch(field.type){
            case FieldType::CSV_END:
                break; 
        }

        // Field 1: user_id [pula, não usado]

        // Field 2: sofifa_id
        field = parser.next_field();
        oSofifa_id = stoi(*field.data);

        // Field 3: tag
        field = parser.next_field();
        oTag = *field.data;

        // Insere tag na trie, juntamente com o id (na folha)
        playerTags.insert(oTag, oSofifa_id);
    }
    cout << "Pronto." << endl;
}

// Print genérico p/ debug
template <typename T>
void printVector(vector<T> &V){
    for(auto &item : V) cout << item << " ";
    cout << endl;
}

// Merge da função do mergesort, ordena baseado em rating.
template <typename T>
void merge(vector<T> &vec, int left, int mid, int right){
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<T> leftVec(n1);
    vector<T> rightVec(n2);

    for (int i = 0; i < n1; ++i)
        leftVec[i] = vec[left + i];
    for (int i = 0; i < n2; ++i)
        rightVec[i] = vec[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftVec[i].rating >= rightVec[j].rating) {
            vec[k] = leftVec[i];
            ++i;
        } else {
            vec[k] = rightVec[j];
            ++j;
        }
        ++k;
    }

    while (i < n1) {
        vec[k] = leftVec[i];
        ++i;
        ++k;
    }

    while (j < n2) {
        vec[k] = rightVec[j];
        ++j;
        ++k;
    }
}

template <typename T>
void mergeSort(vector<T> &vec, int left, int right){
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(vec, left, mid);
        mergeSort(vec, mid + 1, right);

        merge(vec, left, mid, right);
    }
}

// Particionamento do quicksort, pivô inicia na extremidade direita do segmento.
template <typename T>
int partition(std::vector<T> &vec, int low, int high) {
    T pivot = vec[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (vec[j] <= pivot) {
            ++i;
            std::swap(vec[i], vec[j]);
        }
    }
    std::swap(vec[i + 1], vec[high]);
    return i + 1;
}

// Quicksort genérico. Ordena vectors cujos dados não estão inseridos em um membro de classe/struct.
template <typename T>
void quicksort(std::vector<T> &vec, int low, int high) {
    if (low < high) {
        int pi = partition(vec, low, high);

        quicksort(vec, low, pi - 1);
        quicksort(vec, pi + 1, high);
    }
}

// Posiciona os elementos de um vetor usando os ids de outro vetor como source.
// Emparelha os vetores para ordenação estável.
template <typename T1, typename T2>
void updateVectorById(vector<T1> &sourceVec, vector<T2> &toBeUpdated){
    for(int i = 0; i < sourceVec.size(); i++){
        for(int j = 0; j < toBeUpdated.size(); j++){
            if(toBeUpdated[j].id == sourceVec[i].id){
                std::swap(toBeUpdated[j], toBeUpdated[i]);
                break;
            }
        }
    }
}

bool containsSubstring(const string &str, const string &substr){
    return str.find(substr) != string::npos;
}

vector<string> parseTags(istringstream &iss) {
    vector<string> tags;
    string line;
    getline(iss, line);

    bool inside = false;
    string currentTag;

    for (char ch : line) {
        if (ch == '\'') {
            if (inside) {
                // Fim da tag, push.
                tags.push_back(currentTag);
                currentTag.clear();
            }
            // Alterna se dentro/fora das apóstrofes.
            inside = !inside;
        } else if (inside) {
            // Adiciona o caracter à tag atual.
            currentTag += ch;
        }
    }

    return tags;
}

// Realiza a interseção de 2 vectors<int>. Retorna um vector<int>.
vector<int> intersectTwoVectors(const vector<int>& v1, const vector<int>& v2) {
    vector<int> intersection;
    //início e fim do v1, início e fim do v2, iterador no final do vetor 'intersection'.
    set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(intersection));
    return intersection;
}

// Realiza a interseção de vários vectors<int>. Utiliza um vector<vector<int>> povoado
// com diversos vector<int>.
vector<int> intersectMultipleVectors(vector<vector<int>> vectors) {
    if (vectors.empty()) return {};

    // set_intersection funciona apenas em vectors previamente ordenados.
    for (auto& vec : vectors) {
        quicksort(vec, 0, (vec.size()-1));
    }

    // result inicia sendo povoado com o vector na posição 0 do vetor de vetores.
    vector<int> result = vectors[0];

    // Iterativamente chama a função para a interseção de dois vectors.
    for (size_t i = 1; i < vectors.size(); ++i) {
        result = intersectTwoVectors(result, vectors[i]);
    }

    return result;
}

int main(){
    int M = 37879;
    int N = 276989; 

    // Hash tables
    HashTable<Player>   players(M);
    HashTable<User>     users(N);

    // Tries
    Trie playerNames;
    Trie playerTags;

    // Pointers
    Player* playerPtr = nullptr;
    User* userPtr = nullptr;

    // Instances
    Player  oPlayer;
    User    oUser;

    // Input
    string input, query_type, query_args;
    bool quit = false;

    // Vectors
    vector<int> player_id_list;
    vector<Player> player_list;
    vector<Rating> rating_list;
    vector<string> tag_list;

    auto start = chrono::high_resolution_clock::now();

    buildHash(players, users, PLAYERS_DIR, RATING_DIR);
    buildPlayerTrie(players, playerNames);
    buildTagsTrie(TAGS_DIR, playerTags);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Processo finalizado em "<< duration.count() << " segundos.\n" << endl;

    /*int total_users = 0;
    for(int i = 0; i < users.table.size(); i++){
        if(!users.table[i].empty()){
            for(int j = 0; j < users.table[i].size(); j++){
                total_users++;
            }
        }
    }*/

    //cout << "NUM OF USERS: " << total_users << endl; //~138k

    // Menu
    while(!quit){
        cout << "Digite a pesquisa desejada: ";
        getline(cin, input);
        istringstream iss(input);
        iss >> query_type;
        query_type = toLowerCase(query_type);

        // Pesq 1: Player <sofifa_id>
        if(query_type == "player"){
            iss >> query_args;
            query_args = toLowerCase(query_args);

            playerNames.startsWith(query_args, player_id_list);
            for(auto j : player_id_list){
                hashSearch(players, j, playerPtr);
                oPlayer = *playerPtr;
                player_list.push_back(oPlayer);
            }

            // Sorts by global rating
            mergeSort(player_list, 0, (player_list.size()-1));
            
            cout << endl;
            for(auto k : player_list){    
                cout    << setw(ID_FIELD_WIDTH)     << k.id << " " 
                        << setw(SHORT_FIELD_WIDTH)  << k.short_name << " " 
                        << setw(LONG_FIELD_WIDTH)   << k.long_name << " " 
                        << setw(POS_FIELD_WIDTH)    << k.player_positions << " "
                        << setw(RATING_FIELD_WIDTH) << fixed << setprecision(6) << k.rating
                        << setw(COUNT_FIELD_WIDTH)  << k.total_ratings 
                        << endl;
            }
        }

        // Pesq 2: User <user_id>
        else if(query_type == "user"){
            iss >> query_args;
            query_args = toLowerCase(query_args);
            int key = stoi(query_args);

            hashSearch(users, key, userPtr);
            oUser = *userPtr;
            rating_list = oUser.user_ratings;

            for(const auto r : rating_list){
                hashSearch(players, r.id, playerPtr);
                oPlayer = *playerPtr;
                player_list.push_back(oPlayer);
            }

            // Ordenação secundária: global rating
            mergeSort(player_list, 0, (player_list.size()-1));

            // Atualiza rating_list de acordo com os ids de player_list.
            updateVectorById(player_list, rating_list);

            // Ordenação primária: user rating
            mergeSort(rating_list, 0, (rating_list.size()-1));

            // Atualiza player_list de acordo com os ids de rating_list.
            updateVectorById(rating_list, player_list);

            cout << endl;
            for(int i = 0; i < 20; i++){
                cout    << setw(ID_FIELD_WIDTH)     << rating_list[i].id << " " 
                        << setw(SHORT_FIELD_WIDTH)  << player_list[i].short_name << " "
                        << setw(LONG_FIELD_WIDTH)   << player_list[i].long_name << " "
                        << setw(RATING_FIELD_WIDTH) << fixed << setprecision(6) << player_list[i].rating << " "
                        << setw(COUNT_FIELD_WIDTH)  << player_list[i].total_ratings << " "
                        << setw(RATING_FIELD_WIDTH) << fixed << setprecision(1) << rating_list[i].rating << " " 
                        << endl;
            }
        }

        // Pesq 3: Top <N> <position>
        else if(query_type == "top"){
            int N;
            string position;

            // Atribui o segundo e terceiro membros da stream à var N e posição, respectivamente.
            iss >> N >> position;

            // Como as posições estão armazenadas em letras maiusculas, 
            // normaliza o input do usuário para letras maiusculas.
            position = toUpperCase(position);

            for(int i = 0; i < players.table.size(); i++){
                for(const auto player : players.table[i]){
                    // Corre pelo hash de jogadores, adicionando ao vector apenas aqueles que
                    // contêm a substring da posição desejada e que possuem 1000 ou mais avaliações.
                    if(player.total_ratings >= 1000 && (containsSubstring(player.player_positions, position))){
                        oPlayer = player;
                        player_list.push_back(oPlayer);
                    }
                }
            }

            mergeSort(player_list, 0, (player_list.size()-1));
            
            cout << endl;
            for(int i = 0; i < N; i++){
                cout    << setw(ID_FIELD_WIDTH) << player_list[i].id << " "
                        << setw(SHORT_FIELD_WIDTH) << player_list[i].short_name << " "
                        << setw(LONG_FIELD_WIDTH) << player_list[i].long_name << " "
                        << setw(POS_FIELD_WIDTH) << player_list[i].player_positions << " "
                        << setw(NATION_FIELD_WIDTH) << player_list[i].nationality << " "
                        << setw(CLUB_FIELD_WIDTH) << player_list[i].club_name << " "
                        << setw(LEAGUE_FIELD_WIDTH) << player_list[i].league_name << " "
                        << setw(RATING_FIELD_WIDTH) << fixed << setprecision(6) << player_list[i].rating << " "
                        << setw(COUNT_FIELD_WIDTH) << player_list[i].total_ratings << " "
                        << endl;
            }
        }

        // Pesq 4: tags <list of tags>
        else if(query_type ==  "tags"){
            // Realiza o parsing do restante da linha escrita pelo usuário.
            // tags são retornadas no vector tag_list.
            tag_list = parseTags(iss);
            vector<vector<int>> tag_results;

            for(int i = 0; i < tag_list.size(); i++){
                // Busca tag na trie, adiciona os ids (int) relacionados à tag 
                // ao vector player_id_list e posteriormente adiciona ao vector tag_results.
                playerTags.search(tag_list[i], player_id_list);
                tag_results.push_back(player_id_list);
                player_id_list.clear();
            }
            
            // Cria o vetor intersection e realiza a interseção de todos os vectors em tag_results.
            vector<int> intersection = intersectMultipleVectors(tag_results);

            // Para cada id na interseção, busca o player correspondente no Hash 'players'.
            for(const auto i : intersection){
                hashSearch(players, i, playerPtr);
                oPlayer = *playerPtr;
                player_list.push_back(oPlayer);
            }

            // Ordena a player_list com base na nota global.
            mergeSort(player_list, 0, (player_list.size()-1));

            cout << endl;
            for(int i = 0; i < player_list.size(); i++){
                cout    << setw(ID_FIELD_WIDTH) << player_list[i].id << " "
                        << setw(SHORT_FIELD_WIDTH) << player_list[i].short_name << " "
                        << setw(LONG_FIELD_WIDTH) << player_list[i].long_name << " "
                        << setw(POS_FIELD_WIDTH) << player_list[i].player_positions << " "
                        << setw(NATION_FIELD_WIDTH) << player_list[i].nationality << " "
                        << setw(CLUB_FIELD_WIDTH) << player_list[i].club_name << " "
                        << setw(LEAGUE_FIELD_WIDTH) << player_list[i].league_name << " "
                        << setw(RATING_FIELD_WIDTH) << fixed << setprecision(6) << player_list[i].rating << " "
                        << setw(COUNT_FIELD_WIDTH) << player_list[i].total_ratings << " "
                        << endl;
            }
        }

        // Sair
        else if(query_type == "sair") quit = true;

        // Default: comando desconhecido
        else{
            cout << "Comando desconhecido.";
        }
        cout << endl;

        // Clear buffers
        player_id_list.clear();
        player_list.clear();
        rating_list.clear();
        tag_list.clear();
    }

    return 0;
}
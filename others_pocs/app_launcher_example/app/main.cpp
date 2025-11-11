#include <iostream>
#include <dlfcn.h>      // Para carregamento dinâmico (.so)
#include <openssl/ssl.h> // Header do OpenSSL
#include <openssl/err.h> // Header do OpenSSL

#define USE_OPENSSL_1_1_1 
// Se você quiser usar a 3.0, comente a linha acima e descomente o bloco de inicialização 3.0

using namespace std;

// Definição do tipo de ponteiro da função que será carregada dinamicamente
typedef int (*ConnectFunc)(const char*, const char*);

// Path da sua instalação OpenSSL (Use o path exato que você forneceu)
//const char* OPENSSL_ROOT_PATH = "/mnt/data/totvs/projects/advtec9/branches/appserver-24.3.0.x/third_libraries/openssl/3.0.13/linux64/gcc-8.3.1/";

// Credenciais de teste
const char* USERNAME = "totvsvmtests"; // Substitua
const char* PASSWORD = "totvs@123456";   // Substitua
const char* LIB_PATH = "./libconnector.so";

/**
 * @brief Inicializa a OpenSSL 3 estaticamente.
 * @return true se bem-sucedido.
 */
bool initialize_openssl() {
    cout << "--- Inicializando OpenSSL (Estático) ---" << endl;

#ifdef USE_OPENSSL_1_1_1
    // ROTINA DE INICIALIZAÇÃO CLÝSSICA (1.1.1 e anteriores)
    
    // Carrega strings de erro e todos os algoritmos de criptografia e digest
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    cout << "OpenSSL 1.1.1t inicializada com sucesso." << endl;
#else    
    // OpenSSL 3.0 simplificou a inicialização
    if (SSL_library_init() != 1) { // Já obsoleto, mas funciona para OpenSSL 1.x
        cerr << "Erro: SSL_library_init falhou." << endl;
        return false;
    }
    // Load algorithms and error strings
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    cout << "OpenSSL 3.0 inicializada com sucesso." << endl;
#endif    
    cout << "----------------------------------------" << endl;
    return true;
}

/**
 * @brief Carrega a biblioteca ODBC dinamicamente e executa a função.
 */
int main() {
    if (!initialize_openssl()) {
        return 1;
    }

    // 1. Carregamento Dinâmico da Biblioteca .so
    cout << "--- Carregando biblioteca dinâmica (" << LIB_PATH << ") ---" << endl;
    //void* handle = dlopen(LIB_PATH, RTLD_LAZY);
    void* handle = dlopen(LIB_PATH, (RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND));
    if (!handle) {
        cerr << "ERRO FATAL: Nao foi possivel carregar a biblioteca .so: " << dlerror() << endl;
        return 1;
    }

    // 2. Mapeamento da Função
    ConnectFunc connect_fn = (ConnectFunc)dlsym(handle, "connect_and_run_sql");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "ERRO FATAL: Nao foi possivel encontrar a funcao connect_and_run_sql: " << dlsym_error << endl;
        dlclose(handle);
        return 1;
    }
    cout << "Funcao 'connect_and_run_sql' mapeada com sucesso." << endl;

    // 3. Execução do Módulo ODBC/SQL Server
    cout << "\n--- Iniciando o Modulo ODBC/SQL Server ---" << endl;
    int result = connect_fn(USERNAME, PASSWORD);

    // 4. Limpeza
    cout << "\n--- Limpando recursos ---" << endl;
    dlclose(handle);

    if (result == 0) {
        cout << "SUCESSO: Rotina completa executada sem falhas." << endl;
    } else {
        cerr << "FALHA: A rotina SQL retornou codigo de erro: " << result << endl;
    }

    return result;
}
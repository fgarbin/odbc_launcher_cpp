#ifndef DBINTERFACE
#define DBINTERFACE

// dbinterface.hpp

#include <string>
#include <sql.h>
#include <sqlext.h>


#define DBINTERFACE_MSG(x)   printf("[INFO ] %s\n", x)
#define DBINTERFACE_ERR(x)   printf("[ERROR] %s\n", x)

// Bloco extern "C" é crucial para garantir a compatibilidade com dlsym
#ifdef __cplusplus
extern "C" {
#endif

// Define o tipo de ponteiro de função para db_connect
//typedef int (*ConnectFunc)(const char *dbName);

// Define o tipo de ponteiro de função para db_disconnect
//typedef void (*DisconnectFunc)();

/**
 * @brief Carrega a libunixodbc por DLOPEN.
 * @return 0 em caso de sucesso, -1 em caso de falha.
 */
bool load_odbc_symbols();

/**
 * @brief Simula a conexão com um banco de dados.
 * @param dbName Nome do banco de dados.
 * @return 0 em caso de sucesso, -1 em caso de falha.
 */
int db_connect(const char *DSN_NAME, const char *username, const char *password );

/**
 * @brief Simula a desconexão do banco de dados.
 */
int db_disconnect();

#ifdef __cplusplus
}
#endif

#endif
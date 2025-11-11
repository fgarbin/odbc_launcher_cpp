#include "odbc_connector.h"
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <cstring>
#include <iomanip> 

using namespace std;

// Definições de Handle para ODBC (Inicializados como NULL)
SQLHENV env = SQL_NULL_HENV; 
SQLHDBC dbc = SQL_NULL_HDBC; 
SQLHSTMT stmt = SQL_NULL_HSTMT; 

// Nomes de Conexão e Tabela
const char* DSN_NAME = "utlogix_sql"; 
const char* TABLE_NAME = "TabelaTeste"; 

// ... Funções auxiliares (HandleDiagnosticRecord, executeSQL, commitTransaction) ...
// (Omitidas por brevidade, use as versões finais e corrigidas do nosso chat anterior)
// Coloque aqui as três funções HandleDiagnosticRecord, executeSQL e commitTransaction.

// *** INÍCIO DA FUNÇÃO EXPORTADA ***
int connect_and_run_sql(const char* username, const char* password) {
    SQLRETURN ret;
    string sql;
    
    // VARIÁVEIS MOVIDAS PARA O ESCOPO
    SQLINTEGER id;
    SQLCHAR nome[51];
    SQLCHAR valorStr[51];
    SQLLEN idLen, nomeLen, valorStrLen;
    int rowCount = 0; 

    // 1. Inicializa o ambiente ODBC
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // 2. Aloca o handle da conexão
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        // HandleDiagnosticRecord(env, SQL_HANDLE_ENV, "SQLAllocHandle(DBC)");
        // Lógica de limpeza e retorno
        return 1;
    }

    // ***** CONECTA USANDO SQLConnect E DSN *****
    cout << "Tentando conectar ao DSN '" << DSN_NAME << "' com SQLConnect..." << endl;
    ret = SQLConnect(dbc, (SQLCHAR*)DSN_NAME, SQL_NTS, 
                     (SQLCHAR*)username, SQL_NTS, 
                     (SQLCHAR*)password, SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        // HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLConnect");
        goto cleanup;
    }
    cout << "Conexão estabelecida com sucesso usando DSN!" << endl;
    
    // ... Restante da lógica (SQLSetConnectAttr, SQLAllocHandle(STMT), CREATE, INSERT, SELECT, DROP) ...
    // Insira aqui o conteúdo dos passos 4, 5, 6 do main() anterior, adaptando para usar 'username' e 'password'
    // ...

    // --- Seções de SELECT ---
    // ... Código de SELECT e loop de leitura ...
    
    // 6. Limpeza de Handles e Desconexão
    cleanup:
    cout << "\nEncerrando a conexão e liberando recursos..." << endl;
    if (stmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (dbc != SQL_NULL_HDBC) {
        SQLDisconnect(dbc); 
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, env);
    
    return 0; // Sucesso
}
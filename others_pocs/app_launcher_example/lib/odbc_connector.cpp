#include "odbc_connector.h"
#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <cstring>
#include <iomanip> // Para manipulação de saída (cout)

using namespace std;

// Definições de Handle para ODBC (Inicializados como NULL)
SQLHENV env = SQL_NULL_HENV; 
SQLHDBC dbc = SQL_NULL_HDBC; 
SQLHSTMT stmt = SQL_NULL_HSTMT; 

// Nomes de Conexão e Tabela
const char* DSN_NAME = "utlogix_sql"; 
const char* TABLE_NAME = "TabelaTeste"; 

// =======================================================
// FUNÇÕES AUXILIARES
// =======================================================

void HandleDiagnosticRecord (SQLHANDLE handle, SQLSMALLINT type, const char* functionName) {
    SQLSMALLINT i = 1;
    SQLINTEGER nativeError;
    SQLCHAR state[6], text[256];
    SQLSMALLINT length;
    SQLRETURN ret;

    cerr << "--- Erro em " << functionName << " ---" << endl;
    while ((ret = SQLGetDiagRec(type, handle, i, state, &nativeError, text, sizeof(text), &length)) == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        cerr << "SQL State: " << state << endl;
        cerr << "Erro Nativo: " << nativeError << endl;
        cerr << "Mensagem: " << text << endl;
        i++;
    }
}

bool executeSQL(const char* sql, bool logError = true) {
    SQLRETURN ret = SQLExecDirect(stmt, (SQLCHAR*)sql, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        if (logError) {
            HandleDiagnosticRecord(stmt, SQL_HANDLE_STMT, "SQLExecDirect");
        }
        return false;
    }
    return true;
}

bool commitTransaction(SQLHDBC hdbc) {
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, "SQLEndTran(COMMIT)");
        return false;
    }
    return true;
}

// =======================================================
// FUNÇÃO EXPORTADA PELA .SO
// =======================================================

int connect_and_run_sql(const char* username, const char* password) {
    SQLRETURN ret;
    string sql;
    int errorCode = 0; // Código de erro a retornar

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
        errorCode = 1;
        goto cleanup;
    }

    // 3. CONECTA USANDO SQLConnect E DSN
    cout << "Tentando conectar ao DSN '" << DSN_NAME << "' com SQLConnect..." << endl;
    ret = SQLConnect(dbc, (SQLCHAR*)DSN_NAME, SQL_NTS, 
                     (SQLCHAR*)username, SQL_NTS, 
                     (SQLCHAR*)password, SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLConnect");
        errorCode = 2;
        goto cleanup;
    }
    cout << "Conexão estabelecida com sucesso usando DSN!" << endl;
    
    // 4. FORÇA O MODO MANUAL-COMMIT
    ret = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_POINTER);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLSetConnectAttr(AUTOCOMMIT_OFF)");
        errorCode = 3;
        goto cleanup;
    }

    // 5. Aloca o handle da instrução
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLAllocHandle(STMT)");
        errorCode = 4;
        goto cleanup;
    }
    
    // ***** PASSO 6: CREATE TABLE *****
    sql = "DROP TABLE " + string(TABLE_NAME);
    executeSQL(sql.c_str(), false); 

    cout << "\nCriando a tabela " << TABLE_NAME << "..." << endl;
    sql = "CREATE TABLE " + string(TABLE_NAME) + " (ID INT PRIMARY KEY, Nome VARCHAR(50), Valor DECIMAL(10, 2))";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao criar a tabela. Encerrando." << endl;
        errorCode = 5;
        goto cleanup;
    }
    cout << "Tabela criada com sucesso." << endl;
    if (!commitTransaction(dbc)) { errorCode = 6; goto cleanup; }

    // ***** PASSO 7: INSERT *****
    cout << "\nInserindo um registro..." << endl;
    sql = "INSERT INTO " + string(TABLE_NAME) + " (ID, Nome, Valor) VALUES (1, 'Exemplo SQLConnect', 99.99)";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao inserir dados. Encerrando." << endl;
        errorCode = 7;
        goto cleanup;
    }
    cout << "Inserção concluída." << endl;
    if (!commitTransaction(dbc)) { errorCode = 8; goto cleanup; }

    // ***** PASSO 8: SELECT *****
    cout << "\nExecutando SELECT..." << endl;
    sql = "SELECT ID, Nome, Valor FROM " + string(TABLE_NAME) + " ORDER BY ID";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao executar SELECT. Encerrando." << endl;
        errorCode = 9;
        goto cleanup;
    }

    // Associa as colunas aos buffers
    SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &idLen);
    SQLBindCol(stmt, 2, SQL_C_CHAR, nome, sizeof(nome), &nomeLen);
    SQLBindCol(stmt, 3, SQL_C_CHAR, valorStr, sizeof(valorStr), &valorStrLen); 

    cout << "--------------------------------------" << endl;
    cout << "ID | Nome | Valor" << endl;
    cout << "--------------------------------------" << endl;

    // Percorre o Result Set
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        cout << id << " | " << (char*)nome << " | " << (char*)valorStr << endl;
        rowCount++;
    }
    
    if (rowCount == 0) {
        cout << "Nenhum registro encontrado." << endl;
    }
    cout << "--------------------------------------" << endl;
    SQLFreeStmt(stmt, SQL_CLOSE); 

    // ***** PASSO 9: DROP TABLE *****
    cout << "\nExcluindo a tabela " << TABLE_NAME << " (limpeza)..." << endl;
    sql = "DROP TABLE " + string(TABLE_NAME);
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao excluir a tabela." << endl;
        // Não definimos erro fatal aqui, pois a limpeza não impede o sucesso do teste principal
    } else {
        cout << "Tabela excluída com sucesso." << endl;
    }
    if (!commitTransaction(dbc)) { /* Ignora o erro, apenas limpando */ }

// Limpeza de Handles e Desconexão
cleanup:
    cout << "\nEncerrando a conexão e liberando recursos..." << endl;
    if (stmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (dbc != SQL_NULL_HDBC) {
        SQLDisconnect(dbc); 
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, env);

    return errorCode; // Retorna 0 em caso de sucesso
}
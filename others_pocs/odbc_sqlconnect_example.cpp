/*
   Este exemplo usa a função SqlConnect

   $ g++ odbc_sqlconnect_example.cpp -o odbc_connect_example -lodbc
*/

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

// Nome do DSN (Solicitado)
const char* DSN_NAME = "xxxxxx"; 
// Nome da tabela e esquema
const char* TABLE_NAME = "tabelaTeste"; 

// --- Variáveis de Credenciais (A serem substituídas) ---
const char* DB_USER = "xxxxx";
const char* DB_PWD = "xxxxx";
// -------------------------------------------------------

// Função auxiliar para verificar e exibir erros ODBC
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

// Função principal para execução das operações SQL (sem SELECT)
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

// Função para confirmar (commit) a transação
bool commitTransaction(SQLHDBC hdbc) {
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, "SQLEndTran(COMMIT)");
        return false;
    }
    return true;
}

// ----------------------------------------------------
// Função principal para configurar a conexão e executar as tarefas
// ----------------------------------------------------
int main() {
    SQLRETURN ret;
    string sql;

    // --- VARIÝVEIS MOVIDAS PARA O TOPO ---
    SQLINTEGER id;
    SQLCHAR nome[51];
    SQLCHAR valorStr[51];
    SQLLEN idLen, nomeLen, valorStrLen;
    int rowCount = 0; 
    // ------------------------------------

    // 1. Inicializa o ambiente ODBC
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // 2. Aloca o handle da conexão
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(env, SQL_HANDLE_ENV, "SQLAllocHandle(DBC)");
        goto cleanup;
    }

    // ***** ALTERAÇÃO AQUI: USANDO SQLConnect *****
    cout << "Tentando conectar ao DSN '" << DSN_NAME << "'..." << endl;
    ret = SQLConnect(dbc, (SQLCHAR*)DSN_NAME, SQL_NTS, 
                     (SQLCHAR*)DB_USER, SQL_NTS, 
                     (SQLCHAR*)DB_PWD, SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLConnect");
        goto cleanup;
    }
    cout << "Conexão estabelecida com sucesso usando DSN!" << endl;
    
    // 4. FORÇA O MODO MANUAL-COMMIT
    ret = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_POINTER);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLSetConnectAttr(AUTOCOMMIT_OFF)");
        goto cleanup;
    }

    // 5. Aloca o handle da instrução
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLAllocHandle(STMT)");
        goto cleanup;
    }
    
    // ***** PASSO 2: CREATE TABLE *****
    sql = "DROP TABLE " + string(TABLE_NAME);
    executeSQL(sql.c_str(), false); 

    cout << "\nCriando a tabela " << TABLE_NAME << "..." << endl;
    sql = "CREATE TABLE " + string(TABLE_NAME) + " (ID INT PRIMARY KEY, Nome VARCHAR(50), Valor DECIMAL(10, 2))";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao criar a tabela. Encerrando." << endl;
        goto cleanup;
    }
    cout << "Tabela criada com sucesso." << endl;
    if (!commitTransaction(dbc)) goto cleanup;

    // ***** PASSO 3: INSERT *****
    cout << "\nInserindo um registro..." << endl;
    sql = "INSERT INTO " + string(TABLE_NAME) + " (ID, Nome, Valor) VALUES (1, 'Exemplo SQLConnect', 99.99)";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao inserir dados. Encerrando." << endl;
        goto cleanup;
    }
    cout << "Inserção concluída." << endl;
    if (!commitTransaction(dbc)) goto cleanup;

    // ***** PASSO 4: SELECT *****
    cout << "\nExecutando SELECT..." << endl;
    sql = "SELECT ID, Nome, Valor FROM " + string(TABLE_NAME) + " ORDER BY ID";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao executar SELECT. Encerrando." << endl;
        goto cleanup;
    }

    // Associa as colunas aos buffers (SQLBindCol)
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

    // ***** PASSO 5: DROP TABLE *****
    cout << "\nExcluindo a tabela " << TABLE_NAME << " (limpeza)..." << endl;
    sql = "DROP TABLE " + string(TABLE_NAME);
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao excluir a tabela." << endl;
    } else {
        cout << "Tabela excluída com sucesso." << endl;
    }
    if (!commitTransaction(dbc)) goto cleanup;

// 6. Limpeza de Handles e Desconexão
cleanup:
    cout << "\nEncerrando a conexão e liberando recursos..." << endl;
    if (stmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (dbc != SQL_NULL_HDBC) {
        SQLDisconnect(dbc); 
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, env);

    cout << "Programa concluído." << endl;
    return 0;
}
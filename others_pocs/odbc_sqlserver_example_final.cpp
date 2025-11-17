/*
     $ g++ odbc_sqlserver_example_final.cpp -o odbc_example -lodbc
*/

#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <cstring>
#include <iomanip> // Necessário para formatação (cout << fixed << setprecision)

using namespace std;

// Definições de Handle para ODBC (Inicializados como NULL)
SQLHENV env = SQL_NULL_HENV; 
SQLHDBC dbc = SQL_NULL_HDBC; 
SQLHSTMT stmt = SQL_NULL_HSTMT; 

// Nome da tabela
const char* TABLE_NAME = "TabelaTeste";

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
    string connString;
    string sql;

    // --- VARIÝVEIS MOVIDAS PARA O TOPO (CORREÇÃO DO ERRO GOTO) ---
    SQLCHAR outString[1024];
    SQLSMALLINT outStringLen;
    
    // Variáveis usadas no SELECT:
    SQLINTEGER id;
    SQLCHAR nome[51];
    SQLCHAR valorStr[51];
    SQLLEN idLen, nomeLen, valorStrLen;
    int rowCount = 0; 
    // --------------------------------------------------------

    // ***** PASSO 1: CONFIGURAÇÃO DA CONEXÃO *****
    // CERTIFIQUE-SE DE SUBSTITUIR OS PLACEHOLDERS ABAIXO
/*
    string connString = 
        "Driver={ODBC Driver 18 for SQL Server};"
        "Server=192.198.23.141;" 
        "Database=dbtest;"
        "Uid=username;"
        "Pwd=password;"
        "TrustServerCertificate=yes;";
*/
    // Se voc� tivesse um DSN chamado "MyDSN"
    // Esta string seria suficiente se o MyDSN j� tivesse todas as informa��es
    // (Driver, Server, Database, TrustServerCertificate, Uid, Pwd)
   connString = "DSN=xxxx;Uid=xxxx;Pwd=xxxxx;"; 

    // 1. Inicializa o ambiente ODBC
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // 2. Aloca o handle da conexão
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(env, SQL_HANDLE_ENV, "SQLAllocHandle(DBC)");
        goto cleanup;
    }

    // 3. Conecta ao banco de dados usando a string de conexão
    cout << "Tentando conectar ao SQL Server..." << endl;
    ret = SQLDriverConnect(dbc, NULL, (SQLCHAR*)connString.c_str(), SQL_NTS, 
                           outString, sizeof(outString), &outStringLen, SQL_DRIVER_NOPROMPT); // SOLUÇÃO PARA O ERRO IM008

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLDriverConnect");
        goto cleanup;
    }
    cout << "Conexão estabelecida com sucesso!" << endl;
    
    // 4. FORÇA O MODO MANUAL-COMMIT (Melhor controle de transações)
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
    // --- Criação da Tabela (Limpeza) ---
    sql = "DROP TABLE " + string(TABLE_NAME);
    executeSQL(sql.c_str(), false); // Ignora erro se não existir

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
    sql = "INSERT INTO " + string(TABLE_NAME) + " (ID, Nome, Valor) VALUES (1, 'Exemplo UnixODBC', 123.45)";
    if (!executeSQL(sql.c_str())) {
        cerr << "Falha ao inserir dados. Encerrando." << endl;
        goto cleanup;
    }
    cout << "Inserção concluída." << endl;
    // COMMIT para garantir que o SELECT veja os dados
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
    // BIND DO DECIMAL(10, 2) COMO STRING (SQL_C_CHAR) PARA EVITAR ERROS DE CONVERSÃO C++
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
    SQLFreeStmt(stmt, SQL_CLOSE); // Fecha o cursor do SELECT

    // ***** PASSO 5: DROP TABLE *****
    // --- Limpeza (Exclui a Tabela) ---
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
        // SQLDisconnect() é opcional, pois SQLFreeHandle(DBC) faz isso
        SQLDisconnect(dbc); 
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, env);

    cout << "Programa concluído." << endl;
    return 0;
}
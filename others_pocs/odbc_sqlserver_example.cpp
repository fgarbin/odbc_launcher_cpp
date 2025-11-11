/*

     $ g++ odbc_sqlserver_example.cpp -o odbc_example -lodbc
     $ ./odbc_example


*/

#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <cstring>
#include <iomanip> // Adicione este cabe�alho

using namespace std;

// Defini��es de Handle para ODBC
SQLHENV env; // Handle do Ambiente
SQLHDBC dbc; // Handle da Conex�o
SQLHSTMT stmt; // Handle da Instru��o

// Fun��o auxiliar para verificar e exibir erros ODBC
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

// ----------------------------------------------------
// Fun��o principal para execu��o das opera��es SQL
// ----------------------------------------------------
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

// ----------------------------------------------------
// Fun��o para configurar a conex�o e executar as tarefas
// ----------------------------------------------------
int main() {
    SQLRETURN ret;
/*
    string connString = 
        "Driver={ODBC Driver 18 for SQL Server};"
        "Server=192.168.15.142;" // Ex: 192.168.1.10,1433
        "Database=dbtotvsvmtests;"
        "Uid=totvsvmtests;"
        "Pwd=totvs@123456;"
        "TrustServerCertificate=yes;";
*/
    // Se voc� tivesse um DSN chamado "MyDSN"
    // Esta string seria suficiente se o MyDSN j� tivesse todas as informa��es
    // (Driver, Server, Database, TrustServerCertificate, Uid, Pwd)
    string connString = "DSN=utlogix_sql;Uid=totvsvmtests;Pwd=totvs@123456;"; 

    // O restante do SQLDriverConnect permaneceria o mesmo, mas usando SQL_DRIVER_NOPROMPT

    // 1. Inicializa o ambiente ODBC
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        cerr << "Erro ao alocar o handle do ambiente (SQLAllocHandle)." << endl;
        return 1;
    }

    // Define a vers�o da API ODBC (importante para drivers modernos)
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // 2. Aloca o handle da conex�o
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(env, SQL_HANDLE_ENV, "SQLAllocHandle(DBC)");
        SQLFreeHandle(SQL_HANDLE_ENV, env);
        return 1;
    }

    // 3. Conecta ao banco de dados usando a string de conex�o
    SQLCHAR outString[1024];
    SQLSMALLINT outStringLen;
    cout << "Tentando conectar ao SQL Server..." << endl;
//    ret = SQLDriverConnect(dbc, NULL, (SQLCHAR*)connString.c_str(), SQL_NTS, 
//                           outString, sizeof(outString), &outStringLen, SQL_DRIVER_COMPLETE);

      ret = SQLDriverConnect(dbc, NULL, (SQLCHAR*)connString.c_str(), SQL_NTS, 
                            outString, sizeof(outString), &outStringLen, SQL_DRIVER_NOPROMPT);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLDriverConnect");
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
        SQLFreeHandle(SQL_HANDLE_ENV, env);
        return 1;
    }
    cout << "Conex�o estabelecida com sucesso!" << endl;

    // TENTANDO RESOLVER O PROBLEMA DE INSERÇÃO DE DADOS AINDA...
    // *** FORÇA O MODO MANUAL-COMMIT ***
    ret = SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, SQL_IS_POINTER);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLSetConnectAttr(AUTOCOMMIT_OFF)");
        goto cleanup;
    }    

    // 4. Aloca o handle da instru��o
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLAllocHandle(STMT)");
        // Limpeza de handles e sa�da
        goto cleanup;
    }
    
    // --- Cria��o da Tabela (Se j� existir, o erro ser� ignorado) ---
    // Tentativa de dropar a tabela primeiro (para garantir que a tabela esteja limpa)
    executeSQL("DROP TABLE TabelaTeste", false); // N�o loga erro se a tabela n�o existir

    cout << "\nCriando a tabela TabelaTeste..." << endl;
    if (!executeSQL("CREATE TABLE TabelaTeste (ID INT PRIMARY KEY, Nome VARCHAR(50), Valor DECIMAL(10, 2))")) {
        cerr << "Falha ao criar a tabela. Encerrando." << endl;
        goto cleanup;
    }
    cout << "Tabela criada com sucesso." << endl;

    // --- Inser��o de Dados ---
    cout << "\nInserindo um registro..." << endl;
    if (!executeSQL("INSERT INTO TabelaTeste (ID, Nome, Valor) VALUES (1, 'Exemplo UnixODBC', 123.45)")) {
        cerr << "Falha ao inserir dados. Encerrando." << endl;
        goto cleanup;
    }
    cout << "Inser��o conclu�da." << endl;

    // Embora o ODBC geralmente opere em autocommit, em alguns drivers ou configura��es de servidor, 
    // o INSERT pode ter sido submetido, mas a transa��o ainda n�o foi confirmada (committed) no banco de dados 
    // no momento em que o SELECT foi executado. O SELECT n�o consegue ver os dados que est�o em uma transa��o n�o 
    // confirmada por outro processo (ou pela mesma sess�o em alguns casos).    
    // *** PARA GARANTIR O COMMIT...  ***
    ret = SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleDiagnosticRecord(dbc, SQL_HANDLE_DBC, "SQLEndTran(COMMIT)");
    }  

    // --- Sele��o de Dados ---
    cout << "\nExecutando SELECT..." << endl;
    if (!executeSQL("SELECT ID, Nome, Valor FROM TabelaTeste ORDER BY ID")) {
        cerr << "Falha ao executar SELECT. Encerrando." << endl;
        goto cleanup;
    }

    // Recupera os resultados
    SQLINTEGER id;
    SQLCHAR nome[51];
    SQL_NUMERIC_STRUCT valor;
    
    SQLLEN idLen, nomeLen, valorLen;

    // Associa as colunas aos buffers (SQLBindCol)
    SQLBindCol(stmt, 1, SQL_C_LONG, &id, 0, &idLen);
    SQLBindCol(stmt, 2, SQL_C_CHAR, nome, sizeof(nome), &nomeLen);
    SQLBindCol(stmt, 3, SQL_C_NUMERIC, &valor, 0, &valorLen);

    cout << "--------------------------------------" << endl;
    cout << "ID | Nome | Valor" << endl;
    cout << "--------------------------------------" << endl;

    // Percorre o Result Set
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        // Converte o SQL_NUMERIC_STRUCT para double para exibi��o simples (com cautela)
        // Em produ��o, voc� deve usar um tratamento de precis�o mais robusto.
        double dValor = 0.0;
        for (int i = 0; i < valor.precision; ++i) {
            dValor = dValor * 10 + valor.val[i];
        }
        if (valor.sign == 0) dValor = -dValor;
        for (int i = 0; i < valor.scale; ++i) {
            dValor = dValor / 10.0;
        }

        cout << id << " | " << (char*)nome << " | " << fixed << setprecision(2) << dValor << endl;
    }
    cout << "--------------------------------------" << endl;
    SQLFreeStmt(stmt, SQL_CLOSE); // Fecha o cursor do SELECT

    // --- Limpeza (Exclui a Tabela) ---
    cout << "\nExcluindo a tabela TabelaTeste (limpeza)..." << endl;
    if (!executeSQL("DROP TABLE TabelaTeste")) {
        cerr << "Falha ao excluir a tabela." << endl;
    } else {
        cout << "Tabela exclu�da com sucesso." << endl;
    }

// 5. Limpeza de Handles e Desconex�o
cleanup:
    cout << "\nEncerrando a conex�o e liberando recursos..." << endl;
    if (stmt != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (dbc != SQL_NULL_HDBC) {
        SQLDisconnect(dbc);
        SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    }
    if (env != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, env);

    cout << "Programa conclu�do." << endl;
    return 0;
}
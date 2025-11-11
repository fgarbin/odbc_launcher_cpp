#ifndef ODBC_CONNECTOR_H
#define ODBC_CONNECTOR_H

// Interface pública que será exportada pela .so
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa e executa a rotina completa de conexão ODBC, INSERT e SELECT.
 * * @param username Usuário para conexão com o SQL Server.
 * @param password Senha para conexão com o SQL Server.
 * @return 0 em caso de sucesso, código de erro em caso de falha.
 */
int connect_and_run_sql(const char* username, const char* password);

#ifdef __cplusplus
}
#endif

#endif // ODBC_CONNECTOR_H
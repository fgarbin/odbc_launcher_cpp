#ifndef ODBC_LAUNCHER
#define ODBC_LAUNCHER

#define EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa e executa a rotina completa de conexão ODBC, INSERT e SELECT.
 * * @param username Usuário para conexão com o SQL Server.
 * @param password Senha para conexão com o SQL Server.
 * @return 0 em caso de sucesso, código de erro em caso de falha.
 */
EXPORT int connect_and_run_sql(const char * pDSN, const char * pUser, const char * pPwd);

#ifdef __cplusplus
}
#endif

#endif

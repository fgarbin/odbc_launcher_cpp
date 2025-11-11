#ifndef TSTSQLSERVER02
#define TSTSQLSERVER02

#ifdef __cplusplus
extern "C" {
#endif

int MYConnect( const char * pDSN, const char * pUser, const char * pPass );
int MYStatement( const char * pStmt );
int MYDisconnect();
/**
 * @brief Inicializa e executa a rotina completa de conexão ODBC, INSERT e SELECT.
 * * @param username Usuário para conexão com o SQL Server.
 * @param password Senha para conexão com o SQL Server.
 * @return 0 em caso de sucesso, código de erro em caso de falha.
 */
int connect_and_run_sql();

#ifdef __cplusplus
}
#endif

#endif

#ifndef TSTSQLSERVER02
#define TSTSQLSERVER02

#ifdef __cplusplus
extern "C" {
#endif

int MYConnect( const char * pDSN, const char * pUser, const char * pPass );
int MYStatement( const char * pStmt );
int MYDisconnect();

#ifdef __cplusplus
}
#endif

#endif

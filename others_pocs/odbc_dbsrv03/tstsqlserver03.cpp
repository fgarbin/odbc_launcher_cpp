#include <iostream>
#include <dlfcn.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "tstsqlserver03.hpp"

typedef int (*FPMYConnect)( const char *, const char *, const char * );
typedef int (*FPMYStatement)( const char * );
typedef int (*FPMYDisconnect)();

#define _DLLCALL_
#define _EXTERN_

#define TC_m4GLConnect v40TC_m4GLConnect

#define fnTC_GetTopConn             "v40TC_GetTopConn"
#define fnTC_m4GLConnect            "v40TC_m4GLConnect" 
#define fnTC_DisConnect             "v40TC_DisConnect" 

_EXTERN_ void*    (_DLLCALL_ *TC_GetTopConn4gl) ( short Type, short prefPort ); // 2=TCPIP 3=NPIPE
_EXTERN_ short   (_DLLCALL_ *TC_m4GLConnect) ( void* who, char* toServer, char* conn_str, char* usrname );

int main( int argc, char **argv)
{
  void* hLibrary = dlopen( "dbsrv.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND );
  
  if( hLibrary == NULL )
  {
    std::cout << "Could not load library: " << dlerror() << std::endl;
    return -1;
  }

  TC_GetTopConn4gl = (void* (_DLLCALL_ *) (short,short))
    dlsym( hLibrary, fnTC_GetTopConn );

  TC_m4GLConnect = (short  (_DLLCALL_ *) ( void* who, char* toServer, char* conn_str, char* usrname ))
  dlsym( hLibrary, fnTC_m4GLConnect );


  FPMYConnect lpfMYConnect = NULL;
  FPMYStatement lpfMYStatement = NULL;
  FPMYDisconnect lpfMYDisconnect = NULL;
  
 

  std::string sDSN = "utlogix_sql_local";
  std::string sUsername = "sa";
  std::string sPassword = "sa@12345678";
  std::string sServer   = "(local)";

  
  std::string sDbEnvConn = "@!!@";
  sDbEnvConn.append("MSSQL/");
  sDbEnvConn.append(sDSN.c_str());
  
  dlerror();
  
  void* TOPAux = TC_GetTopConn4gl( 2, 7890);

  int ret= TC_m4GLConnect ( TOPAux, (char*)sServer.c_str(), (char*)sDbEnvConn.c_str(), (char*)sUsername.c_str() );
  
  std::cout << "Disconnecting...." << std::endl;
  
finish:
  dlclose( hLibrary );
  
  return 0;
}

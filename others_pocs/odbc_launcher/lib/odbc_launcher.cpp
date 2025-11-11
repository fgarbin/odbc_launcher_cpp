#include <iostream>
#include <dlfcn.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "odbc_launcher.hpp"

#define _DLLCALL_
#define _EXTERN_
#define T_MAYBE_UNUSED_17 [[maybe_unused]]

#define TC_m4GLConnect v40TC_m4GLConnect

#define fnTC_GetTopConn             "v40TC_GetTopConn"
#define fnTC_m4GLConnect            "v40TC_m4GLConnect" 
#define fnTC_DisConnect             "v40TC_DisConnect" 

_EXTERN_ void*    (_DLLCALL_ *TC_GetTopConn4gl) ( short Type, short prefPort ); // 2=TCPIP 3=NPIPE
_EXTERN_ short   (_DLLCALL_ *TC_m4GLConnect) ( void* who, char* toServer, char* conn_str, char* usrname );
_EXTERN_ short   (_DLLCALL_ *TC_DisConnect4gl) ( void* who );

int connect_and_run_sql()
{
  void* hLibrary = dlopen( "./dbsrv.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND );
  
  if( hLibrary == NULL )
  {
    std::cout << "Could not load library: " << dlerror() << std::endl;
    return -1;
  }

  TC_GetTopConn4gl = (void* (_DLLCALL_ *) (short,short))
    dlsym( hLibrary, fnTC_GetTopConn );

  TC_m4GLConnect = (short  (_DLLCALL_ *) ( void* who, char* toServer, char* conn_str, char* usrname ))
  dlsym( hLibrary, fnTC_m4GLConnect );

  TC_DisConnect4gl = (short  (_DLLCALL_ *) ( void* who ))
  dlsym( hLibrary, fnTC_DisConnect );


  std::string sDSN = "utlogix_sql";
  std::string sUsername = "totvsvmtests";
  std::string sPassword = "totvs@123456";
  std::string sServer   = "192.168.15.142";

  
  std::string sDbEnvConn = "@!!@";
  sDbEnvConn.append("MSSQL/");
  sDbEnvConn.append(sDSN.c_str());
  
  dlerror();
  
  void* TOPAux = TC_GetTopConn4gl( 2, 7890);

  int ret= TC_m4GLConnect ( TOPAux, (char*)sServer.c_str(), (char*)sDbEnvConn.c_str(), (char*)sUsername.c_str() );
  
  if (ret == 0)
  {
    std::cout << "Disconnecting...." << std::endl;
    ret = TC_DisConnect4gl(TOPAux);
  }
  else
  {
    std::cout << "Connect Fail...." << std::endl;
  }
  
  
  dlclose( hLibrary );
  
  return 0;
}

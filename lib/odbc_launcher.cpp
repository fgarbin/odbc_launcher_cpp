#include <iostream>
#include <dlfcn.h>
#include <sqlext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <openssl/ssl.h> // Header do OpenSSL
#include <openssl/err.h> // Header do OpenSSL

#include "odbc_launcher.hpp"

using namespace std;

#define _DLLCALL_
#define _EXTERN_
#define T_MAYBE_UNUSED_17 [[maybe_unused]]

// Definicao do tipo de ponteiro da funcao que serah carregada dinamicamente
typedef int (* db_connect_ptr)(const char *DSN_NAME, const char *username, const char *password);
typedef int (* db_disconnect_ptr)();

/**
 * @brief Inicializa a OpenSSL 3 estaticamente.
 * @return true se bem-sucedido.
 */
bool initialize_openssl() 
{
#ifdef USE_OPENSSL  
    cout << "--- Initing OpenSSL (static) ---" << endl;
 
    #if USE_OPENSSL == 1
        // ROTINA DE INICIALIZAÃ‡ÃƒO CLÃ�SSICA (1.1.1 e anteriores)
        
        // Carrega strings de erro e todos os algoritmos de criptografia e digest
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        cout << "OpenSSL 1.1.1t initialized successsfuly." << endl;
    #else    
        // OpenSSL 3.0 simplificou a inicializaÃ§Ã£o
        if (SSL_library_init() != 1) { // JÃ¡ obsoleto, mas funciona para OpenSSL 1.x
            cerr << "ERROR: SSL_library_init failed." << endl;
            return false;
        }
        // Load algorithms and error strings
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        cout << "OpenSSL 3.0 initialized successsfuly." << endl;
    #endif 
#else
  cout << "Version not using OpenSSL." << endl; 
#endif
    cout << "----------------------------------------" << endl;
    return true;
}

EXPORT int connect_and_run_sql(const char * pDSN, const char * pUser, const char * pPwd)
{
  void* dbinterface_handle = nullptr;
  const char* dlsym_error = nullptr;

  if (!initialize_openssl()) 
  {
    return 1;
  }  

  cout << "--- Loading dynamic library (dbinterface.so) ---" << endl;
  dbinterface_handle = dlopen("./libdbinterface.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND );
  if( dbinterface_handle == nullptr )
  {
    std::cout << "Could not load library: " << dlerror() << std::endl;
    return -1;
  }

  // Mapeamento das funcoes
  db_connect_ptr db_connect_fn = (db_connect_ptr)dlsym(dbinterface_handle, "db_connect");
  dlsym_error = dlerror();
  if (dlsym_error) {
      cerr << "FATAL ERROR: Fail to map db_connect function: " << dlsym_error << endl;
      dlclose(dbinterface_handle);
      return 1;
  }
  cout << "Function 'db_connect' mapped successfully." << endl;

  db_disconnect_ptr db_disconnect_fn = (db_disconnect_ptr)dlsym(dbinterface_handle, "db_disconnect");
  dlsym_error = dlerror();
  if (dlsym_error) {
      cerr << "FATAL ERROR: Fail to map db_connect function db_disconnect: " << dlsym_error << endl;
      dlclose(dbinterface_handle);
      return 1;
  }
  cout << "Function 'db_disconnect' mapped successfully." << endl;

  // iniciando operacoes no banco
  cout << "\n--- Initializing ODBC/SQL Server Module---" << endl;
  int result = db_connect_fn(pDSN, pUser, pPwd);

  // Limpeza
  cout << "\n--- Free resources ---" << endl;
  dlclose(dbinterface_handle);

  if (result == 0) {
    cout << "Sucess: Program ODBC_LAUNCHER execution successfully." << endl;
  } else {
    cerr << "Fail: Program ODBC_LAUNCHER return error code: " << result << endl;
  }  

  return 0;
}


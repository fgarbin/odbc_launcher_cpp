#include <iostream>
#include <dlfcn.h>      // Para carregamento dinÃ¢mico (.so)


#include "odbc_launcher.hpp"

using namespace std;

/**
 * @brief Carrega a biblioteca ODBC dinamicamente e executa a funcao.
 */
int main( int argc, char **argv )
{
  if( argc < 4 )
  {
    std::cout << "Invalid number of parameters." << std::endl;
    std::cout << "Parameters: DSN USERNAME PASSWORD" << std::endl;
    exit( 1 );
  }
  
  std::string sParm1 = argv[1];
  std::string sParm2 = argv[2];
  std::string sParm3 = argv[3];

  
  int iRet = 0;

  if( sParm1.empty() || sParm2.empty() || sParm3.empty() )
  {
    std::cout << "Invalid parameters informed." << std::endl;
    return 1;
  }

  cout << "--- Loading app_odbc_launcher dynamically --- " << endl; 
  int result = 0;
  result = connect_and_run_sql(sParm1.c_str(), sParm2.c_str(), sParm3.c_str());

  if (result == 0) {
      cout << "Sucess: Program MAIN execution successfully." << endl;
  } else {
      cerr << "Fail: Program MAIN return error code: " << result << endl;
  }

  return result;
}
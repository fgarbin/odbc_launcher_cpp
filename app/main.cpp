#include <iostream>
#include <dlfcn.h>      // Para carregamento dinÃ¢mico (.so)


#include "odbc_launcher.hpp"

using namespace std;



/**
 * @brief Carrega a biblioteca ODBC dinamicamente e executa a funÃ§Ã£o.
 */
int main() 
{
    cout << "--- Carregando o app_odbc_launcher dinamicamente --- " << endl; 
    // Chama direto a funcao da lib linkada dinamicamente
    int result = connect_and_run_sql();

    if (result == 0) {
        cout << "SUCESSO: Rotina completa executada sem falhas." << endl;
    } else {
        cerr << "FALHA: A rotina SQL retornou codigo de erro: " << result << endl;
    }

    return result;
}
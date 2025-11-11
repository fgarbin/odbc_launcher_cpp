# 1. Navegue para o diretório da biblioteca
cd lib

# 2. Compile o módulo como uma biblioteca compartilhada
# -fPIC (Position Independent Code) é essencial para .so
# -lodbc liga com o unixODBC
g++ -std=c++17 -fPIC -shared odbc_connector.cpp -o ../libconnector.so -lodbc

# 3. Volte ao diretório raiz
cd ..

MAIN

# Definir o caminho raiz do OpenSSL para simplificar
OPENSSL_ROOT="/mnt/data/totvs/projects/advtec9/branches/appserver-24.3.0.x/third_libraries/openssl/1.1.1t/linux64/gcc-8.3.1"
ou
OPENSSL_ROOT="/mnt/data/totvs/projects/advtec9/branches/appserver-24.3.0.x/third_libraries/openssl/3.0.13/linux64/gcc-8.3.1"
OPENSSL_INC="${OPENSSL_ROOT}/include"
OPENSSL_LIB="${OPENSSL_ROOT}/releaase"


# Compilar o executável (main.cpp)
# -I: inclui os headers do OpenSSL
# -L: inclui o diretório das libs OpenSSL
# -Wl,--start-group: força a ligação estática correta entre libssl e libcrypto
# -ldl: necessário para as chamadas de carregamento dinâmico (dlopen, dlsym)
# -lrt -lpthread: libs de sistema frequentemente necessárias pelo OpenSSL
g++ -std=c++17 app/main.cpp -o app_launcher \
    -I${OPENSSL_INC} \
    -L${OPENSSL_LIB} \
    -Wl,--start-group -lssl -lcrypto -Wl,--end-group \
    -ldl -lrt -lpthread
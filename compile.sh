#!/bin/bash

# 1. Verifica se a variável NÃO está vazia/indefinida (-n)
if [[ -n "$USE_OPENSSL" ]]; then    
    # 2. Se for definida, verifica se é 1 OU 3
    if [[ "$USE_OPENSSL" == "1" ]]
    then 
        echo "*** Building using OPENSSL 1"
    elif [[ "$USE_OPENSSL" == "3" ]]
    then
        echo "*** Building using OPENSSL 3"
    else
        echo "*** Wrong version, using default OPENSSL 1"
    fi

    if [[ -n "$OPENSSL_ROOT" ]]; then
      export OPENSSL_INC="${OPENSSL_ROOT}/include"
      export OPENSSL_LIB="${OPENSSL_ROOT}/release" 

      echo "*** Using OPENSSL_ROOT: ${OPENSSL_ROOT}"
      echo "*** Using OPENSSL_INC:  ${OPENSSL_INC}"
      echo "*** Using OPENSSL_LIB:  ${OPENSSL_LIB}"
    else
      echo "*** Error: OPENSSL_ROOT must be defined."
    fi   
else
    echo "*** Building without OpenSSL."
fi


echo "*** Entering directory lib..."
cd lib

echo "*** Build libodbc_launcher.so ..."

if [[ -n "$USE_OPENSSL" ]]; then    
    g++ -std=c++17 -DUSE_OPENSSL=${USE_OPENSSL} -fPIC -shared odbc_launcher.cpp -o libodbc_launcher.so \
        -I${PWD} \
        -I${OPENSSL_INC} \
        -L${OPENSSL_LIB} \
        -L${PWD} \
        -lssl -lcrypto \
        -ldl -lrt -lpthread 
else
    g++ -std=c++17 -fPIC -shared odbc_launcher.cpp -o libodbc_launcher.so \
        -I${PWD} \
        -L${PWD} \
        -ldl -lrt -lpthread 
fi
echo "*** Exiting directory lib..."
cd ..

echo "*** Copying libodbc_launcher.so..."
cp -fv ${PWD}/lib/libodbc_launcher.so .


echo "*** Building app_odbc_launcher... "
g++ -std=c++17 app/main.cpp -o app_odbc_launcher \
    -I${PWD}/lib \
    -L${PWD}/lib \
    -lodbc_launcher

echo "*** Finished."



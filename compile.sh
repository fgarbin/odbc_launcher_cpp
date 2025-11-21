#!/bin/bash

# 1. Verifica se a variável NÃO está vazia/indefinida (-n)  
export WITH_DBINTERFACE=1
echo "*** Building using DBINTERFACE"

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
      export OPENSSL_LIB="${OPENSSL_ROOT}/lib" 

      echo "*** Using OPENSSL_ROOT: ${OPENSSL_ROOT}"
      echo "*** Using OPENSSL_INC:  ${OPENSSL_INC}"
      echo "*** Using OPENSSL_LIB:  ${OPENSSL_LIB}"
    else
      echo "*** Error: OPENSSL_ROOT environment variable must be defined."
      exit $?
    fi   
else
    echo "*** Building without OpenSSL."
fi


echo "*** Entering directory lib..."
cd lib

echo "*** Build libodbc_launcher.so ..."

if [[ -n "$USE_OPENSSL" ]]; then    
    g++ -g -std=c++17 ${PARM_DBSRV} -DUSE_OPENSSL=${USE_OPENSSL} -fPIC -shared odbc_launcher.cpp -o libodbc_launcher.so \
        -I${PWD} \
        -I${OPENSSL_INC} \
        -L${OPENSSL_LIB} \
        -L${PWD} \
        -lssl -lcrypto \
        -ldl -lrt -lpthread -lstdc++ 
else
    g++ -g -std=c++17 ${PARM_DBSRV} -fPIC -shared odbc_launcher.cpp -o libodbc_launcher.so \
        -I${PWD} \
        -L${PWD} \
        -ldl -lrt -lpthread -lstdc++ 
fi
echo "*** Exiting directory lib..."
cd ..

echo "*** Copying libodbc_launcher.so..."
cp -fv ${PWD}/lib/libodbc_launcher.so .

# with dbinterface
if [[ -n "$WITH_DBINTERFACE" ]]; then    
    echo "*** Entering directory lib_dbinterface..."
    cd lib_dbinterface

    echo "*** Build lib_dbinterface.so ..."
    g++ -g  -std=c++17 -fPIC -shared dbinterface.cpp -o libdbinterface.so \
        -I${PWD} \
        -L${PWD} \
        -ldl -lrt -lpthread -lstdc++ 

    echo "*** Exiting directory lib_dbinterface..."
    cd ..

    echo "*** Copying lib_dbinterface.so..."
    cp -fv ${PWD}/lib_dbinterface/libdbinterface.so .    
fi

echo "*** Building app_odbc_launcher... "
g++ -g -std=c++17 ${PARM_DBSRV} app/main.cpp -o app_odbc_launcher \
    -I${PWD}/lib \
    -L${PWD}/lib \
    -lodbc_launcher

echo "*** Finished."



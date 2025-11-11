# odbc_launcher_cpp
SQL Server Connector Proof-of-Concept (C++ / UnixODBC / ODBC Driver 18)

This repository contains a minimalist C++ Proof of Concept (PoC) designed to establish and validate SQL Server connectivity from Linux/Unix environments (such as Oracle Linux, Linux Mint, Ubuntu, etc.).

The project focuses on leveraging the Microsoft ODBC Driver 18 (msodbcsql18) and the unixODBC driver manager as a bridge for the C++ application.

# Main 
## libodbc_launcher (dynamic load)
### dbsrv (dlopen)
#### unixodbc (dlopen)
##### driver sql server tools 18 (dlopen?????)

# Environment variables
export USE_OPENSSL=1
export OPENSSL_ROOT="/mnt/data/totvs/projects/advtec9/branches/appserver-20.3.2.x/third_libraries/openssl/1.1.1t/linux64/gcc-8.3.1"

export USE_OPENSSL=3
export OPENSSL_ROOT="/mnt/data/totvs/projects/advtec9/branches/appserver-24.3.0.x/third_libraries/openssl/3.0.13/linux64/gcc-8.3.1"

dbsrv.so
dbaccess.ini


Enjoy!
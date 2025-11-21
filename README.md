# odbc_launcher_cpp
SQL Server Connector Proof-of-Concept (C++ / UnixODBC / ODBC Driver 18)

This repository contains a minimalist C++ Proof of Concept (PoC) designed to establish and validate SQL Server connectivity from Linux/Unix environments (such as Oracle Linux, Linux Mint, Ubuntu, etc.).

The project focuses on leveraging the Microsoft ODBC Driver 18 (msodbcsql18) and the unixODBC driver manager as a bridge for the C++ application.

---

## Main
```
| - main
| -- libodbc_launcher (dynamic load)
|  --- dbinterface (dlopen)
|  ---- unixodbc (dlopen)
|  ----- driver sql server tools 18 (dlopen?????)
```

---

## Build

### Environment variables

#### Use OpenSSL1
```
export USE_OPENSSL=1 
export OPENSSL_ROOT="/home/user/openssl/1.1.1t/"
```

#### Use OpenSSL3
```
export USE_OPENSSL=3 
export OPENSSL_ROOT="/home/user/openssl/3.0.13"
```

#### without OpenSSL
```
unset USE_OPENSSL
```
#### OpenSSL Libs

OPENSSL libs used are available at openssl folder.


### Build command
```
chmod +x compile.sh 
./compile.sh
```

---

## Run

### Default Mode
```
export LD_LIBRARY_PATH=. 
./app_odbc_launcher dsn  username password
```

---

Enjoy!


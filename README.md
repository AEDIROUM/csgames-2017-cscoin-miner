# CSCoin Miner

The ultimate CSCoin miner, at your service!

## Usage

```
openssl genrsa -out <wallet_file> 1024

cscoin-miner --wallet=<wallet_file> <ws_url>
```

If the wallet does not exist, it will be automatically created.

## Features

 - aggressively optimized OpenMP-based solver
 - libsoup-2.4 for WebSocket
 - OpenSSL for the public key crypto related to the wallet

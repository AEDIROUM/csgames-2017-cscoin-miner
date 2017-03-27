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
 
## Results

Although we could not complete the `shortest_path` challenge, we ended up second! We have mined a total of 1070 CSCoin and spent 325 of them on the Puzzle Hero challenge. The whole transaction set is available at https://cscoins.2017.csgames.org/ and our key id is `a1aeb9bd9bab2594933f4fb9df4fdbeed5c749ce6339873d364b3040933673de`.

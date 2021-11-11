# BlockO Blockchain
 
## A Quick And Simple Blockchain

This library and application is designed to provide a simple to use blockchain.

This may also help you understand the basic concepts and principles that go into a blockchain.

When running the application, the existing blockchain will be loaded and verified. Because the private keys are not shared, you will not be able to add blocks to the current blockchain. You'll need to generate a new blockchain by using the `newchain` command. See below for more details.

Current Commands:
```
newchain <chain-name>
newkey <key-name>
database <file-path>
key <private-key-file-path>
ownerkey <public-key-file-path>
addblock <block-index> <data-field>
printchain
printblock <block-index>
```

*All parameters to the commands are required
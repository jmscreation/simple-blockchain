# The Simple Blockchain
 
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


## To Build (Windows)

If using the provided build script:

Make sure you have the MinGW64 `bin` folder configured in your system environment path.

You'll also need to clone the [libtomcrypt](https://github.com/jmscreation/libtomcrypt) static library binaries to: `libraries/libtomcrypt-main`
You can also use the [dependency tracker](https://github.com/jmscreation/dependency-tracker) to automate this process.

Run the `build.bat` script

### Note:
Building on Linux requires the libtomcrypt and libtommath static library binaries for Linux. You can install this package using `sudo apt-get install libtomcrypt-dev` or whichever package manager you use.

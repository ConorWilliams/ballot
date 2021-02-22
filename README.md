# MCR room ballot

Welcome to the Churchill MCR Postgrad room ballot code please read on to find out how to run the ballot code if you are the computing officer this year **and** how you can verify that the room ballot was run honestly this year!

## Installation
To build the codebase you will need a c++ compiler supporting c++17, git, make, and a version of cmake greater than or equal to 3.14. Then follow the standard procedure:


```zsh
git clone https://github.com/ConorWilliams/ballot
cd ballot
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

You will now have your very own copy of the `ballot` executable!
 

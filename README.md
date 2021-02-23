# MCR room ballot

Welcome to the Churchill MCR postgraduate-room-ballot code, please read on to find out how to: run the ballot code if you are the computing officer this year **and** how you can verify that the room ballot was run honestly this year!

## Installation
To build the codebase you will need a c++ compiler supporting c++20, git, make, and a version of cmake greater than or equal to 3.14. Then follow the standard procedure:


```zsh
git clone https://github.com/ConorWilliams/ballot
cd ballot
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

You will now have your very own copy of the `ballot` executable!

## Running the ballot

First up you're your going to need a csv file containing everyone's room preferences see `example/example.csv` to get an idea of the expected format. Now the ballot like:

 `../build/ballot example.csv`

This will generate two file `public_ballot.csv` and `secret_ballot.csv`. The first can be distributed and used by members of the MCR to anonymously verify the ballot was run fairly. The second contains each persons room assignment and some additional info. You should email each student there result and their "secret_name" (last field in `secret_ballot.csv`).

If you would like to encourage particular rooms to fill up (the hostels) then you can pass in a list of prefixes. For example: 

`../build/ballot example.csv -h RR CJ`

would preferentially fill all rooms beginning with the letters "RR" or "CJ".

Finally you can control the total number of allocated rooms using the `-m` or `--max-rooms` options.

## Verifying the ballot

To verify the MCR computing officer hasn't fiddled your position you need a copy of the `secret_ballot.csv` file they generated and and your "secret_name" which you should have received securely. Now run:

`.ballot secret_ballot.csv -c YOUR_SECRET_NAME ...`

where the ...

## Details about the ballot
 

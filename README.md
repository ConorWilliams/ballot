<br />
<p align="center">
  <img src="./logo/ChuLion.png" height="250" />
</p>
<br />

# MCR Room Ballot

Welcome to the Churchill MCR postgraduate-room-ballot code, please read on to find out how to: 

1. Run the ballot code if you are the computing officer this year.
2. Verify that the room ballot was run honestly!

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

Alternatively 64-bit Linux binaries are provided with each [release](https://github.com/ConorWilliams/ballot/releases).

## Running the ballot

First up you're your going to need a csv file containing everyone's room preferences see `example/example.csv` to get an idea of the expected format. Now compile the program, navigate to `example/` and run the ballot like:

 `../build/ballot run example.csv`

This will generate two files `public_ballot.json` and `secret_ballot.csv`. The first can be distributed and used by members of the MCR to anonymously verify the ballot was run fairly. The second contains the room assignments and some additional info. You should email each student their result, "id" and "secret_name" (last three fields in `secret_ballot.csv` respectively).

If you would like to encourage particular rooms to fill up (e.g. the hostels) then you can pass in a list of prefixes, for example: 

`../build/ballot run example.csv -h RR CJ`

would preferentially fill all rooms beginning with the letters "RR" or "CJ".

Finally you can control the total number of allocated rooms using the `-m` or `--max-rooms` options.

## Verifying the ballot

To verify the MCR computing officer hasn't fiddled your position you need a copy of the `public_ballot.json` file they generated, your "id" and "secret_name" which you should have received securely. Now run:

`./ballot verify YOUR_ID YOUR_SECRET_NAME`

where you can supply the optional flag `-i /path/to/public_ballot.json` to specify the location of the public ballot file if it is not in your current working directory.

## Details about the ballot

The ballot code formulates the task as solving the balanced linear [assignment problem](https://en.wikipedia.org/wiki/Assignment_problem). We define a [cost function](src/cost.hpp) which assigns a value to allocating any student to any room. The student-room pairs are then permuted until the global minimum of the cost function (value summed over all pair) is found. This is done using the [Jonker-Volgenant algorithm](https://doi.org/10.1007/BF02278710). 

In order to allow the possibility that all students get kicked off the ballot the list of rooms is augmented with p (the number of people) "kicked-rooms". To ensure balanced assignment, preference-free null-people are appended to the list of people. 

If the total number of allocated rooms needs to be limited the lowest priority students are removed from the ballot.  

### The cost function

In summary, the cost function prioritises people getting their first choices but prefers kicking people off the ballot to assigning them to a room they didn't want. Finally, the cost function ensures all houses designated as "hostels" are preferentially filled.


 

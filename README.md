# af-learner v1.0 (04-2023)

Compile via
```
    ./build.sh
```

Algorithm for learning argumentation frameworks from a given set of labelings.

Works with an integrated cryptominisat5 solver.
Also supports any external SAT solver that can read dimacs input from <stdin>.
Build the SAT solver seperately and provide the link to the executable in 'af-learner.sh'.

Example usage:
```
  ./af-learner.sh -p DS-PR -fo tgf -f <file in TGF format> -a <argument>
```

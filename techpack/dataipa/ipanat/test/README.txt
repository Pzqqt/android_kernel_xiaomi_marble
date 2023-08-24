INTRODUCTION
------------

The ipanattest allow its user to drive NAT testing.  It is run thusly:

# ipanattest [-d -r N -i N -e N -m mt]
Where:
  -d     Each test is discrete (create table, add rules, destroy table)
         If not specified, only one table create and destroy for all tests
  -r N   Where N is the number of times to run the inotify regression test
  -i N   Where N is the number of times (iterations) to run test
  -e N   Where N is the number of entries in the NAT
  -m mt  Where mt is the type of memory to use for the NAT
         Legal mt's: DDR, SRAM, or HYBRID (ie. use SRAM and DDR)
  -g M-N Run tests M through N only

More about each command line option:

-d    Makes each test discrete; meaning that, each test will create a
      table, add rules, then destory the table.

      Conversely, when -d not specified, each test will not create
      and destroy a table.  Only one table create and destroy at the
      start and end of the run...with all test being run in between.

-r N  Will cause the inotify regression test to be run N times.

-i N  Will cause each test to be run N times

-e N  Will cause the creation of a table with N entries

-m mt Will cause the NAT to live in either SRAM, DDR, or both
      (ie. HYBRID)

-g M-N Will cause test M to N to be run. This allows you to skip
       or isolate tests

When run with no arguments (ie. defaults):

  1) The tests will be non-discrete
  2) With only one iteration of the tests
  3) On a DDR based table with one hundred entries
  4) No inotify regression will be run

EXAMPLE COMMAND LINES
---------------------

To execute discrete tests (create, add rules, and delete table for
each test) one time on a table with one hundred entries:

# ipanattest -d -i 1 -e 100

To execute non-discrete (create and delete table only once) tests five
times on a table with thirty-two entries:

# ipanattest -i 5 -e 32

To execute inotify regression test 5 times

# ipanattest -r 5

ADDING NEW TESTS
----------------

In main.c, please see and embellish nt_array[] and use the following
file as a model: ipa_nat_testMODEL.c

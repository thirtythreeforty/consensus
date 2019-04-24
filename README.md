# Consensus
This is a clean, elegant analog watchface for Pebble watches.
It is notable for its early integration of the Health API, its customizable "complications," its relatively large userbase, and the use of C++ as its implementation language.
The binary fits in about 16KB of space on the watch.

You can see it in the Rebble store [here](https://apps.rebble.io/en_US/application/567312691a9a82d62800003d?native=false&query=consensus&section=watchfaces).

## Design
Most of the Pebble API is wrapped using a small wrapper named "boulder."  Other portions of it were an exercise in writing resource-constrained modern C++ and might not be reusable outside of the project.
Most of the code is not exception-safe; exceptions are disabled due to limited space on the watch.

## Status
Consensus is considered finished.  I am open to community contributions, but I will not be doing much further work on it.

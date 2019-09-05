# Mountain Programming Language
## A (hopefully) fast, C compatible, language designed to enable greatness


### Welcome to the Github repo for the (WIP) Mountain bootstrap compiler

The goal is to have a self hosted compiler by the end of
summer 2020. At the moment Mountain rather far away from that goal.


### Core Tenants

Mountain has several core tenants
* Expressiveness without sacrificing runtime performance.
* Explicitness is important for readability.
    * Explicit != Verbose
* Seamless C interop is important for usefulness.
* It is not the language's job to enforce a specific coding "style".



# This language is nowhere near usable for literally anything
This is currently little more than a fun side project with hopes of
eventually becoming more. Read the feature wishlist
[here](WISHLIST.md) for more information about what Mountain hopes to
be capable of. Feel free to poke around.

Be aware that this is *extremely* WIP. There are many areas where
things should change or be improved. Performance could be improved by
fixing a few rather egregious design cases (go look at
lookup_symbol). However I don't really care right now. This compiler
is being designed to be good enough to be useful and then be thrown
out. My main goal right now is to get the langauge useful. Then a far
superior self-hosted compiler with better validation, performance, and
even a language server can be developed in-language.

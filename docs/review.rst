======
Review
======


suCGI
=====

Design by contract
------------------

* Are each of the following checked
  (either by production code or by assertions):
    * Unacceptable input values (in the called function)?
    * Unacceptalbe return values (in the caller)?
* If a function handles strings, do assertions check string length?

Error handling
--------------

* Is errno set to 0 before each call of a function that sets errno?
* Do switch statements cover all possible cases?

Over-/underflow
---------------

* Has it been shown, for each calculation and typecast, that it either
* cannot over-/underflow or is over-/underflow checked for?
* Has it been shown, for each array and pointer,
  that array operations cannot overflow?
* Have all calculations, arrays, and pointers been
  checked for off-by-one errors?

Attributes
----------

* Review function attributes.

Coding style
------------

* Does the code conform to MISRA C 2012, SEI CERT C, and <insert name>,
  and have exceptions been documented in the suppression file and/or in code?
* Has the code been reviwed against 4.4BSD, POSIX.1-2008, the most recent
  version of POSIX, and the manuals of all targeted operating systems?
* Have the correct feature test macros been defined?
* Has the code been reviewed against the CVE and CWE databases?
* Does the test suite pass?
* Has the output of the test suite been reviewed?
* Does static code analysis pass?
* Has the programme flow been reviewed?
* Are only those headers included that are actually needed?
* Review style adherence to style.


Tests
=====

Coverage
--------

Check if the test suite covers the following cases:

* Paradigmatic values
* Erroneous values
* Real-world values
* Edge cases
* A range of automatically generated values
* Every reachable branch

If the function works on a string:

* The empty string
* A string that is just as long as the function permits
* A string that is longer than the function permits

If the function works on a filename:

* "/"
* "."
* Absolute filenames
* Relative filenames
* Filenames with spaces
* Filenames with characters outside the portable characters
* UTF-8 encoded filenames
* Filenames that start with/contain "./", "/./", "/../", etc.

* Do shell-based tests test for all assertions?


Code quality
------------

* Do tests pass static analysis?


Documentation
=============

* Is the documentation correct, complete and accurate?
* Is it terse?
* Does documentation follow
  <https://www.cs.odu.edu/~zeil/cs350/latest/Public/docgen/index.html>?

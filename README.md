# spotlight-synthesis

The way the script keeps track of what synthesis jobs have been submitted already is by looking at condorfiles.
If you stop running jobs and want to rerun (or if you want to rerun errored jobs)
you have to use clean.sh. It may only remove errored jobs.


Errata:
 - Conv1D accelerator is the Eyeriss accelerator. I never renamed it. Oops.

TODO:
 - Change all format strings to fstrings
 - Change simple AXI to streaming AXI

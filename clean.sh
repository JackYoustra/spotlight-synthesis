grep -r -l "error" condorfiles | cut -d '/' -f 2 | cut -d '.' -f 1 | xargs -I % rm tcl/%.tcl

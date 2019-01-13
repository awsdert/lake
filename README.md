# lu
Tired of make or nmake? Tired of struggling to set up your variables
properly thanks to make's/nmake's poor functionality? Tired of
struggling to pass enviroment variables to the shell? Dislike having
to set enviroment variables in windows just to run make?

So am I! I created this program because I got sick and tired of
struggling to do that s**t! With this libraries can get themselves
compiled WITHOUT affecting a project's flags, a project can grab
whatever system info it needs with or without setting a shell's
enviroment variables, something as basic as math and string manipulation
can be done WITHOUT calling the shell, honestly I surprised the lua
developers didn't think to do this themselves. As for the name,
I orginally wanted "lu" (made that by taking the "l" of lua and the
"ake" of make) but that was taken by ones not following the moto
"know nothing, make everything" so I chose lu after checking it wasn't
already being used, simple enough right?

I'm primarily compiling with the lua sources to increase portability,
in other words one could just dump the executable in the directory of
their favourite editor/ide and have it respond normally with the
command "lu" without needing a batch/shell file to add it to the PATH
variable, personally I favour GeanyPortable for this since geany itself
is cross-platform.

I'm aiming to follow lua's moto of no special flags where-ever possible,
although for now it does require a few gcc options like -mconsole on windows

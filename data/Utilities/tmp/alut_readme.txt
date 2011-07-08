Temporary until we make out how to fix the framework code with OpenAL :

replace both libraries in your freealut v1.1 vc8 directory
and put the dll in the main Legacy folder together with the others dll

LC workaround alutLoadWAVMemory function : 

Hi there all,

I've finally tamed the OpenAL bug ! Ok : here it's all about it - and as you 
can see reading this post we'll have to have some structural decisions for 
the code frame !

First, not only alutLoadWAVMemory is deprecated (strongly deprecated by its 
author) but it is also bugged !!! I had to watch the src code to figure out 
that **data couldn't never being produced.

So I fixed the alut src function and compiled the libraries, which I join to 
this post (note : refused by source forge than committed to the CVS).

So you need to put the new alut.dll in your main Legacy folder and in your 
vc8 freealut 1.1 lib folder directory together with its alut.lib for the 
code compilation.
Then you also need to compile all the Legacy sln again to fix all the 
problems.

Now switches have a proper sound !
Please, confirm it's all the same for you ...

The decisions we have to take are all about the need of such a tricky 
workaround the bugged function : before anyfurther i'll see if we can use 
another alu function not deprecated this time ...

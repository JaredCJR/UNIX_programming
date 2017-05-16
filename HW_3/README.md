0556054   張家榮   jaredcjr.tw@gmail.com
==============================================

* All the features requested by the homework are done.

How to build and run:
===================================
```
cd /path/to/the/hw
make clean
make
./hw3
```

How to exit the shell:
================================
"Ctrl + c" , "Ctrl + \\" , "Ctrl + z" are handled, so you need to type "exit" to exit.
```
shell-prompt$ exit
```

[10%] Execute a single command.
---------------------------------------------
Ex.
```
shell-prompt$ ls -al
```

[10%] Properly block or unblock signals.
---------------------------------------------
Ex. Ctrl+c , Ctrl+\ , Ctrl+z will not affect the shell but the launched commands
```
shell-prompt$ ^C
shell-prompt$ find /         <-- which will produce lots of messages,then ctrl+c will stop it.
```

[10%] Replace standard input/output of a process using the redirection operators (< and >).
---------------------------------------------------------------------------------------------
Ex. Redirection in and out is perfect.
```
shell-prompt$ cat /etc/passwd > /tmp/x     <-- which will see the corresponding contents
shell-prompt$ less < /tmp/x
```

[10%] Setup foreground process group and background process groups.
---------------------------------------------------------------------------------------------
Ex. Keyboard operations can send to the right foreground processes
```
shell-prompt$ less /etc/passwd       <-- up/down arrow or page up/down works 
```

[10%] Pipeline for commands separated by the pipe operator (|), and in the same process group.
-----------------------------------------------------------------------------------------------
Ex. First command input is STDIN, and last command output to STDOUT.Others uses pipe to create pipeline.
```
shell-prompt$ cat /etc/passwd | cat | less
```

[10%] Manipulate environment variables: provide commands like export and unset.
-----------------------------------------------------------------------------------------------
Ex. "%export [var_name]=[what you want]"  and "%unset [var_name]" works
```
shell-prompt$ export 0556054=jia-rung
shell-prompt$ printenv | less       <-- use '/' to find 0556054
shell-prompt$ unset 0556054
shell-prompt$ printenv | less       <-- You are not able to find "0556054"
```

[10%] Expand of the * and ? operators.
-----------------------------------------------------------------------------------------------
Ex. Suppose "main.cpp" and "main.cpp.o" are in the current working directory.
```
shell-prompt$ ls main*
(Its output:)
/home/jared/workspace/UNIX_programming/HW_3/main.cpp  /home/jared/workspace/UNIX_programming/HW_3/main.cpp.o

shell-prompt$ ls main.c?
(Its output:)
ls: cannot access 'main.c?': No such file or directory

shell-prompt$ ls main.cpp.?
(Its output:)
/home/jared/workspace/UNIX_programming/HW_3/main.cpp.o
```


[30%] Job control: support process suspension using Ctrl-Z, and jobs, fg and bg command.
-----------------------------------------------------------------------------------------------
Ex.Use ps aux | grep [pid] to check the results.
```
shell-prompt$ less /etc/passwd
(press ctrl+z in the "less")
shell-prompt$ jobs
[1]  Stopped  pgid=18022
shell-prompt$ sleep 10
(press ctrl+z)
^Zshell-prompt$
(press enter)
shell-prompt$ jobs
[1]  Stopped  pgid=18022       less /etc/passwd
[2]  Stopped  pgid=18027       sleep 10
(USE "ps aux | grep 18027" will see its status is "T")
shell-prompt$ bg
(USE "ps aux | grep 18027" will see its status is "S" or "Z")
shell-prompt$ fg
(bring back "less /etc/passwd" into foreground)
```

Another example:
```
shell-prompt$ cat /etc/passwd | cat | cat | cat | less
(press ctrl+z)
shell-prompt$ jobs
[1]  Stopped  pgid=5001       cat /etc/passwd | cat | cat | cat | less
shell-prompt$ fg
(bring back "cat /etc/passwd | cat | cat | cat | less")
```


* Test "&" background execution
```
shell-prompt$ find / &
(You will not be able to control "find" with your keyboard<ctrl+c,ctrl+z,ctrl+\>)
```



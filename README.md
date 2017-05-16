Program: Micro-Shell

Implement the Unix micro-shell, ush (ie, μ-shell) described in the man page. Do not to implement job control. Therefore, do not implement the following:

  the fg command,
  the bg command,
  the kill command,
  the jobs command, or
  the & operator.

----------

Notes:

1. The parsing routine is pre-provided (parse.h & parse.c).
2. Not used the system library routine.
3. The shell will be forking other processes, which may in turn fork off other processes, etc. Forking too many processes will cause Unix to run out of process descriptors, making everyone on the machine very unhappy. Therefore, be very careful when testing fork. To kill a horde of runaway ush processes, use the <i>killall</i> console command, which will kill every process of a given name.
4. Major tasks performed:
	
	
	Command line parsing:
			
			rc file (~/.ushrc)
			hostname%
			special chars:
			  & | ; < > >> |& >& >>&
			backslash
			strings
			commands:
			  <b>cd echo logout nice pwd setenv unsetenv where</b>
	
	Command execution
			
			fork
			exec family: execvp
	
	I/O redirection
			
			open, close (not fopen, fclose)
			dup, dup2
			stdin, stdout, stderr
			
	Environment variable
			
			putenv, getenv
	
	Signal handling
    		
			signal
			getpid, getpgrp, getppid, getpgid
			setpgid, setpgrp


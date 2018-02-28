# travis-tools

Just some tools I've written for running jobs no CI systems
like Travis, Shippable, etc.

## runlog

This is a program similar to the `travis_wait` function - it
runs a long-running command with output logged to a file. It
occasionally prints a character to prevent timeout errors. If
you hit the maximum runtime in minutes, it send a TERM signal,
followed by a KILL signal. It exits with the same exit code or
signal that your command exited with.

By default, it prints a period character every 30 seconds, and
times out after 30 minutes. The log file name is generated randomly
and printed on the first line of stdout. Each part of this can
be customized:

```
runlog [-s sleeptime] [-c sleepchar] [-t timeout] [-l /path/to/log] [-h] -- program
```

  * `sleeptime` is how often to print your character, in seconds
  * `sleepchar` is the character (or string, if you want) to be printed.
  * `timeout` is how long to wait before sending a TERM/KILL, in minutes
  * `--` (optional) can signal that you're no longer reading options.

So for example:

```
runlog -s 1 -c 💩 -t 5 -- sleep 600
```

Will run the command "sleep 600" in the background. In the foreground,
it will print the "💩" character every second for a maximum of 5 minutes.
Once it reaches 5 minutes, it will send a TERM signal to sleep.

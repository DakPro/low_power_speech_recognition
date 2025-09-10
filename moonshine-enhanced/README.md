# Moonshine-enhanced

This directory is a on-start setup of a model. **Make sure your device is connected to the internet before setup!**

To use it, just copy moonshine-enhanced into your home directory and run
<code>self_start_setup</code>:

    ./moonshine-enhanced/self_start_setup.sh

After that, the model will start up itself on every load of the
operating system.

Current primitive version of the model implements "start/stop recording" commands. They are reconized
using simple regex. The text during recording state is appended to a <code>recording.txt</code>.
Light Programmer
================

Using a computer monitor that flashes black and white I'm able to push text into a microcontroller. I've used an ATmega168 for prototyping, with the analog comparator connected to the input capture feature of TIMER1. On every interrupt I change the edge sensing to capture the next.

Manchester Coding
-------------------

By following the [Atmel Application Note](http://atmel.com/dyn/resources/prod_documents/doc9164.pdf) I decided on timing based decoding. I may try sample based in the future to see if I can increase relability.

On the sending end I'm using a JavaScript file to flash the background of a webpage. I think this is where the majority of my timing issues are created. It works maybe 40% of the time on my desktop computer. But never works on my netbook, or on an iOS device. This is probably due to sloppy timing with this browser-based approach

More Information
----------------

I'll be posting on my blog about this:
[http://jumptuck.com/category/projects/light-programmer/](http://jumptuck.com/category/projects/light-programmer/)

# C

- To get SimpleOS to work:
- - $ sudo apt-get install lib32ncurses5 lib32z1
- - $ cd simple_os_2013_03_05/simple_os_apps/barber/gui
- - $ ./prog_x86_host
- - $ cd simple_os_2013_03_05/java_gui/java
- - $ java GUISimpleOS
 
## Assignment 1:
- Compile and execute the program set_clock:
- - $ cd ~/c/assignment1
- - $ make set_clock
- Run set_clock:
- - $ cd simple_os_2013_03_05/java_gui/java
- - $ java GUISimpleOS
- - $ ./set_clock (in ~/c/assignment1)
- Compile and execute the program odd_even:
- - $ cd ~/c/assignment1
- - $ make odd_even
- Run odd_even:
- - $ cd simple_os_2013_03_05/java_gui/java
- - $ java GUISimpleOS
- - $ ./odd_even (in ~/c/assignment1)

- - With the original code, the error message is displayed almost every time
- - After inserting the long_calculation() call, the error message is displayed every single time
- - In increment_number(), does long_calculation() really have to be called where it's currently called? Feels unnecessary to lock the common variables for such a long time. 

Sonix OS v1.4 - Complete Command List
System & Info:

help - shows all commands
clear - clear screen
version - show Sonix version
fastfetch - show Sonix logo, fox, RAM, compiler info, clipboard, net card
netinfo - show e1000 card + MAC (real network)
reboot - reboot VM
poweroff or shutdown - power off
File System:

ls - list files. Colors: .c=blue, .py=yellow, .java=red, .son=green
cat <file> - read file. Example: cat readme.txt, cat demo.c
touch <file> - create empty file. Example: touch myfile.txt
rm <file> - delete file. Example: rm myfile.txt
edit <file> or vim <file> - open colorful VIM editor
Inside VIM:

Type anything - you are always in INSERT mode
: then w + ENTER = Save
: then q + ENTER = Quit without save
: then wq + ENTER = Save & Quit (use this)
Ctrl+C = Copy whole file to clipboard
Ctrl+V = Paste clipboard
Apps & Compilers:

apps - list all installed .son apps
run <file.son> or ./<file.son> - run app. Example: ./hello.son
cc <file.c> - C compiler inside Sonix! Example: cc demo.c -> creates demo.son then ./demo.son
python <file.py> - Python interpreter. Example: python demo.py
java <file.java> or javac <file.java> - Java compiler. Example: java Demo.java
Network (real e1000 driver):

ping <host> - ping. Example: ping 8.8.8.8 or ping google.com
www <url> or internet <url> - browser (simulated HTTP via gateway). Example: www google.com
Requires QEMU: -netdev user,id=n1 -device e1000,netdev=n1 (already in make run)
Installer:

install - opens blue installer screen to install Sonix to hard disk. Same as boot menu option [2]
Clipboard (Everywhere):

Ctrl+C - Copy current input line (in shell) or whole file (in vim)
Ctrl+V - Paste
On Boot Screen:

[1] - Boot Live
[2] - Install to Disk
Example Session:

Code
Sonix> fastfetch
Sonix> ls
Sonix> cat demo.c
Sonix> cc demo.c
Sonix> ./demo.son
Sonix> python demo.py
Sonix> java Demo.java
Sonix> vim myapp.c
  prints My App
  :wq
Sonix> cc myapp.c
Sonix> ./myapp.son
Sonix> install

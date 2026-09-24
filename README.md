A simple ELF binary parser.

Setup:
```
g++ -Wall -Wextra -std=c++17 main.cpp ELFPrint.cpp ELFParse.cpp -o elfinspect
```

Usage:
```
./elfinspect <path_to_elf/filename>
```

Example:
```
./elfinspect /bin/ls
```

Acknowledgements:  
This project is primarily intended as a learning experience for the author to better understand Linux OS mechanics and binary analysis.  
I'd like to credit the book "Learning Linux Binary Analysis" by Ryan "Elfmaster" O'Neill. I highly recommend it to anyone intending to learn about Linux internals.  

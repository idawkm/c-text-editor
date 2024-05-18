# Terminal

### Configuração do terminal

<ul>
<li>STDIN_FILENO - É a descrição de arquivo padrão para entrada de dados.</li>

</ul>

### Flags

<ul>
<li>ECHO - Turn on/off echoing</li>
<li>ICANON - Turn on/off canonical mode (By disabling it we can reading input byte-by-byte instead of line-by-line)</li>
<li>ISIG - SIGINT (Ctrl + C) e SIGTSTP (Ctrl + Z) signals</li>
<li>IXON - Disable/Enable transmission (Ctrl + S produces XOFF to pause transmission) and (Ctrl + Q produces XON to resume transmission) that is used for software flow control</li>
<li>IEXTEN - Disables (Ctrl + V) and also fix (Ctrl + O in macOS)</li>
<li>ICRNL - The terminal helps translate any carriage return (13, '\r') inputted by the user, so (Ctrl + M returns 10), by disabling it when you press (Ctrl + M) it will return 13</li>
<li>BRKINT - A break condition will cause a SIGINT signal to be sent to the program, like pressing Ctrl + C</li>
<li>INPCK - Enables parity checking, which doesn't seem to apply to modern terminal emulators</li>
<li>ISTRIP - causes the 8th bit of each input byte to be stripped, meaning it will set it to 0. This is probably already turned off</li>
<li>CS8 - is not a flag, it is a bit mask with multiple bits, which we set using the bitwise-OR (|) operator unlike all the flags we are turning off. It sets the character size (CS) to 8 bits per byte. On my system, it’s already set that way</li>

</ul>
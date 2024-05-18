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

</ul>
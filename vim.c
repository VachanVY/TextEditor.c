#include <stdio.h> // perror
#include <unistd.h>
#include <termios.h>
#include <stdlib.h> // atexit, exit
#include <ctype.h> // iscntrl
#include <errno.h> // errno

/*                                                          
The CTRL_KEY macro bitwise-ANDs a character with the value 00011111, in binary.
In other words, it sets the upper 3 bits of the character to 0 
(when compared to binary with 8 digits)
*/
#define CTRL_KEY(k) ((k) & 31)
struct termios orig_termios;
void die(const char*);
void disableRawMode();
void enableRawMode();
char editorReadKey();
void editorProcessKeypress();


int main(){
    enableRawMode();
    
    while(1){
        editorProcessKeypress();
    }

    return 0;
}


void die(const char *str){
    /*
    Most C library functions that fail will set the global errno variable to indicate
    what the error was. perror() looks at the global errno variable and prints a 
    descriptive error message for it
    */
    perror(str);
    exit(1);
}


void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}


void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) 
        die("tcgetattr");
    atexit(disableRawMode); // Register a function to be called when `exit' is called.

    struct termios raw = orig_termios;
    /*
    IXON: By default, Ctrl-S and Ctrl-Q are used for software flow control. Ctrl-S
    stops data from being transmitted to the terminal until you press Ctrl-Q. 
    This originates in the days when you might want to pause the transmission 
    of data to let a device like a printer catch up

    ICRNL: If you run the program now and go through the whole alphabet while holding
    down Ctrl, you should see that we have every letter except M. 
    Ctrl-M is weird: it’s being read as 10, when we expect it to be read as 13, since 
    it is the 13th letter of the alphabet, and Ctrl-J already produces a 10. What 
    else produces 10? The Enter key does.
    It turns out that the terminal is helpfully translating any carriage returns
    (13, '\r') inputted by the user into newlines (10, '\n'). Let’s turn off this
    feature.

    Other Flags below are not important
    */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /*
    Terminal translates each newline ("\n") we print into a carriage return followed 
    by a newline ("\r\n"). The terminal requires both of these characters in order 
    to start a new line of text
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    The carriage return moves the cursor back to the beginning of the current line, 
    and the newline moves the cursor down a line, scrolling the screen if necessary
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    OPOST: We will turn off all output processing features by turning off the OPOST
    flag
    */
    raw.c_oflag &= ~(OPOST);

    raw.c_cflag |= (CS8);
    /*
    ECHO: causes each key you type to be printed to the terminal,
    so you can see what you’re typing. This is useful in canonical mode, 
    but really gets in the way when we are trying to carefully render a 
    user interface in raw mode. So we turn it off. This program does the 
    same thing as the one in the previous step, it just doesn’t print what
    you are typing. You may be familiar with this mode if you’ve ever had
    to type a password at the terminal, when using sudo for example.

    ICANON: can be reading input byte-by-byte, instead of line-by-line with.
    With it can exit just by writing q, insted of q then Enter

    ISIG: By default, Ctrl-C sends a SIGINT signal to the current process which 
    causes it to terminate, and Ctrl-Z sends a SIGTSTP signal to the current process
    which causes it to suspend. Let’s turn off the sending of both of these signals

    IEXTEN: On some systems, when you type Ctrl-V, the terminal waits for you to type
    another character and then sends that character literally. For example, before
    we disabled Ctrl-C, you might’ve been able to type Ctrl-V and then Ctrl-C to
    input a 3 byte. We can turn off this feature using the IEXTEN flag. Turning 
    off IEXTEN also fixes Ctrl-O in macOS, whose terminal driver is otherwise 
    set to discard that control character
    */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    // We can set a timeout, so that read() returns if it doesn’t get any input 
    // for a certain amount of time

    /* Uncomment this, repeatedly prints "0\n"
    raw.c_cc[VMIN] = 0; // sets the minimum number of bytes of input needed before read() can return 
    raw.c_cc[VTIME] = 1; // (1/VTIME)seconds // sets the maximum amount of time to wait before read() returns
    */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) 
        die("tcsetattr");
}


char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}


void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
    case CTRL_KEY('q'):
      exit(0); break;
  }
}


Tested on Linux (Bash)
<br>

## Installation & Usage
```
	git clone https://github.com/sleepychloe/ReadLine.git
	cd ReadLine
	make
	./readline
```

<br>


## Class ReadLine

### Introduction

The `ReadLine` class provides a robust mechanism for terminal input handling, leveraging the POSIX termios library.

By leveraging non-canonical mode and handling raw input, it allows for seamless and intuitive user interactions.


Features:

1. Real-time input processing with non-canonical mode
	- processes input character-by-character instead of waiting for the Enter key.
	- disables automatic echoing of typed characters.
2. Handling special keys
	- supports arrow keys, backspace, delete, function keys, and Alt-key combinations.
	- recognizes multi-character escape sequences.
3. Customizing terminal behavior for interactive programs
	- implements manual backspace, delete, and cursor movement logic.
	- allows real-time updates to terminal display.


usage example:

```
	#include "ReadLine.hpp"
	#include <iostream>

	int	main(void)
	{
		std::string	input;
		ReadLine	rl(STDIN_FILENO);

		std::cout << "Type 'exit' to quit ! << std::endl;

		while (1)
		{
			/* Read user input using the ReadLine class */
			if (rl.read_line("> ", input) == -1)
			{
				std::cerr << "Error: unexpected error from ReadLine::read_line()"
					<< std::endl;
				return (1);
			}
			/* check if the input is empty */
			if (input == "" || input.empty())
			{
				std::cout << "the input is empty" << std::endl;
				continue ;
			}
			/* check exit */
			else if (input == "exit" || input == "EXIT")
			{
				std::cout << "Program terminated" << std::endl;
				break ;
			}
			else
			{
				/* display user input */
				std::cout << "input: " << input << std::endl;
				continue ;
			}
		}
		return (0);
	}

```

<br>

### Terminal input handling with termios

`termios` is a POSIX library used to control terminal I/O behavior.

It allows configuring how the terminal processes input and output,

make it possible to:

- enable or disable canonical mode(line-buffered input)
- enable or disable echoing of type characters\
- control other terminal setting

<br>

### Canonical mode and Non-canonical mode

- canonical mode(default)
	+ input is line-buffered
	+ the terminal waits for the user to press `Enter` before processing input
	+ backspace handling and other input editing features are handled by the terminal

- non-canonical mode
	+ input is processed character-by-character
	+ no need to press `Enter` for input to be available
	+ the program handles special keys(ex. backspace, arrow keys) directly

<br>

### Why use non-canonical mode?

- real-time input handling
	+ process key presses immediately without waiting for `Enter`
- custom input behavior
	+ handle special keys like arrow keys, backspace, and Ctrl+C programmatically
- interactive programs
	+ useful for creating applications like games, shells, or text editors

<br>

### How to enable non-canonical mode

```
	int	ReadLine::enable_raw_mode(void)
	{
		struct termios	raw;

		if (tcgetattr(STDIN_FILENO, &this->_original_termios) == -1)
		{
			perror("tcgetattr failed");
			return (-1);
		}

		raw = this->_original_termios;
		raw.c_lflag &= ~(ICANON | ECHO);
		if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1)
		{
			perror("tcsetattr failed");
			return (-1);
		}
		return (0);
	}
```
- `tcgetattr(STDIN_FILENO, &this->_original_termios)`
	+ saves the current terminal settings
- `raw.c_lflag &= ~(ICANON | ECHO)`
	+ `~(ICANON | ECHO)` clears the `ICANON`(canonical mode) and `ECHO`(echo mode) bits
	+ disabling `ICANON` processes input character-by-character
	+ disabling `ECHO` prevents typed characters from being displayed automatically
- `tcsetattr(STDIN_FILENO, TCSANOW, &raw)`
	+ `TCSANOW` option ensures the settings are applied immediately,
	without waiting for data to be sent or received

<br>

### How to disable non-canonical mode

```
	int	ReadLine::disable_raw_mode(void)
	{
		if (tcsetattr(STDIN_FILENO, TCSANOW, &this->_original_termios) == -1)
		{
			perror("tcsetattr failed");
			return (-1);
		}
		return (0);
	}
```
- restores the terminal to its original state, ensuring no unexpected behavior

<br>

### When to use non-canoncial mode

1. always restore terminal setting
	- ensure `disable_raw_mode()` is called when the program exits
	- use `atexit()` or signal handlers(`SIGINT`) to guarantee cleanup:

	```
		#include <cstdlib>
		atexit(disable_raw_mode);
	```
	- if not, terminal may remain in raw mode, causing unusual behavior like:
		+ characters not being echoed when typed
		+ input being processed character-by-character instead of line-by-line
		+ backspace and enter not working as expected
2. delete and backspace handling
	- non-canonial mode doesn't handle backspace automatically
	- need to be processed manually by detecting `127` or `\b`, and removing the last character from the input buffer
	```
		/* removes the character at the cursor position */
		void	ReadLine::handle_delete(std::string &input, size_t &cursor)
		{
			if (!input.empty() && cursor < input.length())
				input.erase(cursor, 1);
			update_display(input, cursor);
		}
	```
	```
		/* removes the character before the cursor */
		void	ReadLine::handle_backspace(std::string &input, size_t &cursor)
		{
			if (!input.empty() && 0 < cursor)
			{
				input.erase(cursor - 1, 1);
				cursor--;
			}
			update_display(input, cursor);
		}
	```
3. escape sequence handling
	- in raw mode, the terminal sends raw input data directly to the program including multi-character escape sequences for special keys
	- without handling these sequences:
		+ special keys will not work
		+ the terminal may behave unpredictably, waiting for sequences to complete or misinterpreting input.
	```
		int	ReadLine::is_escape_sequence(char c)
		{
			char		seq;
			std::string	escape_sequence(1, c);

			std::unordered_set<std::string>	sequence({
				/* arrow keys */
				ESCAPE_UP_ARROW, ESCAPE_DOWN_ARROW, ESCAPE_RIGHT_ARROW, ESCAPE_LEFT_ARROW,

				/* function keys */
				ESCAPE_F1, ESCAPE_F2, ESCAPE_F3, ESCAPE_F4, ESCAPE_F5, ESCAPE_F6,
				ESCAPE_F7, ESCAPE_F8, ESCAPE_F9, ESCAPE_F10, ESCAPE_F11, ESCAPE_F12,

				/* navigation keys */
				ESCAPE_INSERT, ESCAPE_DELETE, ESCAPE_HOME,
				ESCAPE_END, ESCAPE_PAGE_UP, ESCAPE_PAGE_DOWN,

				/* alt keys */
				ESCAPE_ALT_A, ESCAPE_ALT_B, ESCAPE_ALT_C, ESCAPE_ALT_D, ESCAPE_ALT_E,
				ESCAPE_ALT_F, ESCAPE_ALT_G, ESCAPE_ALT_H, ESCAPE_ALT_I, ESCAPE_ALT_J,
				ESCAPE_ALT_K, ESCAPE_ALT_L, ESCAPE_ALT_M, ESCAPE_ALT_N, ESCAPE_ALT_O,
				ESCAPE_ALT_P, ESCAPE_ALT_Q, ESCAPE_ALT_R, ESCAPE_ALT_S, ESCAPE_ALT_T,
				ESCAPE_ALT_U, ESCAPE_ALT_V, ESCAPE_ALT_W, ESCAPE_ALT_X, ESCAPE_ALT_Y, ESCAPE_ALT_Z});

			if (c != '\033')
				return (NONE);

			if (read(this->_fd, &seq, 1) == 1)
			{
				escape_sequence += std::string(1, seq);
				if ('a' <= seq && seq <= 'z')
				{
					/* alt keys */
					if (sequence.find(escape_sequence) != sequence.end())
						return (SEQUENCE_ALT);
				}
				else if (seq == '[' || seq == 'O')
				{
					while (read(this->_fd, &seq, 1) == 1)
					{
						escape_sequence += std::string(1, seq);
						if ('A' <= seq && seq <= 'D')
						{
							/* arrow keys */
							if (sequence.find(escape_sequence) != sequence.end())
							{
								input += escape_sequence;
								return (SEQUENCE_ARROW);
							}
							/* unrecognized sequence */
							break ;
						}
						if (seq == '~' || ('P' <= seq && seq <= 'S')
							|| seq == 'H' || seq == 'F')
						{
							/* navigation key: delete */
							if (escape_sequence == ESCAPE_DELETE)
								return (SEQUENCE_DELETE);
							/* function keys, navigation keys */
							if (sequence.find(escape_sequence) != sequence.end())
								return (SEQUENCE_FUNCTION_NAVIGATION);
							/* unrecognized sequence */
							break ;
						}
						/* unrecognized sequence */
						if (seq == ';' || seq == '/')
							break ;
						
					}
				}
			}
			return (UNRECOGNIZED_SEQUENCE);
		}
	```

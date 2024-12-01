/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadLine.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhwang <yhwang@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 16:40:38 by yhwang            #+#    #+#             */
/*   Updated: 2024/11/30 20:14:59 by yhwang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ReadLine.hpp"

ReadLine::ReadLine(): _fd(0), _prompt(""), _cursor(0), _history_index(0)
{
}

ReadLine::ReadLine(int fd): _fd(fd), _prompt(""), _cursor(0), _history_index(0)
{
	this->_sequence = std::unordered_set<std::string>({
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
}

ReadLine::ReadLine(const ReadLine& readline)
{
	*this = readline;
}

ReadLine&	ReadLine::operator=(const ReadLine& readline)
{
	if (this == &readline)
		return (*this);
	this->_fd = readline._fd;
	this->_prompt = readline._prompt;
	this->_cursor = readline._cursor;
	this->_history = readline._history;
	this->_history_index = readline._history_index;
	this->_sequence = readline._sequence;
	this->_original_termios = readline._original_termios;
	return (*this);
}

ReadLine::~ReadLine()
{
	disable_raw_mode();
}

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
		disable_raw_mode();
		return (-1);
	}
	return (0);
}

int	ReadLine::disable_raw_mode(void)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, &this->_original_termios) == -1)
	{
		perror("tcsetattr failed");
		return (-1);
	}
	return (0);
}

int	ReadLine::is_escape_sequence(std::string &input, char c)
{
	int		i = 5;
	std::string	escape_sequence(1, c);

	if (c != '\033')
		return (NONE);

	while (i > 0)
	{
		if (read(this->_fd, &c, 1) == -1)
		{
			perror("read failed");
			return (-1);
		}
		escape_sequence += std::string(1, c);

		if ('a' <= c && c <= 'z')
		{
			if (this->_sequence.find(escape_sequence) != this->_sequence.end())
				return (SEQUENCE_ALT);
			return (UNRECOGNIZED_SEQUENCE);
		}
		if ('A' <= c && c <= 'D')
		{
			if (this->_sequence.find(escape_sequence) != this->_sequence.end())
			{
				input += escape_sequence;
				return (SEQUENCE_ARROW);
			}
			return (UNRECOGNIZED_SEQUENCE);
		}
		if (c == '~' || ('P' <= c && c <= 'S')
			|| c == 'H' || c == 'F')
		{
			if (escape_sequence == ESCAPE_DELETE)
				return (SEQUENCE_DELETE);
			if (this->_sequence.find(escape_sequence) != this->_sequence.end())
				return (SEQUENCE_FUNCTION_NAVIGATION);
			return (UNRECOGNIZED_SEQUENCE);
		}
		if (c == ';' || c == '/')
			break ;
		i--;
	}
	return (UNRECOGNIZED_SEQUENCE);
}

void	ReadLine::move_cursor(std::string input, std::string arrow)
{
	if (arrow == ESCAPE_LEFT_ARROW)
	{
		if (0 < this->_cursor)
		{
			this->_cursor--;
			std::cout << arrow;
			std::cout.flush();
		}
	}
	else if (arrow == ESCAPE_RIGHT_ARROW)
	{
		if (this->_cursor < input.length())
		{
			this->_cursor++;
			std::cout << arrow;
			std::cout.flush();
		}
	}
}

void	ReadLine::show_history(std::string &input, std::string arrow)
{
	if (arrow == ESCAPE_UP_ARROW)
	{
		if (0 < this->_history_index)
		{
			this->_history_index--;
			input = this->_history[this->_history_index];
			this->_cursor = input.length();
			update_display(input);
		}
	}
	else
	{
		if (this->_history.size() > 0)
		{
			if (this->_history_index < this->_history.size() - 1)
			{
				this->_history_index++;
				input = this->_history[this->_history_index];
				this->_cursor = input.length();
				update_display(input);
			}
			else if (this->_history_index == this->_history.size() - 1)
			{
				input = "";
				this->_cursor = 0;
				update_display(input);
			}
		}
	}
}

void	ReadLine::handle_arrow(std::string &input)
{
	std::string	arrow = input.substr(input.length() - 3);
	input.erase(input.length() - 3);

	if (arrow == ESCAPE_LEFT_ARROW || arrow == ESCAPE_RIGHT_ARROW)
		move_cursor(input, arrow);
	else
		show_history(input, arrow);
}

void	ReadLine::handle_delete(std::string &input)
{
	if (!input.empty() && this->_cursor < input.length())
		input.erase(this->_cursor, 1);
	update_display(input);
}

void	ReadLine::handle_backspace(std::string &input)
{
	if (!input.empty() && 0 < this->_cursor)
	{
		input.erase(this->_cursor - 1, 1);
		this->_cursor--;
	}
	update_display(input);
}

void	ReadLine::update_display(std::string &input)
{
	std::cout << "\033[2K\r"; // clear current line
	std::cout << this->_prompt << input;
	std::cout << "\033[" << (this->_cursor + this->_prompt.length() + 1) << "G"; // move cursor
	std::cout.flush();
}

int	ReadLine::read_line(std::string prompt, std::string &input)
{
	char		c;
	int		type_seq;

	this->_prompt = prompt;
	this->_cursor = 0;
	input.clear();

	if (enable_raw_mode() == -1)
		return (-1);
	
	std::cout << prompt;
	std::cout.flush();
	while (1)
	{
		if (read(this->_fd, &c, 1) == -1)
		{
			perror("read failed");
			disable_raw_mode();
			return (-1);
		}

		type_seq = is_escape_sequence(input, c);
		if (type_seq == -1)
		{
			disable_raw_mode();
			return (-1);
		}
		else if (type_seq)
		{
			if (type_seq == SEQUENCE_ARROW)
				handle_arrow(input);
			if (type_seq == SEQUENCE_DELETE)
				handle_delete(input);
			continue ;
		}

		if (c == KEY_BACKSPACE)
		{
			handle_backspace(input);
			continue ;
		}
		else if (c == '\n')
		{
			if (input != "")
			{
				this->_history.push_back(input);
				this->_history_index = this->_history.size();
			}
			this->_cursor = 0;
			break ;
		}
		else if (!std::isprint(c))
			continue ;

		input.insert(this->_cursor, 1, c);
		this->_cursor++;
		update_display(input);
	}
	if (disable_raw_mode() == -1)
		return (-1);
	std::cout << BLACK << std::endl;
	return (input.length());
}

int	ReadLine::read_line(std::string prompt, std::string color, std::string &input)
{
	std::cout << color;
	return (read_line(prompt, input));
}

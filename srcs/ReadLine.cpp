/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadLine.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhwang <yhwang@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 16:40:38 by yhwang            #+#    #+#             */
/*   Updated: 2024/11/29 20:11:48 by yhwang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ReadLine.hpp"

ReadLine::ReadLine()
{
}

ReadLine::ReadLine(int fd): _fd(fd)
{
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
					if (sequence.find(escape_sequence) != sequence.end())
					{
						input += escape_sequence;
						return (SEQUENCE_ARROW);
					}
					break ;
				}
				if (seq == '~' || ('P' <= seq && seq <= 'S')
					|| seq == 'H' || seq == 'F')
				{
					if (escape_sequence == ESCAPE_DELETE)
						return (SEQUENCE_DELETE);
					if (sequence.find(escape_sequence) != sequence.end())
						return (SEQUENCE_FUNCTION_NAVIGATION);
					break ;
				}
				if (seq == ';' || seq == '/')
					break ;
				
			}
		}
	}
	return (UNRECOGNIZED_SEQUENCE);
}

void	ReadLine::handle_arrow(std::string &input, size_t &cursor)
{
	std::string	arrow = input.substr(input.length() - 3);
	input.erase(input.length() - 3);

	if (arrow == ESCAPE_LEFT_ARROW)
	{
		if (0 < cursor)
		{
			cursor--;
			std::cout << arrow;
		}
	}
	else if (arrow == ESCAPE_RIGHT_ARROW)
	{
		if (cursor < input.length())
		{
			cursor++;
			std::cout << arrow;
		}
	}
	std::cout.flush();
}

void	ReadLine::handle_delete(std::string &input, size_t &cursor)
{
	if (!input.empty() && cursor < input.length())
		input.erase(cursor, 1);
	update_display(input, cursor);
}

void	ReadLine::handle_backspace(std::string &input, size_t &cursor)
{
	if (!input.empty() && 0 < cursor)
	{
		input.erase(cursor - 1, 1);
		cursor--;
	}
	update_display(input, cursor);
}

void	ReadLine::update_display(std::string &input, size_t cursor)
{
	std::cout << "\033[2K\r"; // clear current line
	std::cout << this->_prompt << input;
	std::cout << "\033[" << (cursor + this->_prompt.length() + 1) << "G"; // move cursor
	std::cout.flush();
}

int	ReadLine::read_line(std::string prompt, std::string &input)
{
	char		c;
	int		type_seq;
	size_t		cursor = 0;

	this->_prompt = prompt;
	input.clear();

	if (enable_raw_mode() == -1)
		return (-1);
	
	std::cout << prompt;
	std::cout.flush();
	while (1)
	{
		read(this->_fd, &c, 1);

		type_seq = is_escape_sequence(input, c);
		if (type_seq)
		{
			if (type_seq == SEQUENCE_ARROW)
				handle_arrow(input, cursor);
			if (type_seq == SEQUENCE_DELETE)
				handle_delete(input, cursor);
			continue ;
		}
		else if (c == 127)
		{
			handle_backspace(input, cursor);
			continue ;
		}
		else if (c == '\n')
			break ;
		else if (!std::isprint(c))
			continue ;

		input.insert(cursor, 1, c);
		cursor++;
		update_display(input, cursor);
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

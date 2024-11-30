/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadLine.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhwang <yhwang@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 16:40:49 by yhwang            #+#    #+#             */
/*   Updated: 2024/11/30 19:30:41 by yhwang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READ_LINE_HPP
# define READ_LINE_HPP

#include <iostream>
#include <unistd.h>
#include <csignal>
#include <termios.h>
#include <unordered_set>
#include <algorithm>
#include <cstdio>
#include <vector>
#include "./EscapeSequence.hpp"

# define BLACK					"\x1b[0m"

# define KEY_BACKSPACE				127

# define NONE					0
# define SEQUENCE_ALT				1
# define SEQUENCE_ARROW				2
# define SEQUENCE_DELETE			3
# define SEQUENCE_FUNCTION_NAVIGATION		4
# define UNRECOGNIZED_SEQUENCE			5

class	ReadLine
{
public:
	ReadLine(int fd);
	ReadLine(const ReadLine& readline);
	ReadLine& operator=(const ReadLine& readline);
	~ReadLine();

	int				enable_raw_mode(void);
	int				disable_raw_mode(void);
	int				read_line(std::string prompt, std::string &input);
	int				read_line(std::string prompt, std::string color, std::string &input);

private:
	ReadLine();

	int				is_escape_sequence(std::string &input, char c);
	void				move_cursor(std::string input, std::string arrow);
	void				show_history(std::string &input, std::string arrow);
	void				handle_arrow(std::string &input);
	void				handle_delete(std::string &input);
	void				handle_backspace(std::string &input);
	void				update_display(std::string &input);

	int				_fd;
	std::string			_prompt;
	size_t				_cursor;
	std::vector<std::string>	_history;
	size_t				_history_index;
	std::unordered_set<std::string>	_sequence;
	struct termios			_original_termios;
};

# endif

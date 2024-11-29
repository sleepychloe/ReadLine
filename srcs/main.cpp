/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhwang <yhwang@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/28 10:09:01 by yhwang            #+#    #+#             */
/*   Updated: 2024/11/29 20:36:31 by yhwang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/ReadLine.hpp"
#include "../incs/Color.hpp"

void	ignore_all_signal(void)
{
	sigset_t	mask;

	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);
}

void	print_msg(std::string color, std::string msg)
{
	std::cout << color << msg << BLACK << std::endl;
}

int	main(int argc, char **argv)
{
	(void)argv;
	if (argc != 1)
	{
		std::cerr << RED << "Error: invalid argument" << BLACK << std::endl;
		return (1);
	}

	std::string	input;
	ReadLine	rl(STDIN_FILENO);

	ignore_all_signal();
	print_msg(CYAN, "type 'exit' to quit !\n");
	while (1)
	{
		/* Read user input using the ReadLine class */
		if (rl.read_line("> ", CYAN, input) == -1)
		{
			std::cerr << RED << "Error: unexpected error from ReadLine::read_line()"
				<< BLACK << std::endl;
			return (1);
		}

		/* check if the input is empty */
		if (input == "" || input.empty())
		{
			print_msg(RED, "Warning: the input is empty");
			continue ;
		}
		/* check exit */
		else if (input == "exit" || input == "EXIT")
		{
			print_msg(CYAN, "Program terminated");
			break ;
		}
		/* display user input */
		std::cout << "input: " << input << std::endl;
	}
	return (0);
}

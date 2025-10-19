#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <vector>

std::vector<std::string> split_by_operator(const std::string& input) {
	std::vector<std::string> result;
	std::string current;

	for(int i = 0; i < input.length(); i++) {
		if(i + 1 < input.length()) {
			std::string twochars = input.substr(i, 2);
			if(twochars == "&&" || twochars == "||" || twochars == ">>") {
				if(!current.empty()) {
					result.push_back(current);
					current.clear();
				}
				result.push_back(twochars);
				i++;
				continue;
			}
		}

		if(input[i] == ';' || input[i] == '>' || input[i] == '|' || input[i] == '&') {
			if(!current.empty()) {
				result.push_back(current);
				current.clear();
			}
			result.push_back(std::string(1, input[i]));
		} else {
			current += input[i];
		}
	}

	if(!current.empty()) {
		result.push_back(current);
	}
	
	return result;

}

std::vector<std::string> split_command(const std::string& cmd) {
	std::vector<std::string> args;
	std::string current;

	for(char c : cmd) {
		if(c == ' ') {
			if(!current.empty()) {
				args.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}	
	
	if(!current.empty()) {
		args.push_back(current);
	}
	return args;
}



bool execute_command(const std::string& cmd, bool silent, pid_t main_pid) {
	std::string command = cmd;
	std::string filename;
	int redirect_type = 0;

	int pos;
	if((pos = command.find(">>")) != std::string::npos) {
		redirect_type = 2;
		filename = command.substr(pos + 2);
		command = command.substr(0, pos);
	} else if((pos = command.find(">")) != std::string::npos) {
		redirect_type = 1;
		filename = command.substr(pos + 1);
		command = command.substr(0, pos);
	}

	while(!command.empty() && command.back() == ' ') command.pop_back();
	while(!filename.empty() && filename.front() == ' ') filename = filename.substr(1);
	while(!filename.empty() && filename.back() == ' ') filename.pop_back();

	std::vector<std::string> args_list = split_command(command);
	if(args_list.empty()) return false;

	char** args = new char*[args_list.size() + 1];
	for(int i = 0; i < args_list.size(); i++) {
		args[i] = (char*)args_list[i].c_str();
	}
	args[args_list.size()] = NULL;


	pid_t pid = fork();

	if(pid == 0) {
		
		if(silent) {
			std::string logname = std::to_string(main_pid) + ".log";
			int fd = open(logname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
			
		}
		else if(redirect_type > 0 && !filename.empty()) {
			int flags = O_WRONLY | O_CREAT;
			if(redirect_type == 1) flags |= O_TRUNC;
			else flags |= O_APPEND;

			int fd = open(filename.c_str(), flags, 0644);
			dup2(fd, 1);
			close(fd);
		}

		execvp(args[0], args);

		perror("execvp");
		exit(1);
		
	} else if(pid > 0) {
		int status;
		waitpid(pid, &status, 0);
		return WIFEXITED(status) && WEXITSTATUS(status) == 0;
	}
	return false;
}



int main() {
	
	std::string input;
	while(true) {
		getline(std::cin, input);

		if(input == "exit") {
			break;
		}

		if(input.empty()) {
			continue;
		}

		bool silent = false;
		std::string com = input;

		if(input.find("silent ") == 0) {
			silent == true;
			com = input.substr(7);
		}

		std::vector<std::string> parts = split_by_operator(com);
		if(parts.empty()) continue;


		pid_t main_pid = fork();

		if(main_pid == 0) {
			bool last_result = true;

			for(int i = 0; i < parts.size(); i++) {
				std::string part = parts[i];

				while(!part.empty() && part.front() == ' ') part = part.substr(1);
				while(!part.empty() && part.back() == ' ') part.pop_back();
				if(part.empty()) continue;

				if(part == ";" || part == "&&" || part == "||" || part == ">" || part == ">>") {
					continue;
				}

				bool should_execute = true;
				if(i > 0) {
					std::string prev = parts[i - 1];
					if(prev == "&&") {
						should_execute = last_result;

					} else if(prev == "||") {
						should_execute = !last_result;
					}
				}

				if(should_execute) {
					std::string full_command = part;
					if(i + 1 < parts.size()) {
						std::string next = parts[i + 1];
						if(next == ">" || next == ">>") {
							if(i + 2 < parts.size()) {
								full_command = part + next + parts[i + 2];
								i += 2;
							}
						}
					}

					last_result = execute_command(full_command, silent, getpid());
				
				}
			
			}
			exit(0);
		} else if(main_pid > 0) {
			wait(NULL);
		} else {
			std::cout << "Failed to create process" << std::endl;
		}
	}
	
	return 0;
}	

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <ucontext.h>
#include <cstring>


void signalHandler(int signum, siginfo_t* signal_info, void* context_data) {
	if(signal_info == nullptr || context_data == nullptr) {
		std::cerr << "Error missing info" << std::endl;
		return;
	}

	ucontext_t* context = static_cast<ucontext_t*>(context_data);
	uid_t user_id = signal_info->si_uid;
	struct passwd* user_data = getpwuid(user_id);
	const char* username = user_data->pw_name;
	if(!user_data) {
		username = "unknown";
	}

	std::cout << "Received SIGUSR1 from process [" << signal_info->si_pid << "]" << "run by user [" <<
	       user_id << "]" << "( " << username << ").\n";	


#if defined(__x86_64__)
	auto& reg = context->uc_mcontext.gregs;
	std::cout << "Context state: EIP =  " << reg[REG_RIP] << ", EAX = " << reg[REG_RAX] << 
	", EBX = " << reg[REG_RBX] << "\n\n";


#else
	std::cout << "Cannot get info about regs" << std::endl;

#endif
}

int main() {
	struct sigaction sa = {};
	pid_t pid = getpid();
	if(pid < 0) {
		std::cout << "Could not get the pid" << std::endl;
		return 1;

	}

	sa.sa_sigaction = signalHandler;
	sa.sa_flags = SA_SIGINFO;

	if(sigaction(SIGUSR1, &sa, nullptr) == -1) {
		std::cerr << "Failed sigaction" << std::endl;
		return 1;
	}

	while(true) {
		std::cout << "Work in progress" << std::endl;
		sleep(3);
	}

	return 0;
}



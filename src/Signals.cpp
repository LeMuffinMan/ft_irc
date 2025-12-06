#include <csignal>

sig_atomic_t g_sig;

// TODO: look for theses functions: sigfillset, sigaddset, sigdelset, sigismember

static void signal_handler(int sig, siginfo_t *info, void *context);

/**
 * @brief Initialize signal handling for SIGINT and SIGQUIT
 *
 * @return int
 */
int signal_init(void)
{
	struct sigaction action_receive;

	action_receive.sa_sigaction = signal_handler;
	sigemptyset(&action_receive.sa_mask);
	action_receive.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGINT, &action_receive, NULL);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	return (0);
}

/**
 * @brief Signal handler
 * Ctrl+C = SIGINT -> quits the program.
 * Ctrl+\ = SIGQUIT -> same

 *
 * @param sig Signal received.
 * @param info Information about the signal.
 * @param context Context of the signal.
 */
void signal_handler(int sig, siginfo_t *info, void *context)
{
	(void)info;
	(void)context;
	if (sig == SIGINT)
	{
		g_sig = sig;
		return;
	}
}

/**
 * @brief Function to reset signals to default
 *
 */
void reset_signal_default(void)
{
	struct sigaction action;

	sigemptyset(&action.sa_mask);
	action.sa_handler = SIG_DFL;
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
}

/**
 * @brief Function to ignore certain signals
 *
 */
void ignore_signal(void)
{
	struct sigaction action;

	sigemptyset(&action.sa_mask);
	action.sa_handler = SIG_IGN;
	action.sa_flags = 0;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
}

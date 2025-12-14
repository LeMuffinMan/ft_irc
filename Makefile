.PHONY : all clean fclean re bonus clean-lib clean-bin clean-obj debug debug-CXX debug-print test doc
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
DEPENDANCIES = -MMD -MP
NO_DIR = --no-print-directory
MAKE := $(MAKE) -j $(NO_DIR)
NAME = ircserv

# Debugging flags
CXXFLAGS_DEBUG = -Wall -Wextra -g3 -std=c++98
CXX_DEBUG = clang++
CXX_DEBUG_CXXFLAGS = -std=c++98 -g3 -Weverything -Wno-padded -pedantic -O2 -Wwrite-strings -Wconversion -fsanitize=address -fsanitize=leak -Wno-covered-switch-default -Wno-suggest-override -Wno-suggest-destructor-override
#############################################################################################
#                                                                                           #
#                                         DIRECTORIES                                       #
#                                                                                           #
#############################################################################################
# Source directories
P_SRC = src/
P_CMDS = commands/

P_OBJ = .obj/
P_DEPS = .obj/

P_INC = inc/

#############################################################################################
#                                                                                           #
#                                           FILES                                           #
#                                                                                           #
#############################################################################################
# Headers
INC = \
	Debug.hpp \
	utils.hpp \
	Client.hpp \
	Channel.hpp \
	Server.hpp \
	Reply.hpp \

INC_CMDS = \
	ACommand.hpp \
	CommandFactory.hpp \
	PassCommand.hpp \
	InviteCommand.hpp \
	TopicCommand.hpp \
	JoinCommand.hpp \
	NickCommand.hpp \
	UserCommand.hpp \
	PongCommand.hpp \
	PrivmsgCommand.hpp \
	KickCommand.hpp \
	ModeCommand.hpp \
	PartCommand.hpp \
	QuitCommand.hpp \
	TimeCommand.hpp \

# Template implementation  files
TPP = \
	Debug.tpp \

# Source files
SRC = \
	main.cpp \
	Debug.cpp \
	utils.cpp \
	Server.cpp \
	ServerIRC.cpp \
	Client.cpp \
	Channel.cpp \
	Signals.cpp \

SRC_CMDS = \
	ACommand.cpp \
	CommandFactory.cpp \
	InviteCommand.cpp \
	TopicCommand.cpp \
	JoinCommand.cpp \
	PassCommand.cpp \
	NickCommand.cpp \
	UserCommand.cpp \
	PongCommand.cpp \
	PrivmsgCommand.cpp \
	KickCommand.cpp \
	ModeCommand.cpp \
	PartCommand.cpp \
	QuitCommand.cpp \
	TimeCommand.cpp \

LIBS = \

#############################################################################################
#                                                                                           #
#                                        MANIPULATION                                       #
#                                                                                           #
#############################################################################################
SRCS = \
	$(addprefix $(P_SRC), $(SRC)) \
	$(addprefix $(P_SRC)$(P_CMDS), $(SRC_CMDS)) \

# List of object files (redirect to P_OBJ)
OBJS = $(subst $(P_SRC), $(P_OBJ), $(SRCS:.cpp=.o))
P_OBJS = $(subst $(P_SRC), $(P_OBJ), $(SRCS))

# List of depedencies
DEPS = $(subst $(P_OBJ), $(P_DEPS), $(OBJS:%.o=%.d))

# List of header files
INCS = \
	$(addprefix $(P_INC), $(INC)) \
	$(addprefix $(P_INC)$(P_CMDS), $(INC_CMDS)) \

# 	$(addprefix $(P_INC), $(TPP))

#############################################################################################
#                                                                                           #
#                                          RULES                                            #
#                                                                                           #
#############################################################################################
all: 
	@$(MAKE) $(NAME)
# 	@$(MAKE) client

# Create $(NAME) executable
$(NAME): $(OBJS) $(INCS)
	@if $(CXX) $(CXXFLAGS) $(DEPENDANCIES) -I $(P_INC) -I $(P_INC)$(P_CMDS) -o $(NAME) $(OBJS) $(LIBS); then \
		echo "$(Green)Creating executable $@$(Color_Off)"; \
	else \
		echo "$(Red)Error creating $@$(Color_Off)"; \
	fi

# Custom rule to compilate all .cpp with there path
$(P_OBJ)%.o: $(P_SRC)%.cpp $(INCS)
	@mkdir -p $(dir $@)
	@if $(CXX) $(CXXFLAGS) $(DEPENDANCIES) -I $(P_INC) -I $(P_INC)$(P_CMDS) -c $< -o $@; then \
		echo "$(Cyan)Compiling $<$(Color_Off)"; \
	else \
		echo "$(Red)Error creating $@$(Color_Off)"; \
	fi

# client:
# 	$(CXX) $(CXXFLAGS) src/client.cpp -I inc -o $@

#############################################################################################
#                                                                                           #
#                                      Other RULES                                          #
#                                                                                           #
#############################################################################################
# Rules for clean up
clean:
	rm -rfd $(P_OBJ)
	rm -rfd $(P_DEPS)
	rm -rf .server_output.log
# 	rm -rfd $(OBJS)
# 	rm -rfd $(DEPS)

clean-lib:
	rm -rfd $(P_LIB)

clean-bin:
	rm -f $(NAME)

clean-obj:
	@$(MAKE) clean

fclean:
	@$(MAKE) clean-obj
	@$(MAKE) clean-bin
# 	@$(MAKE) clean-lib

re:
	@$(MAKE) fclean
	@$(MAKE) all

CLIENTS ?= 1000
LEAKS ?= 0
STRESS ?= 0
LOG ?= 0
BEH ?= 0
#PORT?=6667

test:
ifeq ($(LEAKS),1)
	$(MAKE) fclean
	@$(MAKE) $(NAME) CXXFLAGS="$(CXXFLAGS_DEBUG) -D USE_TESTER=1"
else
	@$(MAKE) $(NAME)
endif
	./tests/run.sh $(CLIENTS) $(LEAKS) $(STRESS) $(LOG) $(BEH)

doc: 
	$(MAKE) doc-clean
	cd tests/irc_tester && cargo doc --no-deps --target-dir ../../docs/irc_tester
	cd bonus/bot && cargo doc --no-deps --target-dir ../../docs/bot

doc-full:
	$(MAKE) doc-clean
	cd tests/irc_tester && cargo doc --target-dir ../../docs/irc_tester
	cd bonus/bot && cargo doc --target-dir ../../docs/bot

doc-clean:
	rm -rf docs/irc_tester
	rm -rf docs/bot

# Aliases
clear: clean
fclear: fclean
fclena: fclean
flcean: fclean
flcear: fclean

#############################################################################################
#                                                                                           #
#                                           DEBUG                                           #
#                                                                                           #
#############################################################################################
# Debugging rules
debug:
	@$(MAKE) $(NAME) CXXFLAGS="$(CXXFLAGS_DEBUG)"

debug-cxx:
	@$(MAKE) $(NAME) CXXFLAGS="$(CXX_DEBUG_CXXFLAGS)" CXX="$(CXX_DEBUG)" Cyan="$(Yellow)" Green="$(Purple)"

# Debugging print
debug-print:
	@$(MAKE) debug-print-project

debug-print-separator:
	@echo ""
	@echo "$(On_Yellow)____________________$(Color_Off)"
	@echo ""

define check_var
	@if [ "$(strip $(1))" = "" ]; then \
		echo "$(Red)No $(2) found$(Color_Off)"; \
	else \
		echo "$(Red)$(2): \n\t$(Blue)$(1)$(Color_Off)"; \
	fi
endef

debug-print-project:
	$(call check_var,$(NAME),"Project")
	$(call check_var,$(SRCS),"SRCS")
	$(call check_var,$(OBJS),"OBJS")
	$(call check_var,$(INCS),"INCS")
	$(call check_var,$(LIBS),"LIBS")

#############################################################################################
#                                                                                           #
#                                         COSMETIC                                          #
#                                                                                           #
#############################################################################################
# Reset
Color_Off=\033[0m       # Text Reset

# Regular Colors
Black=\033[0;30m
Red=\033[0;31m
Green=\033[0;32m
Yellow=\033[0;33m
Blue=\033[0;34m
Purple=\033[0;35m
Cyan=\033[0;36m
White=\033[0;37m

# Background
On_Black=\033[40m
On_Red=\033[41m
On_Green=\033[42m
On_Yellow=\033[43m
On_Blue=\033[44m
On_Purple=\033[45m
On_Cyan=\033[46m
On_White=\033[47m

-include $(DEPS)% 

.PHONY: all clean clean-lib clean-bin clean-obj fclean re test debug debug-print-project debug-print debug-print-separator debug-cxx 

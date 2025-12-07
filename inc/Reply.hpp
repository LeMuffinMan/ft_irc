#ifndef REPLY_HPP
#define REPLY_HPP

#include <string>

/*
============================================================================
Command Responses (001-399)
============================================================================
*/

#define RPL_WELCOME(nick, user, host) "001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host
#define RPL_YOURHOST(nick, servername, version) "002 " + nick + " :Your host is " + servername + ", running version " + version
#define RPL_CREATED(nick, date) "003 " + nick + " :This server was created " + date
#define RPL_MYINFO(nick, servername, version, usermodes, chanmodes) "004 " + nick + " " + servername + " " + version + " " + usermodes + " " + chanmodes
#define RPL_BOUNCE(nick, server, port) "005 " + nick + " :Try server " + server + ", port " + port

#define RPL_USERHOST(nick, reply) "302 " + nick + " :" + reply
#define RPL_ISON(nick, nicks) "303 " + nick + " :" + nicks

#define RPL_AWAY(nick, away_nick, message) "301 " + nick + " " + away_nick + " :" + message
#define RPL_UNAWAY(nick) "305 " + nick + " :You are no longer marked as being away"
#define RPL_NOWAWAY(nick) "306 " + nick + " :You have been marked as being away"

#define RPL_WHOISUSER(nick, target, user, host, realname) "311 " + nick + " " + target + " " + user + " " + host + " * :" + realname
#define RPL_WHOISSERVER(nick, target, server, serverinfo) "312 " + nick + " " + target + " " + server + " :" + serverinfo
#define RPL_WHOISOPERATOR(nick, target) "313 " + nick + " " + target + " :is an IRC operator"
#define RPL_WHOISIDLE(nick, target, idle) "317 " + nick + " " + target + " " + idle + " :seconds idle"
#define RPL_ENDOFWHOIS(nick, target) "318 " + nick + " " + target + " :End of WHOIS list"
#define RPL_WHOISCHANNELS(nick, target, channels) "319 " + nick + " " + target + " :" + channels

#define RPL_WHOWASUSER(nick, target, user, host, realname) "314 " + nick + " " + target + " " + user + " " + host + " * :" + realname
#define RPL_ENDOFWHOWAS(nick, target) "369 " + nick + " " + target + " :End of WHOWAS"

#define RPL_LIST(nick, channel, visible, topic) "322 " + nick + " " + channel + " " + visible + " :" + topic
#define RPL_LISTEND(nick) "323 " + nick + " :End of LIST"

#define RPL_UNIQOPIS(nick, channel, nickname) "325 " + nick + " " + channel + " " + nickname
#define RPL_CHANNELMODEIS(nick, channel, mode, params) "324 " + nick + " " + channel + " " + mode + " " + params

#define RPL_NOTOPIC(nick, channel) "331 " + nick + " " + channel + " :No topic is set"
#define RPL_TOPIC(nick, channel, topic) "332 " + nick + " " + channel + " :" + topic

#define RPL_INVITING(nick, channel, target) "341 " + nick + " " + channel + " " + target
#define RPL_SUMMONING(nick, user) "342 " + nick + " " + user + " :Summoning user to IRC"

#define RPL_INVITELIST(nick, channel, invitemask) "346 " + nick + " " + channel + " " + invitemask
#define RPL_ENDOFINVITELIST(nick, channel) "347 " + nick + " " + channel + " :End of channel invite list"

#define RPL_EXCEPTLIST(nick, channel, exceptionmask) "348 " + nick + " " + channel + " " + exceptionmask
#define RPL_ENDOFEXCEPTLIST(nick, channel) "349 " + nick + " " + channel + " :End of channel exception list"

#define RPL_VERSION(nick, version, debuglevel, server, comments) "351 " + nick + " " + version + "." + debuglevel + " " + server + " :" + comments

#define RPL_WHOREPLY(nick, channel, user, host, server, target, flags, hopcount, realname) "352 " + nick + " " + channel + " " + user + " " + host + " " + server + " " + target + " " + flags + " :" + hopcount + " " + realname
#define RPL_ENDOFWHO(nick, name) "315 " + nick + " " + name + " :End of WHO list"

#define RPL_NAMREPLY(nick, symbol, channel, names) "353 " + nick + " " + symbol + " " + channel + " :" + names
#define RPL_ENDOFNAMES(nick, channel) "366 " + nick + " " + channel + " :End of NAMES list"

#define RPL_LINKS(nick, mask, server, hopcount, serverinfo) "364 " + nick + " " + mask + " " + server + " :" + hopcount + " " + serverinfo
#define RPL_ENDOFLINKS(nick, mask) "365 " + nick + " " + mask + " :End of LINKS list"

#define RPL_BANLIST(nick, channel, banmask) "367 " + nick + " " + channel + " " + banmask
#define RPL_ENDOFBANLIST(nick, channel) "368 " + nick + " " + channel + " :End of channel ban list"

#define RPL_INFO(nick, info) "371 " + nick + " :" + info
#define RPL_ENDOFINFO(nick) "374 " + nick + " :End of INFO list"

#define RPL_MOTDSTART(nick, server) "375 " + nick + " :- " + server + " Message of the day - "
#define RPL_MOTD(nick, text) "372 " + nick + " :- " + text
#define RPL_ENDOFMOTD(nick) "376 " + nick + " :End of MOTD command"

#define RPL_YOUREOPER(nick) "381 " + nick + " :You are now an IRC operator"
#define RPL_REHASHING(nick, configfile) "382 " + nick + " " + configfile + " :Rehashing"
#define RPL_YOURESERVICE(nick, servicename) "383 " + nick + " :You are service " + servicename

#define RPL_TIME(nick, server, time) "391 " + nick + " " + server + " :" + time

#define RPL_USERSSTART(nick) "392 " + nick + " :UserID   Terminal  Host"
#define RPL_USERS(nick, username, ttyline, hostname) "393 " + nick + " :" + username + " " + ttyline + " " + hostname
#define RPL_ENDOFUSERS(nick) "394 " + nick + " :End of users"
#define RPL_NOUSERS(nick) "395 " + nick + " :Nobody logged in"

#define RPL_TRACELINK(nick, version, destination, nextserver, protocol, uptime, backstream, upstream) "200 " + nick + " Link " + version + " " + destination + " " + nextserver + " V" + protocol + " " + uptime + " " + backstream + " " + upstream
#define RPL_TRACECONNECTING(nick, cls, server) "201 " + nick + " Try. " + cls + " " + server
#define RPL_TRACEHANDSHAKE(nick, cls, server) "202 " + nick + " H.S. " + cls + " " + server
#define RPL_TRACEUNKNOWN(nick, cls, ip) "203 " + nick + " ???? " + cls + " " + ip
#define RPL_TRACEOPERATOR(nick, cls, target) "204 " + nick + " Oper " + cls + " " + target
#define RPL_TRACEUSER(nick, cls, target) "205 " + nick + " User " + cls + " " + target
#define RPL_TRACESERVER(nick, cls, intS, intC, server, mask, protocol) "206 " + nick + " Serv " + cls + " " + intS + "S " + intC + "C " + server + " " + mask + " V" + protocol
#define RPL_TRACESERVICE(nick, cls, name, type, activetype) "207 " + nick + " Service " + cls + " " + name + " " + type + " " + activetype
#define RPL_TRACENEWTYPE(nick, newtype, clientname) "208 " + nick + " " + newtype + " 0 " + clientname
#define RPL_TRACECLASS(nick, cls, count) "209 " + nick + " Class " + cls + " " + count
#define RPL_TRACELOG(nick, logfile, debuglevel) "261 " + nick + " File " + logfile + " " + debuglevel
#define RPL_TRACEEND(nick, server, version) "262 " + nick + " " + server + " " + version + " :End of TRACE"

#define RPL_STATSLINKINFO(nick, linkname, sendq, sentmsg, sentkb, recvmsg, recvkb, timeopen) "211 " + nick + " " + linkname + " " + sendq + " " + sentmsg + " " + sentkb + " " + recvmsg + " " + recvkb + " " + timeopen
#define RPL_STATSCOMMANDS(nick, command, count, bytecount, remotecount) "212 " + nick + " " + command + " " + count + " " + bytecount + " " + remotecount
#define RPL_ENDOFSTATS(nick, letter) "219 " + nick + " " + letter + " :End of STATS report"
#define RPL_STATSUPTIME(nick, uptime) "242 " + nick + " :Server Up " + uptime
#define RPL_STATSOLINE(nick, hostmask, name) "243 " + nick + " O " + hostmask + " * " + name

#define RPL_UMODEIS(nick, modes) "221 " + nick + " " + modes

#define RPL_SERVLIST(nick, name, server, mask, type, hopcount, info) "234 " + nick + " " + name + " " + server + " " + mask + " " + type + " " + hopcount + " " + info
#define RPL_SERVLISTEND(nick, mask, type) "235 " + nick + " " + mask + " " + type + " :End of service listing"

#define RPL_LUSERCLIENT(nick, users, services, servers) "251 " + nick + " :There are " + users + " users and " + services + " services on " + servers + " servers"
#define RPL_LUSEROP(nick, ops) "252 " + nick + " " + ops + " :operator(s) online"
#define RPL_LUSERUNKNOWN(nick, unknown) "253 " + nick + " " + unknown + " :unknown connection(s)"
#define RPL_LUSERCHANNELS(nick, channels) "254 " + nick + " " + channels + " :channels formed"
#define RPL_LUSERME(nick, clients, servers) "255 " + nick + " :I have " + clients + " clients and " + servers + " servers"

#define RPL_ADMINME(nick, server) "256 " + nick + " " + server + " :Administrative info"
#define RPL_ADMINLOC1(nick, info) "257 " + nick + " :" + info
#define RPL_ADMINLOC2(nick, info) "258 " + nick + " :" + info
#define RPL_ADMINEMAIL(nick, email) "259 " + nick + " :" + email

#define RPL_TRYAGAIN(nick, command) "263 " + nick + " " + command + " :Please wait a while and try again."

/*
============================================================================
Error Replies (400-599)
============================================================================
*/

#define ERR_NOSUCHNICK(nick, target) "401 " + nick + " " + target + " :No such nick/channel"
#define ERR_NOSUCHSERVER(nick, server) "402 " + nick + " " + server + " :No such server"
#define ERR_NOSUCHCHANNEL(nick, channel) "403 " + nick + " " + channel + " :No such channel"
#define ERR_CANNOTSENDTOCHAN(nick, channel) "404 " + nick + " " + channel + " :Cannot send to channel"
#define ERR_TOOMANYCHANNELS(nick, channel) "405 " + nick + " " + channel + " :You have joined too many channels"
#define ERR_WASNOSUCHNICK(nick, target) "406 " + nick + " " + target + " :There was no such nickname"
#define ERR_TOOMANYTARGETS(nick, target, errorcode) "407 " + nick + " " + target + " :" + errorcode + " recipients."
#define ERR_NOSUCHSERVICE(nick, service) "408 " + nick + " " + service + " :No such service"
#define ERR_NOORIGIN(nick) "409 " + nick + " :No origin specified"

#define ERR_NORECIPIENT(nick, command) "411 " + nick + " :No recipient given (" + command + ")"
#define ERR_NOTEXTTOSEND(nick) "412 " + nick + " :No text to send"
#define ERR_NOTOPLEVEL(nick, mask) "413 " + nick + " " + mask + " :No toplevel domain specified"
#define ERR_WILDTOPLEVEL(nick, mask) "414 " + nick + " " + mask + " :Wildcard in toplevel domain"
#define ERR_BADMASK(nick, mask) "415 " + nick + " " + mask + " :Bad Server/host mask"

#define ERR_UNKNOWNCOMMAND(nick, command) "421 " + nick + " " + command + " :Unknown command"
#define ERR_NOMOTD(nick) "422 " + nick + " :MOTD File is missing"
#define ERR_NOADMININFO(nick, server) "423 " + nick + " " + server + " :No administrative info available"
#define ERR_FILEERROR(nick, fileop, file) "424 " + nick + " :File error doing " + fileop + " on " + file

#define ERR_NONICKNAMEGIVEN(nick) "431 " + nick + " :No nickname given"
#define ERR_ERRONEUSNICKNAME(nick, badnick) "432 " + nick + " " + badnick + " :Erroneous nickname"
#define ERR_NICKNAMEINUSE(nick, badnick) "433 " + nick + " " + badnick + " :Nickname is already in use"
#define ERR_NICKCOLLISION(nick, badnick, user, host) "436 " + nick + " " + badnick + " :Nickname collision KILL from " + user + "@" + host
#define ERR_UNAVAILRESOURCE(nick, resource) "437 " + nick + " " + resource + " :Nick/channel is temporarily unavailable"

#define ERR_USERNOTINCHANNEL(nick, target, channel) "441 " + nick + " " + target + " " + channel + " :They aren't on that channel"
#define ERR_NOTONCHANNEL(nick, channel) "442 " + nick + " " + channel + " :You're not on that channel"
#define ERR_USERONCHANNEL(nick, user, channel) "443 " + nick + " " + user + " " + channel + " :is already on channel"
#define ERR_NOLOGIN(nick, user) "444 " + nick + " " + user + " :User not logged in"
#define ERR_SUMMONDISABLED(nick) "445 " + nick + " :SUMMON has been disabled"
#define ERR_USERSDISABLED(nick) "446 " + nick + " :USERS has been disabled"

#define ERR_NOTREGISTERED(nick) "451 " + nick + " :You have not registered"

#define ERR_NEEDMOREPARAMS(nick, command) "461 " + nick + " " + command + " :Not enough parameters"
#define ERR_ALREADYREGISTRED(nick) "462 " + nick + " :Unauthorized command (already registered)"
#define ERR_NOPERMFORHOST(nick) "463 " + nick + " :Your host isn't among the privileged"
#define ERR_PASSWDMISMATCH(nick) "464 " + nick + " :Password incorrect"
#define ERR_YOUREBANNEDCREEP(nick) "465 " + nick + " :You are banned from this server"
#define ERR_YOUWILLBEBANNED(nick) "466 " + nick + " :You will be banned"

#define ERR_KEYSET(nick, channel) "467 " + nick + " " + channel + " :Channel key already set"
#define ERR_CHANNELISFULL(nick, channel) "471 " + nick + " " + channel + " :Cannot join channel (+l)"
#define ERR_UNKNOWNMODE(nick, c, channel) "472 " + nick + " " + c + " :is unknown mode char to me for " + channel
#define ERR_INVITEONLYCHAN(nick, channel) "473 " + nick + " " + channel + " :Cannot join channel (+i)"
#define ERR_BANNEDFROMCHAN(nick, channel) "474 " + nick + " " + channel + " :Cannot join channel (+b)"
#define ERR_BADCHANNELKEY(nick, channel) "475 " + nick + " " + channel + " :Cannot join channel (+k)"
#define ERR_BADCHANMASK(nick, channel) "476 " + nick + " " + channel + " :Bad Channel Mask"
#define ERR_NOCHANMODES(nick, channel) "477 " + nick + " " + channel + " :Channel doesn't support modes"
#define ERR_BANLISTFULL(nick, channel, c) "478 " + nick + " " + channel + " " + c + " :Channel list is full"

#define ERR_NOPRIVILEGES(nick) "481 " + nick + " :Permission Denied- You're not an IRC operator"
#define ERR_CHANOPRIVSNEEDED(nick, channel) "482 " + nick + " " + channel + " :You're not channel operator"
#define ERR_CANTKILLSERVER(nick) "483 " + nick + " :You can't kill a server!"
#define ERR_RESTRICTED(nick) "484 " + nick + " :Your connection is restricted!"
#define ERR_UNIQOPPRIVSNEEDED(nick) "485 " + nick + " :You're not the original channel operator"

#define ERR_NOOPERHOST(nick) "491 " + nick + " :No O-lines for your host"

#define ERR_UMODEUNKNOWNFLAG(nick) "501 " + nick + " :Unknown MODE flag"
#define ERR_USERSDONTMATCH(nick) "502 " + nick + " :Cannot change mode for other users"

#endif // REPLY_HPP

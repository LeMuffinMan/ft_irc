#ifndef REPLY_HPP
#define REPLY_HPP

#include <string>

// Utiliser const char* au lieu de std::string pour éviter les constructeurs globaux
#define STAR "*"
#define SERVER_NAME "ft_irc"

// Fonction utilitaire pour gérer les nicknames vides
inline const std::string safe_nick(const std::string& nick)
{
    return nick.empty() ? STAR : nick;
}

/*
============================================================================
Command Responses (001-399)
============================================================================
*/

// Welcome messages (001-005)
inline std::string RPL_WELCOME(const std::string& nick, const std::string& host)
{
    return ":" + std::string(SERVER_NAME) + " 001 " + safe_nick(nick) + " :Welcome to the Internet Relay Network " + host;
}

inline std::string RPL_YOURHOST(const std::string& nick, const std::string& servername, const std::string& version)
{
    return ":" + std::string(SERVER_NAME) + " 002 " + safe_nick(nick) + " :Your host is " + servername + ", running version " + version;
}

inline std::string RPL_CREATED(const std::string& nick, const std::string& date)
{
    return ":" + std::string(SERVER_NAME) + " 003 " + safe_nick(nick) + " :This server was created " + date;
}

inline std::string RPL_MYINFO(const std::string& nick, const std::string& servername, const std::string& version, 
                              const std::string& usermodes, const std::string& chanmodes)
{
    return ":" + std::string(SERVER_NAME) + " 004 " + safe_nick(nick) + " " + servername + " " + version + " " + usermodes + " " + chanmodes;
}

inline std::string RPL_BOUNCE(const std::string& nick, const std::string& server, const std::string& port)
{
    return ":" + std::string(SERVER_NAME) + " 005 " + safe_nick(nick) + " :Try server " + server + ", port " + port;
}

// User host and ison (302-303)
inline std::string RPL_USERHOST(const std::string& nick, const std::string& reply)
{
    return ":" + std::string(SERVER_NAME) + " 302 " + safe_nick(nick) + " :" + reply;
}

inline std::string RPL_ISON(const std::string& nick, const std::string& nicks)
{
    return ":" + std::string(SERVER_NAME) + " 303 " + safe_nick(nick) + " :" + nicks;
}

// Away messages (301, 305-306)
inline std::string RPL_AWAY(const std::string& nick, const std::string& away_nick, const std::string& message)
{
    return ":" + std::string(SERVER_NAME) + " 301 " + safe_nick(nick) + " " + away_nick + " :" + message;
}

inline std::string RPL_UNAWAY(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 305 " + safe_nick(nick) + " :You are no longer marked as being away";
}

inline std::string RPL_NOWAWAY(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 306 " + safe_nick(nick) + " :You have been marked as being away";
}

// WHOIS replies (311-319)
inline std::string RPL_WHOISUSER(const std::string& nick, const std::string& target, const std::string& user, 
                                 const std::string& host, const std::string& realname)
{
    return ":" + std::string(SERVER_NAME) + " 311 " + safe_nick(nick) + " " + target + " " + user + " " + host + " * :" + realname;
}

inline std::string RPL_WHOISSERVER(const std::string& nick, const std::string& target, const std::string& server, 
                                   const std::string& serverinfo)
{
    return ":" + std::string(SERVER_NAME) + " 312 " + safe_nick(nick) + " " + target + " " + server + " :" + serverinfo;
}

inline std::string RPL_WHOISOPERATOR(const std::string& nick, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 313 " + safe_nick(nick) + " " + target + " :is an IRC operator";
}

inline std::string RPL_WHOISIDLE(const std::string& nick, const std::string& target, const std::string& idle)
{
    return ":" + std::string(SERVER_NAME) + " 317 " + safe_nick(nick) + " " + target + " " + idle + " :seconds idle";
}

inline std::string RPL_ENDOFWHOIS(const std::string& nick, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 318 " + safe_nick(nick) + " " + target + " :End of WHOIS list";
}

inline std::string RPL_WHOISCHANNELS(const std::string& nick, const std::string& target, const std::string& channels)
{
    return ":" + std::string(SERVER_NAME) + " 319 " + safe_nick(nick) + " " + target + " :" + channels;
}

// WHOWAS replies (314, 369)
inline std::string RPL_WHOWASUSER(const std::string& nick, const std::string& target, const std::string& user, 
                                  const std::string& host, const std::string& realname)
{
    return ":" + std::string(SERVER_NAME) + " 314 " + safe_nick(nick) + " " + target + " " + user + " " + host + " * :" + realname;
}

inline std::string RPL_ENDOFWHOWAS(const std::string& nick, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 369 " + safe_nick(nick) + " " + target + " :End of WHOWAS";
}

// LIST replies (322-323)
inline std::string RPL_LIST(const std::string& nick, const std::string& channel, const std::string& visible, 
                            const std::string& topic)
{
    return ":" + std::string(SERVER_NAME) + " 322 " + safe_nick(nick) + " " + channel + " " + visible + " :" + topic;
}

inline std::string RPL_LISTEND(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 323 " + safe_nick(nick) + " :End of LIST";
}

// Channel mode replies (324-325)
inline std::string RPL_UNIQOPIS(const std::string& nick, const std::string& channel, const std::string& nickname)
{
    return ":" + std::string(SERVER_NAME) + " 325 " + safe_nick(nick) + " " + channel + " " + nickname;
}

inline std::string RPL_CHANNELMODEIS(const std::string& nick, const std::string& channel, const std::string& mode, 
                                     const std::string& params)
{
    return ":" + std::string(SERVER_NAME) + " 324 " + safe_nick(nick) + " " + channel + " " + mode + " " + params;
}

// Topic replies (331-332)
inline std::string RPL_NOTOPIC(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 331 " + safe_nick(nick) + " " + channel + " :No topic is set";
}

inline std::string RPL_TOPIC(const std::string& nick, const std::string& channel, const std::string& topic)
{
    return ":" + std::string(SERVER_NAME) + " 332 " + safe_nick(nick) + " " + channel + " :" + topic;
}

// Invite replies (341-342)
inline std::string RPL_INVITING(const std::string& nick, const std::string& channel, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 341 " + safe_nick(nick) + " " + channel + " " + target;
}

inline std::string RPL_INVITEACCEPTED(const std::string& host, const std::string& channel, const std::string& target)
{
    return ":" + host + " INVITE " + target + " :" + channel;
}

inline std::string RPL_SUMMONING(const std::string& nick, const std::string& user)
{
    return ":" + std::string(SERVER_NAME) + " 342 " + safe_nick(nick) + " " + user + " :Summoning user to IRC";
}

// Invite list (346-347)
inline std::string RPL_INVITELIST(const std::string& nick, const std::string& channel, const std::string& invitemask)
{
    return ":" + std::string(SERVER_NAME) + " 346 " + safe_nick(nick) + " " + channel + " " + invitemask;
}

inline std::string RPL_ENDOFINVITELIST(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 347 " + safe_nick(nick) + " " + channel + " :End of channel invite list";
}

// Exception list (348-349)
inline std::string RPL_EXCEPTLIST(const std::string& nick, const std::string& channel, const std::string& exceptionmask)
{
    return ":" + std::string(SERVER_NAME) + " 348 " + safe_nick(nick) + " " + channel + " " + exceptionmask;
}

inline std::string RPL_ENDOFEXCEPTLIST(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 349 " + safe_nick(nick) + " " + channel + " :End of channel exception list";
}

// Version reply (351)
inline std::string RPL_VERSION(const std::string& nick, const std::string& version, const std::string& debuglevel, 
                               const std::string& server, const std::string& comments)
{
    return ":" + std::string(SERVER_NAME) + " 351 " + safe_nick(nick) + " " + version + "." + debuglevel + " " + server + " :" + comments;
}

// WHO replies (315, 352)
inline std::string RPL_WHOREPLY(const std::string& nick, const std::string& channel, const std::string& user, 
                                const std::string& host, const std::string& server, const std::string& target, 
                                const std::string& flags, const std::string& hopcount, const std::string& realname)
{
    return ":" + std::string(SERVER_NAME) + " 352 " + safe_nick(nick) + " " + channel + " " + user + " " + host + " " + server + " " + target + " " + flags + " :" + hopcount + " " + realname;
}

inline std::string RPL_ENDOFWHO(const std::string& nick, const std::string& name)
{
    return ":" + std::string(SERVER_NAME) + " 315 " + safe_nick(nick) + " " + name + " :End of WHO list";
}

// NAMES replies (353, 366)
inline std::string RPL_NAMREPLY(const std::string& nick, const std::string& symbol, const std::string& channel, 
                                const std::string& names)
{
    return ":" + std::string(SERVER_NAME) + " 353 " + safe_nick(nick) + " " + symbol + " " + channel + " :" + names;
}

inline std::string RPL_ENDOFNAMES(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 366 " + safe_nick(nick) + " " + channel + " :End of NAMES list";
}

// LINKS replies (364-365)
inline std::string RPL_LINKS(const std::string& nick, const std::string& mask, const std::string& server, 
                             const std::string& hopcount, const std::string& serverinfo)
{
    return ":" + std::string(SERVER_NAME) + " 364 " + safe_nick(nick) + " " + mask + " " + server + " :" + hopcount + " " + serverinfo;
}

inline std::string RPL_ENDOFLINKS(const std::string& nick, const std::string& mask)
{
    return ":" + std::string(SERVER_NAME) + " 365 " + safe_nick(nick) + " " + mask + " :End of LINKS list";
}

// Ban list (367-368)
inline std::string RPL_BANLIST(const std::string& nick, const std::string& channel, const std::string& banmask)
{
    return ":" + std::string(SERVER_NAME) + " 367 " + safe_nick(nick) + " " + channel + " " + banmask;
}

inline std::string RPL_ENDOFBANLIST(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 368 " + safe_nick(nick) + " " + channel + " :End of channel ban list";
}

// INFO replies (371, 374)
inline std::string RPL_INFO(const std::string& nick, const std::string& info)
{
    return ":" + std::string(SERVER_NAME) + " 371 " + safe_nick(nick) + " :" + info;
}

inline std::string RPL_ENDOFINFO(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 374 " + safe_nick(nick) + " :End of INFO list";
}

// MOTD replies (372, 375-376)
inline std::string RPL_MOTDSTART(const std::string& nick, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 375 " + safe_nick(nick) + " :- " + server + " Message of the day - ";
}

inline std::string RPL_MOTD(const std::string& nick, const std::string& text)
{
    return ":" + std::string(SERVER_NAME) + " 372 " + safe_nick(nick) + " :- " + text;
}

inline std::string RPL_ENDOFMOTD(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 376 " + safe_nick(nick) + " :End of MOTD command";
}

// Operator replies (381-383)
inline std::string RPL_YOUREOPER(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 381 " + safe_nick(nick) + " :You are now an IRC operator";
}

inline std::string RPL_REHASHING(const std::string& nick, const std::string& configfile)
{
    return ":" + std::string(SERVER_NAME) + " 382 " + safe_nick(nick) + " " + configfile + " :Rehashing";
}

inline std::string RPL_YOURESERVICE(const std::string& nick, const std::string& servicename)
{
    return ":" + std::string(SERVER_NAME) + " 383 " + safe_nick(nick) + " :You are service " + servicename;
}

// Time reply (391)
inline std::string RPL_TIME(const std::string& nick, const std::string& server, const std::string& time)
{
    return ":" + std::string(SERVER_NAME) + " 391 " + safe_nick(nick) + " " + server + " :" + time;
}

// USERS replies (392-395)
inline std::string RPL_USERSSTART(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 392 " + safe_nick(nick) + " :UserID   Terminal  Host";
}

inline std::string RPL_USERS(const std::string& nick, const std::string& username, const std::string& ttyline, 
                             const std::string& hostname)
{
    return ":" + std::string(SERVER_NAME) + " 393 " + safe_nick(nick) + " :" + username + " " + ttyline + " " + hostname;
}

inline std::string RPL_ENDOFUSERS(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 394 " + safe_nick(nick) + " :End of users";
}

inline std::string RPL_NOUSERS(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 395 " + safe_nick(nick) + " :Nobody logged in";
}

// TRACE replies (200-209, 261-262)
inline std::string RPL_TRACELINK(const std::string& nick, const std::string& version, const std::string& destination, 
                                 const std::string& nextserver, const std::string& protocol, const std::string& uptime, 
                                 const std::string& backstream, const std::string& upstream)
{
    return ":" + std::string(SERVER_NAME) + " 200 " + safe_nick(nick) + " Link " + version + " " + destination + " " + nextserver + " V" + protocol + " " + uptime + " " + backstream + " " + upstream;
}

inline std::string RPL_TRACECONNECTING(const std::string& nick, const std::string& cls, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 201 " + safe_nick(nick) + " Try. " + cls + " " + server;
}

inline std::string RPL_TRACEHANDSHAKE(const std::string& nick, const std::string& cls, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 202 " + safe_nick(nick) + " H.S. " + cls + " " + server;
}

inline std::string RPL_TRACEUNKNOWN(const std::string& nick, const std::string& cls, const std::string& ip)
{
    return ":" + std::string(SERVER_NAME) + " 203 " + safe_nick(nick) + " ???? " + cls + " " + ip;
}

inline std::string RPL_TRACEOPERATOR(const std::string& nick, const std::string& cls, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 204 " + safe_nick(nick) + " Oper " + cls + " " + target;
}

inline std::string RPL_TRACEUSER(const std::string& nick, const std::string& cls, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 205 " + safe_nick(nick) + " User " + cls + " " + target;
}

inline std::string RPL_TRACESERVER(const std::string& nick, const std::string& cls, const std::string& intS, 
                                   const std::string& intC, const std::string& server, const std::string& mask, 
                                   const std::string& protocol)
{
    return ":" + std::string(SERVER_NAME) + " 206 " + safe_nick(nick) + " Serv " + cls + " " + intS + "S " + intC + "C " + server + " " + mask + " V" + protocol;
}

inline std::string RPL_TRACESERVICE(const std::string& nick, const std::string& cls, const std::string& name, 
                                    const std::string& type, const std::string& activetype)
{
    return ":" + std::string(SERVER_NAME) + " 207 " + safe_nick(nick) + " Service " + cls + " " + name + " " + type + " " + activetype;
}

inline std::string RPL_TRACENEWTYPE(const std::string& nick, const std::string& newtype, const std::string& clientname)
{
    return ":" + std::string(SERVER_NAME) + " 208 " + safe_nick(nick) + " " + newtype + " 0 " + clientname;
}

inline std::string RPL_TRACECLASS(const std::string& nick, const std::string& cls, const std::string& count)
{
    return ":" + std::string(SERVER_NAME) + " 209 " + safe_nick(nick) + " Class " + cls + " " + count;
}

inline std::string RPL_TRACELOG(const std::string& nick, const std::string& logfile, const std::string& debuglevel)
{
    return ":" + std::string(SERVER_NAME) + " 261 " + safe_nick(nick) + " File " + logfile + " " + debuglevel;
}

inline std::string RPL_TRACEEND(const std::string& nick, const std::string& server, const std::string& version)
{
    return ":" + std::string(SERVER_NAME) + " 262 " + safe_nick(nick) + " " + server + " " + version + " :End of TRACE";
}

// STATS replies (211-212, 219, 242-243)
inline std::string RPL_STATSLINKINFO(const std::string& nick, const std::string& linkname, const std::string& sendq, 
                                     const std::string& sentmsg, const std::string& sentkb, const std::string& recvmsg, 
                                     const std::string& recvkb, const std::string& timeopen)
{
    return ":" + std::string(SERVER_NAME) + " 211 " + safe_nick(nick) + " " + linkname + " " + sendq + " " + sentmsg + " " + sentkb + " " + recvmsg + " " + recvkb + " " + timeopen;
}

inline std::string RPL_STATSCOMMANDS(const std::string& nick, const std::string& command, const std::string& count, 
                                     const std::string& bytecount, const std::string& remotecount)
{
    return ":" + std::string(SERVER_NAME) + " 212 " + safe_nick(nick) + " " + command + " " + count + " " + bytecount + " " + remotecount;
}

inline std::string RPL_ENDOFSTATS(const std::string& nick, const std::string& letter)
{
    return ":" + std::string(SERVER_NAME) + " 219 " + safe_nick(nick) + " " + letter + " :End of STATS report";
}

inline std::string RPL_STATSUPTIME(const std::string& nick, const std::string& uptime)
{
    return ":" + std::string(SERVER_NAME) + " 242 " + safe_nick(nick) + " :Server Up " + uptime;
}

inline std::string RPL_STATSOLINE(const std::string& nick, const std::string& hostmask, const std::string& name)
{
    return ":" + std::string(SERVER_NAME) + " 243 " + safe_nick(nick) + " O " + hostmask + " * " + name;
}

// User mode reply (221)
inline std::string RPL_UMODEIS(const std::string& nick, const std::string& modes)
{
    return ":" + std::string(SERVER_NAME) + " 221 " + safe_nick(nick) + " " + modes;
}

// Service list (234-235)
inline std::string RPL_SERVLIST(const std::string& nick, const std::string& name, const std::string& server, 
                                const std::string& mask, const std::string& type, const std::string& hopcount, 
                                const std::string& info)
{
    return ":" + std::string(SERVER_NAME) + " 234 " + safe_nick(nick) + " " + name + " " + server + " " + mask + " " + type + " " + hopcount + " " + info;
}

inline std::string RPL_SERVLISTEND(const std::string& nick, const std::string& mask, const std::string& type)
{
    return ":" + std::string(SERVER_NAME) + " 235 " + safe_nick(nick) + " " + mask + " " + type + " :End of service listing";
}

// LUSER replies (251-255)
inline std::string RPL_LUSERCLIENT(const std::string& nick, const std::string& users, const std::string& services, 
                                   const std::string& servers)
{
    return ":" + std::string(SERVER_NAME) + " 251 " + safe_nick(nick) + " :There are " + users + " users and " + services + " services on " + servers + " servers";
}

inline std::string RPL_LUSEROP(const std::string& nick, const std::string& ops)
{
    return ":" + std::string(SERVER_NAME) + " 252 " + safe_nick(nick) + " " + ops + " :operator(s) online";
}

inline std::string RPL_LUSERUNKNOWN(const std::string& nick, const std::string& unknown)
{
    return ":" + std::string(SERVER_NAME) + " 253 " + safe_nick(nick) + " " + unknown + " :unknown connection(s)";
}

inline std::string RPL_LUSERCHANNELS(const std::string& nick, const std::string& channels)
{
    return ":" + std::string(SERVER_NAME) + " 254 " + safe_nick(nick) + " " + channels + " :channels formed";
}

inline std::string RPL_LUSERME(const std::string& nick, const std::string& clients, const std::string& servers)
{
    return ":" + std::string(SERVER_NAME) + " 255 " + safe_nick(nick) + " :I have " + clients + " clients and " + servers + " servers";
}

// Admin replies (256-259)
inline std::string RPL_ADMINME(const std::string& nick, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 256 " + safe_nick(nick) + " " + server + " :Administrative info";
}

inline std::string RPL_ADMINLOC1(const std::string& nick, const std::string& info)
{
    return ":" + std::string(SERVER_NAME) + " 257 " + safe_nick(nick) + " :" + info;
}

inline std::string RPL_ADMINLOC2(const std::string& nick, const std::string& info)
{
    return ":" + std::string(SERVER_NAME) + " 258 " + safe_nick(nick) + " :" + info;
}

inline std::string RPL_ADMINEMAIL(const std::string& nick, const std::string& email)
{
    return ":" + std::string(SERVER_NAME) + " 259 " + safe_nick(nick) + " :" + email;
}

// Try again (263)
inline std::string RPL_TRYAGAIN(const std::string& nick, const std::string& command)
{
    return ":" + std::string(SERVER_NAME) + " 263 " + safe_nick(nick) + " " + command + " :Please wait a while and try again.";
}

/*
============================================================================
Error Replies (400-599)
============================================================================
*/

// No such errors (401-409)
inline std::string ERR_NOSUCHNICK(const std::string& nick, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 401 " + safe_nick(nick) + " " + target + " :No such nick/channel";
}

inline std::string ERR_NOSUCHSERVER(const std::string& nick, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 402 " + safe_nick(nick) + " " + server + " :No such server";
}

inline std::string ERR_NOSUCHCHANNEL(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 403 " + safe_nick(nick) + " " + channel + " :No such channel";
}

inline std::string ERR_CANNOTSENDTOCHAN(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 404 " + safe_nick(nick) + " " + channel + " :Cannot send to channel";
}

inline std::string ERR_TOOMANYCHANNELS(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 405 " + safe_nick(nick) + " " + channel + " :You have joined too many channels";
}

inline std::string ERR_WASNOSUCHNICK(const std::string& nick, const std::string& target)
{
    return ":" + std::string(SERVER_NAME) + " 406 " + safe_nick(nick) + " " + target + " :There was no such nickname";
}

inline std::string ERR_TOOMANYTARGETS(const std::string& nick, const std::string& target, const std::string& errorcode)
{
    return ":" + std::string(SERVER_NAME) + " 407 " + safe_nick(nick) + " " + target + " :" + errorcode + " recipients.";
}

inline std::string ERR_NOSUCHSERVICE(const std::string& nick, const std::string& service)
{
    return ":" + std::string(SERVER_NAME) + " 408 " + safe_nick(nick) + " " + service + " :No such service";
}

inline std::string ERR_NOORIGIN(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 409 " + safe_nick(nick) + " :No origin specified";
}

// Message errors (411-415)
inline std::string ERR_NORECIPIENT(const std::string& nick, const std::string& command)
{
    return ":" + std::string(SERVER_NAME) + " 411 " + safe_nick(nick) + " :No recipient given (" + command + ")";
}

inline std::string ERR_NOTEXTTOSEND(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 412 " + safe_nick(nick) + " :No text to send";
}

inline std::string ERR_NOTOPLEVEL(const std::string& nick, const std::string& mask)
{
    return ":" + std::string(SERVER_NAME) + " 413 " + safe_nick(nick) + " " + mask + " :No toplevel domain specified";
}

inline std::string ERR_WILDTOPLEVEL(const std::string& nick, const std::string& mask)
{
    return ":" + std::string(SERVER_NAME) + " 414 " + safe_nick(nick) + " " + mask + " :Wildcard in toplevel domain";
}

inline std::string ERR_BADMASK(const std::string& nick, const std::string& mask)
{
    return ":" + std::string(SERVER_NAME) + " 415 " + safe_nick(nick) + " " + mask + " :Bad Server/host mask";
}

// Command errors (421-424)
inline std::string ERR_UNKNOWNCOMMAND(const std::string& nick, const std::string& command)
{
    return ":" + std::string(SERVER_NAME) + " 421 " + safe_nick(nick) + " " + command + " :Unknown command";
}

inline std::string ERR_NOMOTD(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 422 " + safe_nick(nick) + " :MOTD File is missing";
}

inline std::string ERR_NOADMININFO(const std::string& nick, const std::string& server)
{
    return ":" + std::string(SERVER_NAME) + " 423 " + safe_nick(nick) + " " + server + " :No administrative info available";
}

inline std::string ERR_FILEERROR(const std::string& nick, const std::string& fileop, const std::string& file)
{
    return ":" + std::string(SERVER_NAME) + " 424 " + safe_nick(nick) + " :File error doing " + fileop + " on " + file;
}

// Nickname errors (431-437)
inline std::string ERR_NONICKNAMEGIVEN(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 431 " + safe_nick(nick) + " :No nickname given";
}

inline std::string ERR_ERRONEUSNICKNAME(const std::string& nick, const std::string& badnick)
{
    return ":" + std::string(SERVER_NAME) + " 432 " + safe_nick(nick) + " " + badnick + " :Erroneous nickname";
}

inline std::string ERR_NICKNAMEINUSE(const std::string& nick, const std::string& badnick)
{
    return ":" + std::string(SERVER_NAME) + " 433 " + safe_nick(nick) + " " + badnick + " :Nickname is already in use";
}

inline std::string ERR_NICKCOLLISION(const std::string& nick, const std::string& badnick, const std::string& user, 
                                     const std::string& host)
{
    return ":" + std::string(SERVER_NAME) + " 436 " + safe_nick(nick) + " " + badnick + " :Nickname collision KILL from " + user + "@" + host;
}

inline std::string ERR_UNAVAILRESOURCE(const std::string& nick, const std::string& resource)
{
    return ":" + std::string(SERVER_NAME) + " 437 " + safe_nick(nick) + " " + resource + " :Nick/channel is temporarily unavailable";
}

// User/Channel errors (441-446)
inline std::string ERR_USERNOTINCHANNEL(const std::string& nick, const std::string& target, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 441 " + safe_nick(nick) + " " + target + " " + channel + " :They aren't on that channel";
}

inline std::string ERR_NOTONCHANNEL(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 442 " + safe_nick(nick) + " " + channel + " :You're not on that channel";
}

inline std::string ERR_USERONCHANNEL(const std::string& nick, const std::string& user, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 443 " + safe_nick(nick) + " " + user + " " + channel + " :is already on channel";
}

inline std::string ERR_NOLOGIN(const std::string& nick, const std::string& user)
{
    return ":" + std::string(SERVER_NAME) + " 444 " + safe_nick(nick) + " " + user + " :User not logged in";
}

inline std::string ERR_SUMMONDISABLED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 445 " + safe_nick(nick) + " :SUMMON has been disabled";
}

inline std::string ERR_USERSDISABLED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 446 " + safe_nick(nick) + " :USERS has been disabled";
}

// Registration errors (451, 461-466)
inline std::string ERR_NOTREGISTERED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 451 " + safe_nick(nick) + " :You have not registered";
}

inline std::string ERR_NEEDMOREPARAMS(const std::string& nick, const std::string& command)
{
    return ":" + std::string(SERVER_NAME) + " 461 " + safe_nick(nick) + " " + command + " :Not enough parameters";
}

inline std::string ERR_ALREADYREGISTRED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 462 " + safe_nick(nick) + " :Unauthorized command (already registered)";
}

inline std::string ERR_NOPERMFORHOST(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 463 " + safe_nick(nick) + " :Your host isn't among the privileged";
}

inline std::string ERR_PASSWDMISMATCH(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 464 " + safe_nick(nick) + " :Password incorrect";
}

inline std::string ERR_YOUREBANNEDCREEP(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 465 " + safe_nick(nick) + " :You are banned from this server";
}

inline std::string ERR_YOUWILLBEBANNED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 466 " + safe_nick(nick) + " :You will be banned";
}

// Channel operation errors (467, 471-478)
inline std::string ERR_KEYSET(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 467 " + safe_nick(nick) + " " + channel + " :Channel key already set";
}

inline std::string ERR_CHANNELISFULL(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 471 " + safe_nick(nick) + " " + channel + " :Cannot join channel (+l)";
}

inline std::string ERR_UNKNOWNMODE(const std::string& nick, const std::string& c, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 472 " + safe_nick(nick) + " " + c + " :is unknown mode char to me for " + channel;
}

inline std::string ERR_INVITEONLYCHAN(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 473 " + safe_nick(nick) + " " + channel + " :Cannot join channel (+i)";
}

inline std::string ERR_BANNEDFROMCHAN(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 474 " + safe_nick(nick) + " " + channel + " :Cannot join channel (+b)";
}

inline std::string ERR_BADCHANNELKEY(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 475 " + safe_nick(nick) + " " + channel + " :Cannot join channel (+k)";
}

inline std::string ERR_BADCHANMASK(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 476 " + safe_nick(nick) + " " + channel + " :Bad Channel Mask";
}

inline std::string ERR_NOCHANMODES(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 477 " + safe_nick(nick) + " " + channel + " :Channel doesn't support modes";
}

inline std::string ERR_BANLISTFULL(const std::string& nick, const std::string& channel, const std::string& c)
{
    return ":" + std::string(SERVER_NAME) + " 478 " + safe_nick(nick) + " " + channel + " " + c + " :Channel list is full";
}

// Permission errors (481-485, 491)
inline std::string ERR_NOPRIVILEGES(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 481 " + safe_nick(nick) + " :Permission Denied- You're not an IRC operator";
}

inline std::string ERR_CHANOPRIVSNEEDED(const std::string& nick, const std::string& channel)
{
    return ":" + std::string(SERVER_NAME) + " 482 " + safe_nick(nick) + " " + channel + " :You're not channel operator";
}

inline std::string ERR_CANTKILLSERVER(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 483 " + safe_nick(nick) + " :You can't kill a server!";
}

inline std::string ERR_RESTRICTED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 484 " + safe_nick(nick) + " :Your connection is restricted!";
}

inline std::string ERR_UNIQOPPRIVSNEEDED(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 485 " + safe_nick(nick) + " :You're not the original channel operator";
}

inline std::string ERR_NOOPERHOST(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 491 " + safe_nick(nick) + " :No O-lines for your host";
}

// User mode errors (501-502)
inline std::string ERR_UMODEUNKNOWNFLAG(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 501 " + safe_nick(nick) + " :Unknown MODE flag";
}

inline std::string ERR_USERSDONTMATCH(const std::string& nick)
{
    return ":" + std::string(SERVER_NAME) + " 502 " + safe_nick(nick) + " :Cannot change mode for other users";
}

// Custom Errors Replies
inline std::string ERR_CLOSINGLINK(const std::string& ip)
{
    return "ERROR :Closing Link: " + ip + " (Connection timed out)";
}

#endif // REPLY_HPP

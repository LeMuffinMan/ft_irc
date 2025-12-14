//! Privmsg Command tests
use crate::client::Client;
use anyhow::Result;


pub async fn privmsg_no_such_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgnosuchnick", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = line.replace("PING", "PONG");
            client.send(&resp, 0).await?;
        }
    }
    client
        .try_expect(
            "PRIVMSG nosuchnickname :msg\r\n",
            " :No such nick/channel",
            "ERR_NOSUCHNICK missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_to_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgtonick", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;

    client
        .try_expect(
            "JOIN #privmsg_nick\r\n",
            "JOIN #privmsg_nick",
            "Failed to join #privmsg_nick",
            timeout_ms,
        )
        .await?;
    client
        .try_expect(
            "PRIVMSG invite_chan_client :msg\r\n",
            "PRIVMSG invite_chan_client :msg",
            "Controle client privmsg didn't answered back to PRIVMSG to user ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

//j'ai une reply 402, pourtant on devrait pouvoir le faire ?
pub async fn privmsg_to_nick_not_sharing_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_prmsmnicknotinchan", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG privmsg_nick_client :msg\r\n",
            "PRIVMSG privmsg_nick_client :msg",
            "Controle client privmsg didn't answered back to PRIVMSG to user ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_to_chan(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgtochan", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "JOIN #privmsg_chan\r\n",
            "JOIN #privmsg_chan",
            "Failed to join #privmsg_chan",
            timeout_ms,
        )
        .await?;
    client
        .try_expect(
            "PRIVMSG #privmsg_chan :msg\r\n",
            "PRIVMSG #privmsg_chan :msg",
            "Controle client privmsg didn't answered back to PRIVMSG to chan ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsg_no_such_channel", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG #notexistingchan :msg\r\n",
            " :No such channel",
            "ERR_NOSUCHCHANNEL missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn privmsg_no_recipient(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgnorecipient", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG\r\n",
            " :No recipient given",
            "ERR_NORECIPIENT missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_no_text_to_send(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}privmsgnotexttosend", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG target\r\n",
            " :No text to send",
            "ERR_NORECIPIENT missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;

    Ok(())
}

pub async fn privmsg_too_many_targets(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_privmsgtomanytar", id);
    let mut client = Client::connect(port).await?;

    client.authenticate(nick, timeout_ms).await?;
    client
        .try_expect(
            "PRIVMSG target target :msg\r\n",
            "407",
            "ERR_TOOMANYTARGET missing ",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn privmsg_not_registered(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .try_expect(
            "PRIVMSG reserved_nick :msg\r\n",
            " :You have not registered",
            "ERR_NOTREGISTERED missing",
            timeout_ms,
        )
        .await?;
    client.shutdown().await?;
    Ok(())
}

use crate::client::Client;
use anyhow::Result;

// PrivmsgCannotSendToChan,
// - NORMAL : 2 client in same chan
// - TOOMANYTARGET
// - Not registerd
// - NOT op in chan targeted ?
//

pub async fn privmsg_no_such_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_privmsg_no_such_nick\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_privmsg_no_such_nick 0 * :privmsg_no_such_nick\r\n",
                nick
            ),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("PRIVMSG nosuchnickname :msg\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No such nick/channel") {
            return Err(anyhow::anyhow!("Err_NOSUCHNICK missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn privmsg_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let channel = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_pmsg_ns_chan\r\n", channel), 0)
        .await?;
    client
        .send(
            &format!("USER {}_pmsg_ns_chan 0 * :pmsg_ns_chan\r\n", channel),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("PRIVMSG #nosuchchannel\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No such channel") {
            return Err(anyhow::anyhow!("ERR_NOSUCHCHAN missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn privmsg_no_recipient(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_privmsg_no_recipient\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_privmsg_no_recipient 0 * :privmsg_no_recipient\r\n",
                nick
            ),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("PRIVMSG\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No recipient given") {
            return Err(anyhow::anyhow!("ERR_NORECIPIENT missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn privmsg_no_text_to_send(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_privmsg_no_text\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!("USER {}_privmsg_no_text 0 * :privmsg_no_text\r\n", nick),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    client.send("PRIVMSG target\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No text to send") {
            return Err(anyhow::anyhow!("ERR_NORECIPIENT missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

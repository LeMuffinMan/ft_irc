use crate::client::Client;
use anyhow::Result;

// ClientBehavior::KickBadChanMask,
// ClientBehavior::KickChaNoPrivsNeeded,
// ClientBehavior::KickUserNotInChannel,
//

pub async fn kick_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("kickneedmorep_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {} 0 * :kick_need_more_params\r\n",
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
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = line.replace("PING", "PONG");
            client.send(&resp, 0).await?;
        }
    }
    client.send("KICK \r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("ERR_NEEDMOREPARAMS missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn kick_no_such_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let channel = format!("Kick_no_such_chan_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}\r\n", channel), 0)
        .await?;
    client
        .send(
            &format!("USER {} 0 * :kick_ns_chan\r\n", channel),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
        }
    }

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = line.replace("PING", "PONG");
            client.send(&resp, 0).await?;
        }
    }

    client.send("KICK #nosuchchannel user\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No such channel") {
            return Err(anyhow::anyhow!("ERR_NOSUCHCHAN missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

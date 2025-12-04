use crate::client::Client;
use anyhow::Result;

// ClientBehavior::KickBadChanMask,
// ClientBehavior::KickChaNoPrivsNeeded,
// ClientBehavior::KickUserNotInChannel,
//

pub async fn kick_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_kick_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_kick_more_params 0 * :kick_need_more_params\r\n",
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
    let channel = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("channel {}_kick_ns_chan\r\n", channel), 0)
        .await?;
    client
        .send(
            &format!("USER {}_kick_ns_chan 0 * :kick_ns_chan\r\n", channel),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("welcome message missing | received [{line}]"));
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

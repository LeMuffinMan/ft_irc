use crate::client::Client;
use anyhow::Result;

//Rpl inviting

pub async fn invite_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_invite_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_invite_more_params 0 * :invite_need_more_params\r\n",
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

    client.send("INVITE \r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("Err_NEEDMOREPARAMS missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_not_on_channel(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_invite_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_invite_more_params 0 * :invite_need_more_params\r\n",
                nick
            ),
            0,
        )
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
        }
    }

    client.send("INVITE reserved_nick channel\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :You're not on that channel") {
            return Err(anyhow::anyhow!("ERR_NOTONCHANNEL missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn invite_no_such_nick(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_invite_more_params\r\n", nick), 0)
        .await?;
    client
        .send(
            &format!(
                "USER {}_invite_more_params 0 * :invite_need_more_params\r\n",
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

    client.send("INVITE nosuchnickname\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :No such nick/channel") {
            return Err(anyhow::anyhow!("Err_NOSUCHNICK missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

//un witness qui join un channel par defaut
// :is already on channel
//  :You're not channel operator

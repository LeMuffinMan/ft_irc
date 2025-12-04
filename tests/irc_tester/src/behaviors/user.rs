use crate::client::Client;
use anyhow::Result;

pub async fn user_already_registered(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("user_already_registered_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}\r\n", nick), 0)
        .await?;
    client
        .send(&format!("USER {} 0 * :useralreadyregistered\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
        }
    }

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = line.replace("PING", "PONG");
            client.send(&resp, 0).await?;
        }
    }

    client
        .send(&format!("USER {}_no_nick 0 * :NoNick\r\n", nick), 0)
        .await?;


    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Unauthorized command (already registered)") {
            return Err(anyhow::anyhow!("ERR_ALREADYREGISTERED missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn user_need_more_params(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("user_more_p_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_no_nick\r\n", nick), 0)
        .await?;
    client
        .send(&format!("USER {}_no_nick 0 *\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("ERR_NEEDMOREPARAMS on User missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}



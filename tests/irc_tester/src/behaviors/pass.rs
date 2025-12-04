use crate::client::Client;
use anyhow::Result;

pub async fn pass_already_registered(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("pass_already_registered_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}\r\n", nick), 0)
        .await?;
    client
        .send(&format!("USER {} 0 * :NoNick\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
        }
    }

    client.send("PASS password\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Unauthorized command (already registered)") {
            return Err(anyhow::anyhow!("ERR_ALREADYREGISTERED missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn pass_need_more_params(port: u16, _id: usize, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client.send("PASS\r\n", 0).await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains(" :Not enough parameters") {
            return Err(anyhow::anyhow!("ERR_NEEDMOREPARAMS on Pass missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn normal_connection_wrong_password(port: u16, _debug: bool) -> Result<()> {
    let mut client = Client::connect(port).await?;
    client.send("PASS incorrect_password\r\n", 0).await?;

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(1000).await? {
        responses.push(line);
    }

    let combined = responses.concat();
    client.shutdown().await?;

    if combined.contains("Password incorrect") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'Invalid password' in server response | received [{combined}]"
        ))
    }
}

pub async fn pass_not_first(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send(&format!("NICK {}\r\n", nick), 0).await?;
    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("USER {} 0 * :PassNotFirst\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Should not have been welcomed | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

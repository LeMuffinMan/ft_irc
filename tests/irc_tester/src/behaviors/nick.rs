use crate::client::Client;
use anyhow::Result;

pub async fn nick_normal_claim_and_change(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("nick_change_{}", id);
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

    client
        .send(&format!("NICK {}_claimed\r\n", nick), 0)
        .await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("updated his nickname to") {
            return Err(anyhow::anyhow!("failed to update nickname | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

pub async fn nick_no_nickname_given(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("nonickgiven_{}", id);
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

    client.send("NICK\r\n", 0).await?;
    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("No nickname given") {
            return Err(anyhow::anyhow!("failed to reject update | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}



use crate::client::Client;
use anyhow::Result;

pub async fn normal_connection(port: u16, _debug: bool, id: usize) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client.send(&format!("NICK normal_{}\r\n", id), 0).await?;
    client
        .send(&format!("USER normal_{} 0 * :normal_connection\r\n", id), 0)
        .await?;

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(1000).await? {
        responses.push(line);
    }

    let combined = responses.concat();
    client.shutdown().await?;

    if combined.contains("Welcome to the Internet Relay Network") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(
            "Expected 'Welcome to the Internet Relay Network' in server response | received [{combined}]"
        ))
    }
}

pub async fn legit_disconnect(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client.send(&format!("NICK {}\r\n", nick), 0).await?;
    client
        .send(&format!("USER {} 0 * :LegitDisconnect\r\n", nick), 0)
        .await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
        }
    }
    client.shutdown().await?;
    Ok(())
}

//! Protocol connection tests
use crate::client::Client;
use anyhow::Result;
use tokio::time::Duration;

pub async fn legit_disconnect(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("legit_disco_{}", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;
    client.shutdown().await?;
    Ok(())
}

pub async fn fragmented_messages(port: u16, _debug: bool, id: usize) -> Result<()> {
    let mut client = Client::connect(port).await?;

    let nick = format!("frag_{}", id);
    let user = format!("user_{}", id);

    let fragments: &[(&str, u64)] = &[
        ("PASS pass", 100),
        ("word\r\n", 100),
        ("N", 50),
        ("ICK", 50),
        (&format!(" {}\r\n", nick), 100),
        ("USE", 80),
        ("R ", 80),
        (&format!("{} 0 /// ", user), 80),
        (&format!(":{}\r\n", user), 100),
    ];

    for (frag, delay) in fragments {
        client.send_raw(frag.as_bytes()).await?;
        tokio::time::sleep(Duration::from_millis(*delay)).await;
    }

    let mut responses = Vec::new();
    while let Some(line) = client.read_line_timeout(1000).await? {
        responses.push(line);
    }

    let combined = responses.concat();
    client.shutdown().await?;

    if combined.contains("Welcome to the Internet Relay Network") {
        Ok(())
    } else {
        Err(anyhow::anyhow!(format!(
            "Expected 'Welcome to the Internet Relay Network' in server response, received instead  : {combined}"
        )))
    }
}

pub async fn low_bandwidth(port: u16, _debug: bool, id: usize) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let msg = &format!(
        "PASS password\r\nNICK slow_{}\r\nUSER slow 0 /// :slow user\r\n",
        id
    );

    for b in msg.bytes() {
        client.send_raw(&[b]).await?;
        tokio::time::sleep(Duration::from_millis(10)).await;
    }

    tokio::time::sleep(Duration::from_millis(1000)).await;

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

pub async fn too_long_message(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let long = "A".repeat(513) + "\r\n";

    client.send(&long, 0).await?;

    if let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.contains("Buffer limit") {
            return Ok(());
        }
        return Err(anyhow::anyhow!(
            "Server did not reject overly long message | received [{line}]"
        ));
    }
    Err(anyhow::anyhow!("Server closed connection without error"))
}

pub async fn continuous_noise(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;
    let mut counter = 0;

    loop {
        let msg = format!("NOISE_{}\r\n", counter);
        counter += 1;

        client.send(&msg, 0).await?;
        tokio::time::sleep(tokio::time::Duration::from_millis(50)).await;

        if let Some(line) = client.read_line_timeout(timeout_ms).await? {
            if line.contains("timed out") {
                return Ok(());
            }
        } else {
            break;
        }
    }

    Err(anyhow::anyhow!("Server should have kicked us"))
}

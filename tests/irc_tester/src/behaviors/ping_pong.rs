use crate::client::Client;
use anyhow::Result;
use tokio::time::{Duration, timeout};


/**
*
* @Brief Ping tests
*
* Tests to verify the server behavior sending and answering pong, or
* expulsing inactives users 
*
*/

pub async fn legit_ignore_pong(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;
    let total_timeout = Duration::from_secs(20);

    client.authenticate(nick, timeout_ms).await?;
    let result = timeout(total_timeout, async {
        loop {
            match client.read_line_timeout(timeout_ms).await? {
                Some(line) => {
                    if line.starts_with("PING") {
                        continue;
                    } else if line.contains("timed out") {
                        return Ok::<(), anyhow::Error>(());
                    }
                }
                None => {}
            }
        }
    })
    .await;
    match result {
        Ok(inner) => inner,
        Err(_) => Err(anyhow::anyhow!("Timeout 20s waiting for server kick")),
    }
}

pub async fn pong_only(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_Uncomplete_registered_pong\r\n", nick), 0)
        .await?;

    loop {
        if let Some(line) = client.read_line_timeout(timeout_ms).await? {
            if line.starts_with("PING") {
                let resp = line.replace("PING", "PONG");
                client.send(&resp, 0).await?;
            } else if line.contains("timed out") {
                return Ok(());
            }
        } else {
            break;
        }
    }
    Err(anyhow::anyhow!("Should have been kicked"))
}

pub async fn wrong_pong(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("{}_wrong_pong", id);
    let mut client = Client::connect(port).await?;
    client.authenticate(nick, timeout_ms).await?;

    while let Some(line) = client.read_line_timeout(timeout_ms).await? {
        if line.starts_with("PING") {
            let resp = "PONG :4242424242\r\n";
            client.send(&resp, 0).await?;
        } else if line.contains("Invalid pong") {
            return Ok(());
        }
    }
    Err(anyhow::anyhow!("Should have been kicked"))
}

pub async fn pong_without_connect(port: u16, timeout_ms: u64) -> Result<()> {
    let mut client = Client::connect(port).await?;

    loop {
        if let Some(line) = client.read_line_timeout(timeout_ms).await? {
            if line.starts_with("PING") {
                let resp = line.replace("PING", "PONG");
                client.send(&resp, 0).await?;
            } else if line.contains("timed out") {
                return Ok(());
            }
        } else {
            break;
        }
    }
    Err(anyhow::anyhow!("Should have been kicked"))
}

pub async fn start_ignore_all(port: u16, id: usize, timeout_ms: u64) -> Result<()> {
    let nick = format!("stress_{}", id);
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client
        .send(&format!("NICK {}_StartIgnoreAll\r\n", nick), 0)
        .await?;

    let total_timeout = Duration::from_secs(20);

    let result = timeout(total_timeout, async {
        loop {
            match client.read_line_timeout(timeout_ms).await? {
                Some(line) => {
                    if line.starts_with("PING") {
                        continue;
                    } else if line.contains("timed out") {
                        return Ok::<(), anyhow::Error>(());
                    }
                }
                None => {}
            }
        }
    })
    .await;

    match result {
        Ok(inner) => inner,
        Err(_) => Err(anyhow::anyhow!("Timeout 20s waiting for server kick")),
    }
}

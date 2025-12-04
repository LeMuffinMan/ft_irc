use crate::client::Client;
use anyhow::Result;
use std::sync::Arc;
use tokio::sync::Notify;
use tokio::time::{Duration, sleep};

pub async fn witness_client(port: u16, stop_signal: Arc<Notify>) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client.send("NICK reserved_nick\r\n", 0).await?;
    client
        .send("USER reserved_nick 0 * :reserved_nick\r\n", 0)
        .await?;

    loop {
        tokio::select! {
            _ = stop_signal.notified() => {
                client.shutdown().await?;
                break;
            }
            line = client.read_line_timeout(1000) => {
                match line? {
                    Some(l) => {
                        if l.starts_with("PING") {
                            let resp = l.replace("PING", "PONG");
                            client.send(&resp, 0).await?;
                        }
                    }
                    None => {
                        sleep(Duration::from_millis(50)).await;
                    }
                }
            }
        }
    }

    Ok(())
}

use crate::client::Client;
use anyhow::Result;
use tokio::time::{Duration, sleep};

pub async fn controle_client(port: u16) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client.send("NICK reserved_nick\r\n", 0).await?;
    client
        .send("USER reserved_nick 0 * :reserved_nick\r\n", 0)
        .await?;

    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

pub async fn no_mdp_chan_client(port: u16) -> Result<()> {
    let mut client = Client::connect(port).await?;

    client.send("PASS password\r\n", 0).await?;
    client.send("NICK no_mdp_chan_client\r\n", 0).await?;
    client
        .send("USER no_mdp_chan_client 0 * :no_mdp_chan_client\r\n", 0)
        .await?;

    if let Some(line) = client.read_line_timeout(1000).await? {
        if !line.contains("Welcome to the Internet Relay Network") {
            return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
        }
    }
    client
        .send("JOIN #no_mdp_chan\r\n", 0)
        .await?;


    loop {
        match client.read_line_timeout(1000).await {
            Ok(Some(l)) => {
                if l.starts_with("PING") {
                    let resp = l.replace("PING", "PONG");
                    client.send(&resp, 0).await?;
                }
            }
            _ => {
                sleep(Duration::from_millis(50)).await;
            }
        }
    }
}

// pub async fn mdp_chan_client(port: u16, stop_signal: Arc<Notify>) -> Result<()> {
//     let mut client = Client::connect(port).await?;
//
//     client.send("PASS password\r\n", 0).await?;
//     client.send("NICK mdp_chan_client\r\n", 0).await?;
//     client
//         .send("USER mdp_chan_client 0 * :mdp_chan_client\r\n", 0)
//         .await?;
//
//     if let Some(line) = client.read_line_timeout(1000).await? {
//         if !line.contains("Welcome to the Internet Relay Network") {
//             return Err(anyhow::anyhow!("Welcome message missing | received [{line}]"));
//         }
//     }
//     client
//         .send(("JOIN #mdp_chan +k chan\r\n"), 0)
//         .await?;
//
//     loop {
//         tokio::select! {
//             _ = stop_signal.notified() => {
//                 client.shutdown().await?;
//                 break;
//             }
//             line = client.read_line_timeout(1000) => {
//                 match line? {
//                     Some(l) => {
//                         if l.starts_with("PING") {
//                             let resp = l.replace("PING", "PONG");
//                             client.send(&resp, 0).await?;
//                         }
//                     }
//                     None => {
//                         sleep(Duration::from_millis(50)).await;
//                     }
//                 }
//             }
//         }
//     }
//
//     Ok(())
// }
